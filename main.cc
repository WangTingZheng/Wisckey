#include "test/vlog_test_file.h"
#include <iostream>

int main() {
    leveldb::Status status = leveldb::vlog::VlogTestFile::TestReadAndWrite();
    if(!status.ok()){
        std::cout << status.ToString() << std::endl;
    }
    return 0;
}