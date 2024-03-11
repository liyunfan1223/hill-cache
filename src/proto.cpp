#include <iostream>
#include <memory>

#include "arc_replacer.h"
#include "cacheus_replacer.h"
#include "def.h"
#include "dlirs_replacer.h"
#include "drrip_replacer.h"
#include "efsw_replacer.h"
#include "getopt.h"
#include "hill_replacer.h"
#include "lfu_replacer.h"
#include "lirs_replacer.h"
#include "lrfu_replacer.h"
#include "lru_replacer.h"
#include "opt_replacer.h"
#include "s3fifo_replacer.h"
#include "srrip_replacer.h"
#include "unistd.h"
#include "unittest_utils.h"

std::unordered_map<std::string, CachePolicy> cachePolicy = {
    {"LRU", CachePolicy::LRU},         {"LFU", CachePolicy::LFU},
    {"ARC", CachePolicy::ARC},         {"OPT", CachePolicy::OPT},
    {"SRRIP", CachePolicy::SRRIP},     {"DRRIP", CachePolicy::DRRIP},
    {"EFSW", CachePolicy::EFSW},       {"LRFU", CachePolicy ::LRFU},
    {"LIRS", CachePolicy::LIRS},       {"DLIRS", CachePolicy::DLIRS},
    {"CACHEUS", CachePolicy::CACHEUS}, {"HILL", CachePolicy::HILL},
    {"S3FIFO", CachePolicy::S3FIFO}};

void usage() {
    std::cout << "Usage: ./proto [options]\n"
              << "Options:\n"
              << "  -h, --help             Show this help message\n"
              << "  -c, --cache-policy     Cache policy (LRU, LFU, ARC, SRRIP, "
                 "DRRIP, LRFU, LIRS, DLIRS, CACHEUS, S3FIFO, HILL, OPT)\n"
              << "  -b, --buffer-size      Buffer size (expressed as a decimal "
                 "in proportion to footprint)\n"
              << "  -t, --trace-file       Trace file path\n"
              << "  -s, --stats-interval   Statistics reporting interval\n";
}

