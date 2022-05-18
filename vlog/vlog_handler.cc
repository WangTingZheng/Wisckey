//
// Created by 14037 on 2022/5/17.
//

#include "vlog_handler.h"
#include <map>

namespace leveldb {
    namespace vlog {
        using namespace std;

        map<string, string> lsm;

        bool VlogHandler::IsKeyVaild(std::string key) {
            auto iterator = lsm.find(key);
            if (iterator == lsm.end()) return false;
            return true;
        }

        void VlogHandler::Update(std::string key, vlog::VlogFormat::Meta meta) {
            string value;
            VlogFormat::PutMeta(&value, meta.offset, meta.size);
            lsm[key] = value;
        }

        void VlogHandler::Put(string key, string value) {
            lsm[key] = value;
        }

        void VlogHandler::Get(string key, string *value) {
            auto iter = lsm.find(key);
            if(iter != lsm.end())
                *value = iter->second;

        }
    }
} // leveldb