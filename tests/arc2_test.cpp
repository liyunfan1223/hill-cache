//
// Created by Yunfan Li on 2022/10/9.
//

#include <iostream>

#include "arc2_cache_manager.h"
#include "unittest_utils.h"

int main() {
    std::vector<Key> access_list;
    UnittestUtils::get_access_list("../traces/P1.lis", access_list);
    UnittestUtils::make_test("../traces/P1.lis", std::shared_ptr<CacheManager>(new ARC2CacheManager(65536, 0.02, access_list)));
    return 0;
}