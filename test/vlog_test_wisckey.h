//
// Created by 14037 on 2022/5/18.
//

#ifndef VLOG_VLOG_TEST_WISCKEY_H
#define VLOG_VLOG_TEST_WISCKEY_H

#include "../include/leveldb/status.h"

namespace leveldb {
    namespace vlog {
        class VlogTestWisckey {
            public:
                static Status RunTest();
        };
    }
} // leveldb

#endif //VLOG_VLOG_TEST_WISCKEY_H
