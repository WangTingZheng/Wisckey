//
// Created by 14037 on 2022/5/18.
//

#ifndef VLOG_VLOG_TEST_FILE_H
#define VLOG_VLOG_TEST_FILE_H

#include "../include/leveldb/status.h"

namespace leveldb {
    namespace vlog {
        class VlogTestFile {
        public:
            static Status TestReadAndWrite();
        };
    }
} // leveldb

#endif //VLOG_VLOG_TEST_FILE_H
