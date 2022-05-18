//
// Created by 14037 on 2022/5/17.
//

#ifndef VLOG_VLOG_FORMAT_H
#define VLOG_VLOG_FORMAT_H

#include <utility>

#include "../db/dbformat.h"
#include "../util/crc32c.h"

namespace leveldb {

    namespace vlog {
        using namespace std;
        class VlogFormat {
            public:
                enum VlogValueType{vLogValue, vLogMeta};

                struct Value{
                    VlogValueType type;
                    string data;
                };

                static void PutValue(string *dst, VlogValueType type, string data);
                static void GetValue(string *src, Value *value);

                struct KV{
                    ValueType type;
                    string key;
                    Value value;
                };

                static void PutKV(string *dst, ValueType type, string key, Value value);
                static void GetKV(string *src, KV *kv);


                struct Meta{
                    size_t offset;
                    size_t size;
                };

                static void PutMeta(string *dst, size_t offset, size_t size);
                static void GetMeta(string *dst, Meta *meta);
        };
    }
} // leveldb

#endif //VLOG_VLOG_FORMAT_H
