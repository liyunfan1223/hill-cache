//
// Created by Yunfan Li on 2023/4/15.
//

#include <libmemcached/memcached.h>
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <sys/time.h>

#include <csignal>
#include <cstdio>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <thread>

#include "def.h"
#include "getopt.h"

using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::PinnableSlice;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteBatch;
using ROCKSDB_NAMESPACE::WriteOptions;
using namespace std;

const uint32_t MAX_THREAD_NUM = 1 << 12;
// const uint32_t MAX_TOTAL_COUNTER = 1 << 20;
const uint32_t MEGABYTES = 1 << 20;
const char *config_string = "--SERVER=127.0.0.1";
const uint32_t warmup_seconds = 30;

// const uint32_t simulated_network_latency = 5; // 5ms - 10ms for network
// request
uint32_t simulated_network_latency = 0;
int32_t earlyStop = 0;
uint32_t maxLength;
uint32_t threadNum;
std::string kDBPath;
DB *rocksDB;
string traceFile;
uint32_t report_interval = 1 << 14;
string default_str;

double timer[MAX_THREAD_NUM + 1][3];
uint32_t counter[MAX_THREAD_NUM + 1][3];

double memc_timer[MAX_THREAD_NUM + 1];
uint32_t memc_counter[MAX_THREAD_NUM + 1];

bool warming_up[MAX_THREAD_NUM + 1];
timeval start_time[MAX_THREAD_NUM + 1];

vector<int> access_list;
int warming_up_counter = 0;
bool hasWarmup = false;
uint32_t warmup_access = 0;
bool test_finished = false;
pthread_mutex_t stats_mutex;

int global_i;
pthread_mutex_t i_mutex;
bool threadsSync = false;

// std::vector<double> latency_vec;
std::mutex latency_mutex;

double GenerateRandomRTT() {
    std::default_random_engine generator(233);
    // 第一个参数为高斯分布的平均值，第二个参数为标准差
    std::normal_distribution<double> distribution(
        simulated_network_latency, simulated_network_latency / 4);
    return distribution(generator);
}

double GenrateRandomRatio() { return pow(1.25, rand() % 10000 / 2500.0); }

DB *rocksdb_create() {
    DB *db;
    Options options;
    options.use_direct_reads = true;
    options.use_direct_io_for_flush_and_compaction = true;
    options.IncreaseParallelism(threadNum);
    options.max_background_jobs = threadNum * 4;
    // 文件夹没有数据就创建
    options.create_if_missing = true;
    // 打开数据库，加载数据到内存
    Status s = DB::Open(options, kDBPath, &db);
    return db;
}

bool request_from_memcached(const char *key, string &value, memcached_st *memc,
                            int32_t thread_id) {
    memcached_return_t ret;
    size_t value_len;
    uint32_t flags;
    uint32_t key_length = strlen(key);

    const auto start = std::chrono::high_resolution_clock::now();

    char *v = memcached_get(memc, key, key_length, &value_len, &flags, &ret);
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::milli> elapsed = end - start;
    double time = elapsed.count();  // ms
    memc_counter[thread_id]++;
    memc_timer[thread_id] += time;

    if (ret == MEMCACHED_SUCCESS) {
        value = v;
        // must free manually
        free(v);
    }
    return ret == MEMCACHED_SUCCESS;
}

bool save_to_memcached(const char *key, string &value, uint32_t v_len,
                       memcached_st *memc, int thread_id) {
    memcached_return_t ret;
    size_t value_len;
    uint32_t flags;
    uint32_t key_length = strlen(key);
    string val = value.substr(0, v_len);
    timeval start_time, end_time;
    gettimeofday(&start_time, nullptr);
    ret = memcached_set(memc, key, key_length, val.c_str(), v_len, 0, 0);
    gettimeofday(&end_time, nullptr);
    double time = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                  (end_time.tv_usec - start_time.tv_usec) / 1000.0;  // ms
    memc_counter[thread_id]++;
    memc_timer[thread_id] += time;
    return ret == 0;
}

bool request_from_rocksdb(const char *key, string &value) {
    std::this_thread::sleep_for(GenerateRandomRTT() * 1ms);
    Status status = rocksDB->Get(ReadOptions(), key, &value);
    return status.ok();
}

