//
// Created by 14037 on 2022/5/17.
//

#include "vlog_file.h"
#include "../util/crc32c.h"
#include "../util/coding.h"
#include <iostream>

namespace leveldb {
    namespace vlog{

        VlogFile::VlogFile(const string& pathname):
            reader_(NewReader(pathname)),
            writer_(NewWriter(pathname)){

        }

        Status VlogFile::WriteRecord(const Slice &data) {
            return writer_->AddRecord(data);
        }

        Status VlogFile::Read(size_t offset, size_t len, Slice *value) {
            char *val = new char[len];
            if(reader_->Read(val, len, offset)){
                *value = Slice(val, len);
                return Status::OK();
            }

            return Status::Corruption("Can not read from disk");
        }


        Status VlogFile::Sync() {
            return writer_->Sync();
        }

        Status VlogFile::ReadRecord(Slice *record) {
            string scratch;
            if(reader_->ReadRecord(record, &scratch)){
                return Status::OK();
            }
            return Status::Corruption("Read Record failed");
        }

        VReader *VlogFile::NewReader(const string &pathname) {
            Env *env = Env::Default();
            SequentialFile *file;
            env->NewSequentialFile(pathname, &file);
            return new VReader(file, true);
        }

        VWriter *VlogFile::NewWriter(const string &pathname) {
            Env *env = Env::Default();
            WritableFile *file;
            env->NewWritableFile(pathname, &file);
            return new VWriter(file);
        }

        Status VlogFile::CheckCRC(string src, string *kv) {
            Slice input(src);

            if(input.size() < kVHeaderSize) return Status::Corruption("string is too short");

            uint32_t expected_crc = crc32c::Unmask(DecodeFixed32(input.data()));
            input.remove_prefix(4);

            uint64_t length = DecodeFixed64(input.data());
            input.remove_prefix(8);

            uint32_t actual_crc = crc32c::Value(input.data(), length);
            if(actual_crc != expected_crc){
                return Status::Corruption("Check crc failed!");
            }

            *kv = string(input.data(), length);

            return Status::OK();
        }
    }

} // leveldb