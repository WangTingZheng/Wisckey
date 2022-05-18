//
// Created by 14037 on 2022/5/17.
//

#ifndef VLOG_VLOG_HANDLER_H
#define VLOG_VLOG_HANDLER_H

#include "vlog_format.h"

namespace leveldb {
    namespace vlog {
        using namespace std;

        class VlogHandler {
            public:
                VlogHandler() = default;
                virtual bool IsKeyVaild(string key); // 查询数据结构中，key是否存在
                virtual void Update(string key, VlogFormat::Meta meta); // 向数据结构中更新key-meta

                virtual void Put(string key, string value);
                virtual void Get(string key, string *value);
        };
    }
} // leveldb

#endif //VLOG_VLOG_HANDLER_H
