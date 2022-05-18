//
// Created by 14037 on 2022/5/17.
//

#include "vlog_format.h"
#include <iostream>

namespace leveldb {
    namespace vlog{
        using namespace std;

        void VlogFormat::PutValue(string *dst, VlogFormat::VlogValueType type, string data) {
            string real_dst;
            real_dst.push_back(static_cast<char>(type));
            PutLengthPrefixedSlice(&real_dst, data);
            *dst = real_dst;
        }

        void VlogFormat::GetValue(string *src, VlogFormat::Value *value) {
            Slice input(*src);
            value->type = static_cast<VlogValueType>(input[0]);
            input.remove_prefix(1);

            Slice result;
            GetLengthPrefixedSlice(&input, &result);
            value->data = result.ToString();
        }

        void VlogFormat::PutKV(string *dst, ValueType type, string key, VlogFormat::Value value) {
            // 把value转换为string
            string val_dst;
            PutValue(&val_dst, value.type, value.data);

            // 把KV转换为string
            dst->push_back(type);
            PutLengthPrefixedSlice(dst, key);
            PutLengthPrefixedSlice(dst, val_dst);
        }

        void VlogFormat::GetKV(string *src, VlogFormat::KV *kv) {
            Slice input(*src);
            string val_src;

            kv->type = static_cast<ValueType>(input[0]);
            input.remove_prefix(1);

            Slice key;
            Slice value;
            GetLengthPrefixedSlice(&input, &key);
            GetLengthPrefixedSlice(&input, &value);

            kv->key = key.ToString();

            VlogFormat::Value struct_value{};
            val_src = value.ToString();
            GetValue(&val_src, &struct_value);

            kv->value = struct_value;
        }

        void VlogFormat::PutMeta(string *dst, size_t offset, size_t size) {
            PutVarint64(dst, offset);
            PutVarint64(dst, size);
        }

        void VlogFormat::GetMeta(string *src, VlogFormat::Meta *meta) {
            Slice input(*src);
            GetVarint64(&input, &(meta->offset));
            GetVarint64(&input, &(meta->size));
        }
    }
} // leveldb