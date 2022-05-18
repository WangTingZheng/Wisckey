//
// Created by 14037 on 2022/5/17.
//

#include <iostream>
#include "vlog_wisckey.h"

namespace leveldb {
    namespace vlog {
        Status VlogWisckey::AddBatch(WriteBatch *batch) {
            class H : public WriteBatch::Handler{
                public:
                    explicit H(VlogWisckey *wisckey):wisckey_(wisckey){}

                    void Put(const Slice &key, const Slice &value) override {
                       VlogFormat::Value vLogvalue;
                       vLogvalue.type = VlogFormat::vLogValue;
                       vLogvalue.data = value.ToString();

                       // 序列化kv
                       string kvs;
                       VlogFormat::PutKV(&kvs, kTypeValue, key.ToString(), vLogvalue);

                       // 记录要持久化的record的头
                       size_t offset = wisckey_->head_;

                        // 持久化kv
                       Slice data(kvs);
                       Status status = wisckey_->file_->WriteRecord(data);


                       if (status.ok()){ // 如果写入成功
                           // 计算写入的数据量
                            size_t write_size = (data.size() + kVHeaderSize);
                            // 向后移动head_
                            wisckey_->head_ += write_size;

                            // 封装meta
                            VlogFormat::Meta meta{};
                            // 写入前的head
                            meta.offset = offset;
                            /*
                            cout << "offset" << endl;
                            cout << offset << endl;
                            cout << "size" << endl;
                            cout << write_size << endl;
                             */
                            // 写入的大小
                            meta.size = write_size;
                            // 在LSM Tree中更新key-meta
                            wisckey_->handler_->Update(key.ToString(), meta);
                       }
                    }

                    void Delete(const Slice &key) override {

                    }

                    private:
                        VlogWisckey *wisckey_;
            };

            H handler(this);
            // 遍历每一个kv对，把它们插入vlog
            Status status = batch->Iterate(&handler);
            if(!status.ok()) return status;

            // 如果未gc的数据量达到阈值
            if(GetSize() >= gc_threshold_){
                return StartGC(); // 开启gc处理
            }

            return Status::OK();
        }

        VlogWisckey::VlogWisckey(string pathname, size_t gc_threshold, VlogHandler *handler)
            :handler_(handler),
            gc_threshold_(gc_threshold),
            head_(0),
            tail_(0),
            file_(new VlogFile(pathname)){}

        // 当vLog中的数据量大于gc_threshold的时候开始gc
        // 从tail_开始读取record，kTypeValue的，还有效的kv放回vlog，其它的舍弃
        // 当tail_运动到原来head_的位置时，停止gc
        Status VlogWisckey::StartGC() {
            Status status;
            Slice record;

            VlogFormat::KV kv;

            size_t read_tail;
            size_t offset = head_; //刚开始的时候head_的位置，tail_超过它就停止gc

            // 读取一个kv的原始数据
            while (file_->ReadRecord(&record).ok()){
                string dst = record.ToString();
                VlogFormat::GetKV(&dst, &kv);
                if(kv.type == kTypeValue && handler_->IsKeyVaild(kv.key)){   // 如果读取的kv是put入的kv且有效，就重新放回vLog
                    read_tail = head_; // 记录下当前record的head
                    // 写入它
                    status = file_->WriteRecord(record);
                    if(!status.ok())return status;

                    // 写入的大小为数据的大小加上crc和长度的大小
                    size_t write_size = record.size() + kVHeaderSize;

                    // 前面少了一部分数据，所以tail_向前推进
                    tail_ += write_size;
                    // 后面加了一部分数据，所以head_向后推进
                    head_ += write_size;

                    // 读取这一条record的offset和size转换为meta
                    VlogFormat::Meta meta{};

                    // 把写入的当前的record的开头赋值进meta
                    meta.offset = read_tail;
                    // 赋值大小
                    meta.size  = write_size;

                    // 插入key-meta
                    handler_->Update(kv.key, meta);

                    // 如果gc之后读取偏移量达到了之前的写入偏移量，说明应该停止gc了
                    // 不然gc会一直进行下去
                    if(tail_ >= offset) break;
                }
            }

            // 写入到磁盘
            return file_->Sync();
        }

        // 根据从LSM Tree中查询到的meta数据读取对应的value
        Status VlogWisckey::Read(string key, string *meta_val, string *value) {
            VlogFormat::Meta meta{};
            VlogFormat::GetMeta(meta_val, &meta); //把meta_val转换为meta结构体

            // 读取出kv值，并解析为kv结构体
            Slice dst;
            //根据meta里的offset和size来读取数据到value
            Status status = file_->Read(meta.offset, meta.size, &dst);
            if(!status.ok()) return status;

            // dst进行校验
            string kv_dst;
            status = file_->CheckCRC(dst.ToString(), &kv_dst);
            if(!status.ok()) return status;

            VlogFormat::KV kv;

            VlogFormat::GetKV(&kv_dst, &kv);

            if(kv.type != kTypeValue) {
                return Status::Corruption("kv is not kTypeValue");
            } else if(kv.key != key) {
                return Status::Corruption("key is not matched");
            }else{ // 如果kv即是kTypeValue，key又匹配成功
                if(kv.value.type == VlogFormat::vLogValue) {
                    *value = kv.value.data;
                    return Status::OK();
                }
                return Status::Corruption("value is not vLog value");
            }
        }
    }
} // leveldb