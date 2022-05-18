// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "vlog_writer.h"
#include "vlog_file.h"
#include <cstdint>

#include "../include/leveldb/env.h"

#include "../util/coding.h"
#include "../util/crc32c.h"

namespace leveldb {
namespace vlog {

VWriter::VWriter(WritableFile* dest) : dest_(dest) {}

VWriter::~VWriter() = default;


// 向vlog文件中写入一段数据
Status VWriter::AddRecord(const Slice& slice) {
  const char* ptr = slice.data();
  size_t left = slice.size();
  // Header is checksum (4 bytes), length (8 bytes).
  // length方便读取时进行区分
  char buf[kVHeaderSize];
  uint32_t crc = crc32c::Extend(0, ptr, left);
  crc = crc32c::Mask(crc);  // Adjust for storage
  EncodeFixed32(buf, crc); // 写入crc校验值
  EncodeFixed64(&buf[4], left); // 写入长度
  // 持久化头数据
  Status s = dest_->Append(Slice(buf, kVHeaderSize));
  assert(s.ok());
  if (s.ok()) {
    //    std::string t;
    //    t.push_back(static_cast<char>(kTypeDeletion));
    //    dest_->Append(t);
    // 持久化数据
    s = dest_->Append(Slice(ptr, left));
    assert(s.ok());
    if (s.ok()) {
      s = dest_->Flush();
    }
  }
  return s;
}

    Status VWriter::Sync() {
        return dest_->Sync();
    }

}  // namespace vlog
}  // namespace leveldb
