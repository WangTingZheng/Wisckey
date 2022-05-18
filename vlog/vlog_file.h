//
// Created by 14037 on 2022/5/17.
//

#ifndef VLOG_VLOG_FILE_H
#define VLOG_VLOG_FILE_H

#include <cstdint>
#include "../include/leveldb/status.h"
#include "../include/leveldb/env.h"
#include "vlog_writer.h"
#include "vlog_reader.h"


namespace leveldb {
    namespace vlog {
        using namespace std;
        static const int kVHeaderSize = 4 + 8;
        static const int kBlockSize = 32768;
        class VWriter;
        class VReader;

        class VlogFile {
            public:

                explicit VlogFile(const string& pathname);

                Status WriteRecord(const Slice &data);

                Status ReadRecord(Slice *record);

                Status Read(size_t offset, size_t len, Slice *value);

                Status CheckCRC(string src, string *kv);

                //Status Fallocate(size_t offset, size_t len);

                Status Sync();
            private:
                VWriter *writer_;
                VReader *reader_;

                VReader *NewReader(const string& pathname);
                VWriter *NewWriter(const string& pathname);
            };
    }
} // leveldb

#endif //VLOG_VLOG_FILE_H
