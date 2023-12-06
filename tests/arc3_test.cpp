//
// Created by l50029536 on 2022/10/13.
//

#include <iostream>

#include "arc3_cache_manager.h"
#include "unittest_utils.h"

int main() {
    std::vector<Key> access_list;
    UnittestUtils::get_access_list("../traces/P1.lis", access_list);
    UnittestUtils::make_test("../traces/P1.lis", std::shared_ptr<CacheManager>(new ARC3CacheManager(65536, 8192, access_list)));
    return 0;
}