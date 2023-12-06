//
// Created by Yunfan Li on 2022/10/5.
//

#include "unittest_utils.h"

RC UnittestUtils::make_test(const char *filename,
                            std::shared_ptr<Replacer> cacheManager) {
    FILE *pFile;
    pFile = fopen(filename, "r");
    if (pFile == nullptr) {
        std::cerr << "can't not find trace_file " << filename << std::endl;
        return RC::FAILED;
    }
    trace_line l;
    while (fscanf(pFile, "%d %d %d %d\n", &l.starting_block,
                  &l.number_of_blocks, &l.ignore, &l.request_number) != EOF) {
        for (auto i = l.starting_block;
             i < (l.starting_block + l.number_of_blocks); ++i) {
            UnittestUtils::check_get(cacheManager.get(), i);
        }
    }
    std::cout << cacheManager->stats();
    return RC::SUCCESS;
}

RC UnittestUtils::check_get(Replacer *cacheManager, Key &key) {
    cacheManager->access(key);
    RC status = cacheManager->check_consistency();
    return status;
}

RC UnittestUtils::get_access_list(const char *filename,
                                  std::vector<Key> &access_list,
                                  int32_t &unique_key_nums) {
    FILE *pFile;
    pFile = fopen(filename, "r");
    if (pFile == nullptr) {
        std::cerr << "Can not find trace_file " << filename << std::endl;
        return RC::FAILED;
    }
    trace_line l;
    std::unordered_set<int32_t> unique_key_set;
    while (fscanf(pFile, "%d %d %d %d\n", &l.starting_block,
                  &l.number_of_blocks, &l.ignore, &l.request_number) != EOF) {
        for (auto i = l.starting_block;
             i < (l.starting_block + l.number_of_blocks); ++i) {
            unique_key_set.insert(i);
            access_list.push_back(i);
        }
    }
    unique_key_nums = unique_key_set.size();
    printf("Unique keys: %ld\n", unique_key_set.size());
    printf("Access size: %ld\n", access_list.size());
    return RC::SUCCESS;
}