bool save_to_rocksdb(const char *key, string &value, uint32_t v_len) {
    Status status = rocksDB->Put(WriteOptions(), key, value.substr(0, v_len));
    return status.ok();
}

enum RequestResult {
    in_memcached,
    in_rocksdb,
    not_found,
    unknown,
};

RequestResult do_request_item(const char *key, memcached_st *memc,
                              int32_t thread_id) {
    string value;
    if (request_from_memcached(key, value, memc, thread_id)) {
        return in_memcached;
    }
    if (request_from_rocksdb(key, value)) {
        save_to_memcached(key, value, value.length(), memc, thread_id);
        return in_rocksdb;
    }
    uint32_t v_len = rand() % (int)(maxLength * 0.6) + maxLength * 0.4 + 1;
    //    uint32_t v_len = maxLength;
    save_to_rocksdb(key, default_str, v_len);
    save_to_memcached(key, default_str, v_len, memc, thread_id);
    return not_found;
}

RequestResult request_item(const char *key, int thread_id, memcached_st *memc) {
    timeval start_time, end_time;
    //    gettimeofday(&start_time, NULL);
    const auto start = std::chrono::high_resolution_clock::now();
    RequestResult rr;
    rr = do_request_item(key, memc, thread_id);
    //    gettimeofday(&end_time, NULL);
    //    double time = (end_time.tv_sec - start_time.tv_sec) * 1000 +
    //    (end_time.tv_usec - start_time.tv_usec) / 1000.0; // ms
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::milli> elapsed = end - start;
    counter[thread_id][rr]++;
    timer[thread_id][rr] += elapsed.count();
    //    if (rand() % 100 <= 10) {
    //        if (latency_mutex.try_lock()) {
    //            latency_vec.push_back(elapsed.count());
    //            latency_mutex.unlock();
    //        }
    //    }
    return rr;
}

struct ThreadArg {
    int pid;
};

