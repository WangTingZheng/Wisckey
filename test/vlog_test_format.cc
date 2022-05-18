//
// Created by 14037 on 2022/5/18.
//

#include "vlog_test_format.h"
#include "../vlog/vlog_format.h"

namespace leveldb {
    namespace vlog {
        bool VlogTestFormat::TestValue() {
            string dst;
            VlogFormat::PutValue(&dst, VlogFormat::vLogValue, "helloworld");

            VlogFormat::Value value;
            VlogFormat::GetValue(&dst, &value);
            return (value.type == VlogFormat::vLogValue) && (value.data.compare("helloworld") == 0);
        }

        bool VlogTestFormat::TestKV() {
            VlogFormat::Value value;
            value.type = VlogFormat::vLogValue;
            value.data = "value";

            string dst;
            VlogFormat::PutKV(&dst, kTypeValue, "key", value);

            VlogFormat::KV kv;
            VlogFormat::GetKV(&dst, &kv);
            return (kv.type == kTypeValue) && (kv.key.compare("key") == 0) &&(kv.value.type == value.type) &&(kv.value.data.compare(value.data) ==0);
        }

        bool VlogTestFormat::TestMeta() {
            VlogFormat::Meta meta{};

            string dst;
            VlogFormat::PutMeta(&dst, 1212, 12123);

            VlogFormat::GetMeta(&dst, &meta);
            return (meta.offset == 1212) && (meta.size == 12123);
        }

        Status VlogTestFormat::TestAll() {
            if(!TestValue())return Status::Corruption("Value test failed");
            if (!TestKV()) return Status::Corruption("KV test failed");
            if(!TestMeta()) return Status::Corruption("Meta test failed");
            return Status::OK();
        }
    }
} // leveldb