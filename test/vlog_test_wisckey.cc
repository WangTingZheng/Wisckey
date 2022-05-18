//
// Created by 14037 on 2022/5/18.
//

#include "vlog_test_wisckey.h"
#include "../vlog/vlog_wisckey.h"
#include <map>

namespace leveldb {
    namespace vlog{

        Status VlogTestWisckey::RunTest() {
            map<string, string> test_case;

            test_case["key1"] = "value1";
            test_case["key2"] = "value2";
            test_case["key3"] = "value3";
            test_case["key4"] = "value4";
            test_case["key5"] = "value5";
            test_case["key6"] = "value6";
            test_case["key7"] = "value7";
            test_case["key8"] = "value8";

            auto *handler = new VlogHandler();
            auto *vlogWisckey = new VlogWisckey("./1.vlogfile", 400, handler);
            auto *batch = new WriteBatch();

            for(auto item: test_case){
                batch->Put(item.first, item.second);
            }
            Status status = vlogWisckey->AddBatch(batch);
            if(!status.ok()) return status;

            for(auto check:test_case) {
                // 从map中读取key对应的meta
                string value;
                handler->Get(check.first, &value);

                // 解析meta为结构体
                VlogFormat::Meta meta{};
                VlogFormat::GetMeta(&value, &meta);

                // 根据meta从vlog中读取value
                string get_value;
                status = vlogWisckey->Read(check.first, &value, &get_value);
                if(!status.ok()) return status;

                // 检查是否读取准确
                if(get_value != check.second) return Status::Corruption("get value is not matched:", get_value);
            }

            return Status::OK();
        }
    }
} // leveldb