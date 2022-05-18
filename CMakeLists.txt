cmake_minimum_required(VERSION 3.16)
project(vLog)

set(CMAKE_CXX_STANDARD 11)


add_executable(vLog
        main.cc

        vlog/vlog_file.cc
        vlog/vlog_file.h

        vlog/vlog_format.cc
        vlog/vlog_format.h

        vlog/vlog_wisckey.cc
        vlog/vlog_wisckey.h

        vlog/vlog_handler.cc
        vlog/vlog_handler.h

        vlog/vlog_writer.h
        vlog/vlog_writer.cc

        vlog/vlog_reader.h
        vlog/vlog_reader.cc

        util/crc32c.cc
        util/crc32c.h
        util/coding.h
        util/coding.cc
        util/status.cc
        util/env_posix.cc
        util/env.cc
        util/logging.h
        util/logging.cc


        db/write_batch.cc
        db/write_batch_internal.h
        db/filename.cc
        db/filename.h
        db/vlog_filename.cc
        db/vlog_filename.h

        include/leveldb/write_batch.h
        include/leveldb/status.h
        include/leveldb/env.h

        test/vlog_test_format.cc
        test/vlog_test_format.h
        test/vlog_test_file.cc
        test/vlog_test_file.h
        test/vlog_test_wisckey.cc
        test/vlog_test_wisckey.h
        )

target_link_libraries(vLog pthread)