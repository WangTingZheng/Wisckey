//
// Created by 14037 on 2022/5/16.
//

#include "vlog_filename.h"
#include "filename.h"
#include <vector>
#include "../include/leveldb/env.h"

namespace leveldb {
    namespace vlog{
        using namespace std;
        string VlogNumberToPath(uint64_t vlog_number){
            return "./" + to_string(vlog_number) + ".vlogfile";
        }

        string VlogNumberToGCPath(uint64_t vlog_number){
            return VlogNumberToPath(vlog_number).append("ForGC");
        }

        void CleanFile(string filepath, FileType type){
            vector<string> filenames;
            Env *env = Env::Default();
            env->GetChildren(filepath, &filenames);

            uint64_t number;
            FileType get_type;
            for (auto filename: filenames) {
                if(ParseFileName(filename, &number, &type)){
                    if(get_type == type){
                        env->RemoveFile(filename);
                    }
                }
            }
        }
    }
} // leveldb