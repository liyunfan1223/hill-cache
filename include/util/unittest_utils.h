//
// Created by Yunfan Li on 2022/10/5.
//

#pragma once

#include "def.h"
#include "replacer.h"

#ifndef HILL_CACHE_UNITTEST_UTILS_H
#define HILL_CACHE_UNITTEST_UTILS_H

class UnittestUtils {
   public:
    static RC make_test(const char *filename,
                        std::shared_ptr<Replacer> cacheManager);
    static RC make_test(std::vector<int32_t> access_order,
                        std::shared_ptr<Replacer> cacheManager);
    static RC check_get(Replacer *cacheManager, Key &key);
    static RC get_access_list(const char *filename,
                              std::vector<Key> &access_list);
    static RC get_access_list(const char *filename,
                              std::vector<Key> &access_list,
                              int32_t &unique_key_nums);
    const char *DEFAULT_TRACE_PATH = "../traces/P1.lis";
    const int32_t DEFAULT_BUFFER_SIZE = 65536;
};

#endif  //