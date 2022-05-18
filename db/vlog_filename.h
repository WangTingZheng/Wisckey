//
// Created by 14037 on 2022/5/16.
//

#ifndef VLOG_VLOG_FILENAME_H
#define VLOG_VLOG_FILENAME_H

#include <string>
#include "filename.h"

namespace leveldb {
    namespace vlog{
        using namespace std;
        string VlogNumberToPath(uint64_t vlog_number);
        string VlogNumberToGCPath(uint64_t vlog_number);
        void CleanFile(string filepath, FileType type);
    }

} // leveldb

#endif //VLOG_VLOG_FILENAME_H
