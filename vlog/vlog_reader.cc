// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <cstdio>

#include "vlog_reader.h"
#include "vlog_file.h"
#include "../include/leveldb/env.h"
#include "../util/coding.h"
#include "../util/crc32c.h"
#include "../util/mutexlock.h"

namespace leveldb {
namespace vlog {

VReader::Reporter::~Reporter() {}

VReader::VReader(SequentialFile* file, bool checksum, uint64_t initial_offset)
    : file_(file),
      reporter_(nullptr),
      checksum_(checksum),
      //一次从磁盘读kblocksize，多余的做缓存以便下次读
      backing_store_(new char[kBlockSize]),
      buffer_(),
      eof_(false) {
  if (initial_offset > 0) JumpToPos(initial_offset);
}

VReader::VReader(SequentialFile* file, Reporter* reporter, bool checksum,
                 uint64_t initial_offset)
    : file_(file),
      reporter_(reporter),
      checksum_(checksum),
      //一次从磁盘读kblocksize，多余的做缓存以便下次读
      backing_store_(new char[kBlockSize]),
      buffer_(),
      eof_(false) {
  if (initial_offset > 0) JumpToPos(initial_offset);
}

VReader::~VReader() {
  delete[] backing_store_;
  delete file_;
}

// Skip只能根据当前的位置移动，底层用的是SEEK_CUR
bool VReader::JumpToPos(size_t pos) {
  if (pos > 0) {  //跳到距file文件头偏移pos的地方
    Status skip_status = file_->Jump(pos);
    if (!skip_status.ok()) {
      ReportDrop(pos, skip_status);
      return false;
    }
  }
  return true;
}

// 用于恢复，也可以用作GC
bool VReader::ReadRecord(Slice* record, std::string* scratch) {
  scratch->clear();
  record->clear();

  if (buffer_.size() < kVHeaderSize) {  //遇到buffer_剩的空间不够解析头部时
    if (!eof_) {
      // 先拷贝到backing_store
      // 再读取数据凑成一个Block到buffer和backing_store
      // 重新封装buffer_
      size_t left_head_size = buffer_.size();
      if (left_head_size > 0)  //如果读缓冲还剩内容，拷贝到读缓冲区头
        memcpy(backing_store_, buffer_.data(), left_head_size);
      buffer_.clear();
      // 读取数据到backing_store_中
      // 把backing_store_封装为Slice，赋值到buffer_
      // 此刻buffer_中，data_是[left_head_size, end], size_是kBlockSize - left_head_size
      Status status = file_->Read(kBlockSize - left_head_size, &buffer_,
                                  backing_store_ + left_head_size);

      // kBlockSize - left_head_size + left_head_size = kBlockSize
      // backing_store_
      buffer_ = Slice(backing_store_, buffer_.size() + left_head_size);

      if (!status.ok()) { // 偏移量大于文件大小，读到头了
        buffer_.clear();
        ReportDrop(kBlockSize, status);
        eof_ = true;
        // 文件数据读取完了，直接返回，停止读取
        return false;
      } else if (buffer_.size() < kBlockSize) { //说明文件剩下的数据不管kBlockSize了
        eof_ = true;
        // 如果数据都不够一个Record的头，就没办法读了，直接返回，停止读取
        if (buffer_.size() < kVHeaderSize) return false;
      }
    } else { // 如果上次读取到头了，说明从文件读取数据已经没有可能了
      // 那就只能读取buffer_的数据讲究着用了
      // 如果buffer_的数据都不够头的，那就没必要读了，直接返回
      if (buffer_.size() < kVHeaderSize) {
        buffer_.clear();
        return false;
      }
    }
  }

  //解析头部
  uint64_t length = 0;
  // 解析crc
  uint32_t expected_crc = crc32c::Unmask(DecodeFixed32(
      buffer_.data()));  //早一点解析出crc,因为后面可能buffer_.data内容会变
  buffer_.remove_prefix(4);

  // 解析length
  length = DecodeFixed64(buffer_.data());
  buffer_.remove_prefix(8);

  // 接下来的目标是从buffer_中读取length长度的数据
  // 如果可以从buffer_中读取数据
  if (length <= buffer_.size()) {
    //逻辑记录完整的在buffer中(在block中)
    if (checksum_) {
      uint32_t actual_crc = crc32c::Value(buffer_.data(), length);
      if (actual_crc != expected_crc) {
        ReportCorruption(kVHeaderSize + length, "checksum mismatch");
        return false;
      }
    }

    // 取出长度为length的数据
    *record = Slice(buffer_.data(), length);
    buffer_.remove_prefix(length);
    return true;
  }
  else { // 如果buffer_根本没有那么多数据，那么就要从磁盘中读
    if (eof_) { // 如果磁盘已经没有数据了，那么实际上数据是不完整的，直接返回
      return false;  //日志最后一条记录不完整的情况，直接忽略
    }

    // 逻辑记录不能在buffer中全部容纳，需要将读取结果写入到scratch
    // 是考虑到left_length比kBlockSize还大吗
    // length > buffer_.size()
    // 把buffer_的数据搬移到scratch
    scratch->reserve(length);
    size_t buffer_size = buffer_.size();
    // 把buffer_的前buffer_size个Byte赋值到scratch
    scratch->assign(buffer_.data(), buffer_size);
    buffer_.clear();

    const uint64_t left_length = length - buffer_size;

    // 如果剩余待读的记录超过block块的一半大小，则直接读到scratch中
    // 剩余得比较多，可以直接读？减少磁盘IO？
    if (left_length > kBlockSize / 2) {
      Slice buffer;
      scratch->resize(length);

      // 读取left_length到buffer
      Status status =
          file_->Read(left_length, &buffer,
                      const_cast<char*>(scratch->data()) + buffer_size);

      if (!status.ok()) {
        ReportDrop(left_length, status);
        return false;
      }

      if (buffer.size() < left_length) {
        eof_ = true;
        scratch->clear();
        return false;
      }
    } else {  //否则读一整块到buffer中
      Status status = file_->Read(kBlockSize, &buffer_, backing_store_);

      if (!status.ok()) {
        ReportDrop(kBlockSize, status);
        return false;
      } else if (buffer_.size() < kBlockSize) {
        if (buffer_.size() < left_length) {
          eof_ = true;
          scratch->clear();
          ReportCorruption(left_length, "last record not full");
          return false;
        }
        //这个判断不要也可以，加的话算是优化，提早知道到头了，省的read一次才知道
        eof_ = true;
      }
      scratch->append(buffer_.data(), left_length);
      buffer_.remove_prefix(left_length);
    }


    if (checksum_) {
      uint32_t actual_crc = crc32c::Value(scratch->data(), length);
      if (actual_crc != expected_crc) {
        ReportCorruption(kVHeaderSize + length, "checksum mismatch");
        return false;
      }
    }

    *record = Slice(*scratch);
    return true;
  }
}

// get查询中根据索引从vlog文件中读value值
bool VReader::Read(char* val, size_t size, size_t pos) {  //要考虑多线程情况
  MutexLock l(&mutex_);
  //因为read读的位置随机，因此file的skip接口不行，因为file的skip是相对于当前位置的
  if (!JumpToPos(pos)) {
    return false;
  }
  Slice buffer;
  Status status = file_->Read(size, &buffer, val);
  if (!status.ok() || buffer.size() != size) {
    ReportDrop(size, status);
    return false;
  }
  return true;
}

void VReader::ReportCorruption(uint64_t bytes, const char* reason) {
  ReportDrop(bytes, Status::Corruption(reason));
}

void VReader::ReportDrop(uint64_t bytes, const Status& reason) {
  if (reporter_ != nullptr) {
    reporter_->Corruption(static_cast<size_t>(bytes), reason);
  }
}

bool VReader::DeallocateDiskSpace(uint64_t offset, size_t len) {
  return file_->DeallocateDiskSpace(offset, len).ok();
}

}  // namespace vlog
}  // namespace leveldb
