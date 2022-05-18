//
// Created by 14037 on 2022/5/18.
//

#ifndef VLOG_VLOG_TEST_FORMAT_H
#define VLOG_VLOG_TEST_FORMAT_H

#include "../vlog/vlog_format.h"
#include "../include/leveldb/status.h"

namespace leveldb {

    namespace vlog {
        class VlogTestFormat {
            public:
               static bool TestValue();
               static bool TestKV();
               static bool TestMeta();
               static Status TestAll();

        };
    }

} // leveldb

#endif //VLOG_VLOG_TEST_FORMAT_H