int main(int argc, char **argv) {
    std::string cache_policy("LRU");
    float buffer_param = 256;
    const char *trace_file = "../traces/example.lis";
    int32_t stats_interval = -1;
    int opt;
    const char *short_options = "hc:b:t:s:";
    const option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"cache policy", required_argument, nullptr, 'c'},
        {"buffer size", required_argument, nullptr, 'b'},
        {"trace file", required_argument, nullptr, 't'},
        {"stats interval", required_argument, nullptr, 's'},
        {nullptr, 0, nullptr, 0}};
    while ((opt = getopt_long(argc, argv, short_options, long_options,
                              nullptr)) != -1) {
        switch (opt) {
            case 'h':
                usage();
                return 0;
            case 'c':
                if (cachePolicy.find(std::string(optarg)) ==
                    cachePolicy.end()) {
                    std::cout << "Invalid cache policy."
                              << "\n";
                    return 0;
                }
                cache_policy = std::string(optarg);
                break;
            case 'b':
                buffer_param = std::stof(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            case 's':
                stats_interval = std::stoi(optarg);
                break;
            case '?':
            default:
                std::cerr << "Unknown option\n";
                return 0;
        }
    }

    std::vector<Key> access_list;
    int unique_key_nums;
    if (UnittestUtils::get_access_list(trace_file, access_list,
                                       unique_key_nums) != RC::SUCCESS) {
        return 0;
    }

    int32_t buffer_size =
        buffer_param > 1 ? (int)buffer_param
                         : std::max(1, (int)(buffer_param * unique_key_nums));

    int aargc = argc - optind;
    char *params[256];
    for (int i = optind; i < argc; ++i) {
        params[i - optind] = argv[i];
    }
    timeval start_time;
    gettimeofday(&start_time, nullptr);
    switch (cachePolicy.at(cache_policy)) {
        case CachePolicy::LRU:
            UnittestUtils::make_test(
                trace_file,
                std::make_shared<LRUReplacer>(buffer_size, stats_interval));
            break;
        case CachePolicy::LFU:
            UnittestUtils::make_test(
                trace_file,
                std::make_shared<LFUReplacer>(buffer_size, stats_interval));
            break;
        case CachePolicy::ARC:
            UnittestUtils::make_test(
                trace_file,
                std::make_shared<ARCReplacer>(buffer_size, stats_interval));
            break;
        case CachePolicy::OPT:
            UnittestUtils::make_test(
                trace_file, std::make_shared<OPTReplacer>(
                                buffer_size, stats_interval, access_list));
            break;
        case CachePolicy::SRRIP:
            if (aargc <= BASIC_MAIN_ARG_NUM) {
                UnittestUtils::make_test(trace_file,
                                         std::make_shared<SRRIPReplacer>(
                                             buffer_size, stats_interval));
            } else {
                UnittestUtils::make_test(
                    trace_file,
                    std::make_shared<SRRIPReplacer>(buffer_size, stats_interval,
                                                    std::stof(params[0])));
            }
            break;
        case CachePolicy::DRRIP:
            if (aargc <= BASIC_MAIN_ARG_NUM) {
                UnittestUtils::make_test(trace_file,
                                         std::make_shared<DRRIPReplacer>(
                                             buffer_size, stats_interval));
            } else {
                UnittestUtils::make_test(
                    trace_file,
                    std::make_shared<DRRIPReplacer>(buffer_size, stats_interval,
                                                    std::stof(params[0])));
            }
            break;
        case CachePolicy::EFSW:
            if (aargc <= BASIC_MAIN_ARG_NUM) {
                UnittestUtils::make_test(
                    trace_file, std::make_shared<EFSWReplacer>(buffer_size,
                                                               stats_interval));
            } else {
                UnittestUtils::make_test(
                    trace_file,
                    std::make_shared<EFSWReplacer>(
                        buffer_size, stats_interval, std::stof(params[0]),
                        std::stof(params[1]), std::stof(params[2])));
            }
            break;
        case CachePolicy::LRFU:
            if (aargc <= BASIC_MAIN_ARG_NUM) {
                UnittestUtils::make_test(
                    trace_file, std::make_shared<LRFUReplacer>(buffer_size,
                                                               stats_interval));
            } else if (aargc <= BASIC_MAIN_ARG_NUM + 1) {
                UnittestUtils::make_test(
                    trace_file,
                    std::make_shared<LRFUReplacer>(buffer_size, stats_interval,
                                                   std::stof(params[0])));
            } else {
                UnittestUtils::make_test(
                    trace_file,
                    std::make_shared<LRFUReplacer>(buffer_size, stats_interval,
                                                   std::stof(params[0]),
                                                   std::stof(params[1])));
            }
            break;
        case CachePolicy::LIRS:
            if (aargc <= BASIC_MAIN_ARG_NUM) {
                UnittestUtils::make_test(
                    trace_file, std::make_shared<LIRSReplacer>(buffer_size,
                                                               stats_interval));
            } else {
                UnittestUtils::make_test(
                    trace_file,
                    std::make_shared<LIRSReplacer>(buffer_size, stats_interval,
                                                   std::stof(params[0])));
            }
            break;
        case CachePolicy::DLIRS:
            if (aargc <= BASIC_MAIN_ARG_NUM) {
                UnittestUtils::make_test(trace_file,
                                         std::make_shared<DLIRSReplacer>(
                                             buffer_size, stats_interval));
            } else {
                UnittestUtils::make_test(
                    trace_file,
                    std::make_shared<DLIRSReplacer>(buffer_size, stats_interval,
                                                    std::stof(params[0])));
            }
            break;
        case CachePolicy::CACHEUS:
            UnittestUtils::make_test(
                trace_file,
                std::make_shared<CACHEUSReplacer>(buffer_size, stats_interval));
            break;
        case CachePolicy::S3FIFO:
            UnittestUtils::make_test(
                trace_file,
                std::make_shared<S3FIFOReplacer>(buffer_size, stats_interval));
            break;
        case CachePolicy::HILL:
            std::cout << aargc;
            if (aargc <= BASIC_MAIN_ARG_NUM) {
                UnittestUtils::make_test(
                    trace_file, std::make_shared<HillReplacer>(buffer_size,
                                                               stats_interval));
            } else {
                UnittestUtils::make_test(
                    trace_file,
                    std::make_shared<HillReplacer>(
                        buffer_size, stats_interval, std::stof(params[0]),
                        std::stof(params[1]), std::stof(params[2]),
                        std::stof(params[3]), std::stof(params[4]),
                        std::stof(params[5]), std::stof(params[6]),
                        std::stof(params[7]), std::stof(params[8]),
                        std::stof(params[9]), std::stof(params[10])));
            }
            break;
        default:
            break;
    }
    timeval end_time;
    gettimeofday(&end_time, NULL);
    return 0;
}
