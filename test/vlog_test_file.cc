//
// Created by 14037 on 2022/5/18.
//

#include "vlog_test_file.h"
#include "../vlog/vlog_file.h"


namespace leveldb {
    namespace vlog{
        Status VlogTestFile::TestReadAndWrite() {
            VlogFile *vlogFile = new VlogFile("./vlogfile.test");
            vector<string> records;
            records.emplace_back("1ww12swxsn");
            records.emplace_back("sbshxsaxljxshhx");
            records.emplace_back("egdywejsjncdjbccjssc");
            records.emplace_back("jadnjandjansnjdhbdhe");
            records.emplace_back("jajdbjdjsndaowoefgscj");

            Status status;
            for(auto item:records){
                status = vlogFile->WriteRecord(item);
                if(!status.ok()) return status;
            }

            Slice record;
            int i = 0;
            while (vlogFile->ReadRecord(&record).ok()){
                if(record.ToString() != records[i]) return Status::Corruption("Get record wrong", record);
                i++;
            }

            return Status::OK();
        }
    }
} // leveldb