void *subprocess_work(void *arg) {
    int thread_id = ((ThreadArg *)arg)->pid;
    printf("Thread-%d Started.\n", thread_id);
    memcached_st *memc = memcached(config_string, strlen(config_string));
    if (!memc) {
        cerr << "Memcached initialization failed!" << std::endl;
        return nullptr;
    }
    timeval end_time;
    int i = thread_id;
    while (threadsSync ? global_i < access_list.size()
                       : i < access_list.size()) {
        if (threadsSync) {
            pthread_mutex_lock(&i_mutex);
            int now_i = global_i;
            global_i++;
            pthread_mutex_unlock(&i_mutex);
            request_item(to_string(access_list[now_i]).c_str(), thread_id,
                         memc);
        } else {
            request_item(to_string(access_list[i]).c_str(), thread_id, memc);
            i += threadNum;
        }
        gettimeofday(&end_time, NULL);
        if (hasWarmup) {
            if (!warming_up[thread_id]) {
                if (counter[thread_id][0] + counter[thread_id][1] +
                        counter[thread_id][2] >=
                    warmup_access / threadNum) {
                    pthread_mutex_lock(&stats_mutex);
                    warming_up[thread_id] = true;
                    for (int j = 0; j < 3; j++) {
                        timer[thread_id][j] = 0;
                        counter[thread_id][j] = 0;
                        memc_counter[thread_id] = 0;
                        memc_timer[thread_id] = 0;
                    }
                    warming_up_counter++;
                    printf("Thread-%d warmed up.\n", thread_id);
                    gettimeofday(&start_time[thread_id], NULL);
                    if (warming_up_counter == threadNum) {
                        printf("All threads warmed up, clean stats.\n");
                    }
                    pthread_mutex_unlock(&stats_mutex);
                }
            }
        }
        // 0 for public
        if ((counter[thread_id][0] + counter[thread_id][1] +
             counter[thread_id][2]) %
                    (report_interval / threadNum) ==
                0 &&
            thread_id == 1) {
            pthread_mutex_lock(&stats_mutex);
            double total_time =
                0;  // (end_time.tv_sec - start_time[thread_ud].tv_sec) +
            // (end_time.tv_usec - start_time.tv_usec) / 1000000.0; //s
            for (int j = 1; j <= threadNum; j++) {
                total_time +=
                    end_time.tv_sec - start_time[j].tv_sec +
                    (end_time.tv_usec - start_time[j].tv_usec) / 1000000.0;
            }
            total_time /= threadNum;
            for (int k = 0; k < 3; k++) {
                timer[0][k] = 0;
                counter[0][k] = 0;
                memc_counter[0] = 0;
                memc_timer[0] = 0;
                for (int j = 1; j <= threadNum; j++) {
                    timer[0][k] += timer[j][k];
                    counter[0][k] += counter[j][k];
                    memc_counter[0] += memc_counter[j];
                    memc_timer[0] += memc_timer[j];
                }
            }
            uint32_t tot_counter =
                counter[0][0] + counter[0][1] + counter[0][2];
            double average_latency =
                (timer[0][0] + timer[0][1] + timer[0][2]) / tot_counter;
            double mem_latency =
                memc_counter[0] ? memc_timer[0] / memc_counter[0] : 0;
            double rdb_latency =
                counter[0][1] ? timer[0][1] / counter[0][1] : 0;
            double nf_latency = counter[0][2] ? timer[0][2] / counter[0][2] : 0;
            double throughput_req = tot_counter / total_time;
            double throughput_mb =
                throughput_req * (maxLength / 4 * 3) / MEGABYTES;
            double hit_ratio = (double)counter[0][0] / tot_counter * 100;
            double throughput_mem = throughput_mb * hit_ratio / 100;
            double throughput_rdb = throughput_mb * (100 - hit_ratio) / 100;
            printf(
                "runtime: %.2fs "
                "warming up: %d "
                "average latency: %.4f "
                "mem: %.4f "
                "rdb: %.4f "
                "nf: %.4f "
                "tps: %.2f tps_mb: %.2f tps_mem: %.2f tps_rdb: %.2f "
                "h_ratio: %.2f%% t_counter: %u "
                "mem:rdb:nf=%d:%d:%d\n",
                total_time, warming_up_counter != threadNum, average_latency,
                mem_latency, rdb_latency, nf_latency, throughput_req,
                throughput_mb, throughput_mem, throughput_rdb, hit_ratio,
                tot_counter, counter[0][0], counter[0][1], counter[0][2]);
            fflush(stdout);

            pthread_mutex_unlock(&stats_mutex);
            if (earlyStop && tot_counter > earlyStop) {
                test_finished = true;
            }
        }
        if (test_finished && earlyStop) return 0;
    }
    delete memc;
    // small trace
    if (hasWarmup) {
        if (!warming_up[thread_id]) {
            pthread_mutex_lock(&stats_mutex);
            warming_up[thread_id] = true;
            for (int j = 0; j < 3; j++) {
                timer[thread_id][j] = 0;
                counter[thread_id][j] = 0;
                memc_counter[0] += memc_counter[j];
                memc_timer[0] += memc_timer[j];
            }
            gettimeofday(&start_time[thread_id], NULL);
            warming_up_counter++;
            if (warming_up_counter == threadNum) {
                printf("All threads warmed up, clean stats.\n");
            }
            pthread_mutex_unlock(&stats_mutex);
        }
    }
    return nullptr;
}

void usage() {
    std::cout << "Usage: ./bin/bench [options]\n"
              << "Options:\n"
              << "  -h, --help                          Show help message\n"
              << "  -t, --thread-number <num>           Number of threads\n"
              << "  -l, --max-data-length <len>         Maximum data length\n"
              << "  -f, --trace-file <file>             Trace file\n"
              << "  -e, --early-stop-access <num>       Early stop access\n"
              << "  -s, --sync-threads <bool>           Synchronize threads\n"
              << "  -w, --warm-up-access <num>          Warm-up access\n"
              << "  -m, --manual-remote-latency <num>   Set the manual remote "
                 "latency\n"
              << "  -d, --rocksdb-path <path>           Set rocksdb path\n"
              << "  -r, --report-interval <num>         Report interval\n";
}

