//
// Created by 14037 on 2022/5/17.
//

#ifndef VLOG_VLOG_WISCKEY_H
#define VLOG_VLOG_WISCKEY_H

#include <cstdint>
#include "vlog_format.h"
#include "vlog_file.h"
#include "vlog_handler.h"
#include "../db/write_batch_internal.h"

namespace leveldb {

    namespace vlog {
        class VlogWisckey {
            public:
                VlogWisckey(string pathname, size_t gc_threshold, VlogHandler * handler);
                Status AddBatch(WriteBatch *batch);
                Status Read(string key, string *meta_val, string *value); //输入key-meta从vlog中取出value，输入key是为了校验

            private:
                VlogFile *file_;
                VlogHandler *handler_;
                uint64_t gc_threshold_;

                uint64_t head_;
                uint64_t tail_;

                Status StartGC();
                uint64_t GetSize(){return head_ - tail_;}
        };
    }

} // leveldb

#endif //VLOG_VLOG_WISCKEY_H