int main(int argc, char *argv[]) {
    printf("Bench started.\n");
    int opt;
    const char *short_options = "ht:l:f:e:sw:m:d:r:";
    const option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"thread number", required_argument, nullptr, 't'},
        {"maximum data length", required_argument, nullptr, 'l'},
        {"trace file", required_argument, nullptr, 'f'},
        {"early stop access", required_argument, nullptr, 'e'},
        {"sync threads", no_argument, nullptr, 's'},
        {"warm up access", required_argument, nullptr, 'w'},
        {"manual remote latency", required_argument, nullptr, 'm'},
        {"rocksdb path", required_argument, nullptr, 'd'},
        {"report interval", required_argument, nullptr, 'r'},
        {nullptr, 0, nullptr, 0}};
    while ((opt = getopt_long(argc, argv, short_options, long_options,
                              nullptr)) != -1) {
        switch (opt) {
            case 'h':
                usage();
                return 0;
            case 't':
                threadNum = stoi(optarg);
                if (threadNum <= 0 || threadNum > MAX_THREAD_NUM) {
                    cerr << "Threads number invalid. (range from 1 to "
                         << MAX_THREAD_NUM - 1 << ")\n";
                    return 0;
                }
                break;
            case 'l':
                maxLength = stoi(optarg);
                break;
            case 'f':
                traceFile = optarg;
                break;
            case 'e':
                earlyStop = stoi(optarg);
                break;
            case 's':
                threadsSync = true;
                break;
            case 'w':
                hasWarmup = true;
                warmup_access = stoi(optarg);
                break;
            case 'm':
                simulated_network_latency = stoi(optarg);
                break;
            case 'd':
                kDBPath = optarg;
                break;
            case 'r':
                report_interval = stoi(optarg);
                break;
            case '?':
            default:
                std::cerr << "Unknown option\n";
                return 0;
        }
    }
    if (kDBPath.empty()) {
        string str_length;
        if (maxLength % 1024 == 0) {
            str_length = to_string(maxLength / 1024) + 'k';
        } else {
            str_length = to_string(maxLength);
        }
        kDBPath = "/tmp/rocksdb";
    }
    /* initialize connection of rocksdb & memcached */
    rocksDB = rocksdb_create();
    if (!rocksDB) {
        cerr << "RocksDB initialization failed!" << std::endl;
        return 0;
    }
    /* generate default string */
    for (int i = 0; i < maxLength + 1; i++) {
        default_str += rand() % 26 + 'a';
    }
    /* get trace */
    FILE *pFile;
    //    std::string filename;
    //    filename = "traces/" + traceFile + ".lis";
    pFile = fopen(traceFile.c_str(), "r");
    if (pFile == nullptr) {
        cerr << "File " << traceFile << " open failed!" << std::endl;
        return 0;
    }
    trace_line l;
    while (fscanf(pFile, "%d %d %d %d\n", &l.starting_block,
                  &l.number_of_blocks, &l.ignore, &l.request_number) != EOF) {
        for (auto i = l.starting_block;
             i < (l.starting_block + l.number_of_blocks); ++i) {
            access_list.push_back(i);
        }
    }
    for (int i = 1; i <= threadNum; i++) gettimeofday(&start_time[i], NULL);

    pthread_t threads[threadNum];

    for (int i = 0; i < threadNum; i++) {
        ThreadArg *targ = new ThreadArg();
        targ->pid = i + 1;
        pthread_create(&threads[i], NULL, subprocess_work, (void *)(targ));
        pthread_setname_np(threads[i], ("THREAD-" + to_string(i + 1)).c_str());
    }
    //    pthread_join(threads[0], NULL);
    for (int i = 0; i < threadNum; i++) {
        pthread_join(threads[i], NULL);
    }
    //    printf("Sampled %zu, now calculating tail latency...\n",
    //           latency_vec.size());
    //    std::sort(latency_vec.begin(), latency_vec.end());
    //    static std::vector<double> stastic_percentiles;
    //    for (int i = 0; i <= 100; i++) {
    //        stastic_percentiles.push_back(i / 100.0);
    //    }
    //    for (double percentile : stastic_percentiles) {
    //        printf("Percentage %.8f%%: %.8fms\n", percentile * 100,
    //               latency_vec[(int)(latency_vec.size() * percentile)]);
    //    }
    delete rocksDB;
    return 0;
}