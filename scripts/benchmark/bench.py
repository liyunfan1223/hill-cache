import subprocess
import time

THREADS_LIST = [512, 256, 128, 64, 32, 16, 8, 4, 2, 1]
LOGFILE = 'log1201'
results = []

def bench(thread_num, value_size, memcached_mem, trace_file, memc_suffix, server_thread, early_stop=True, threads_sync=True, has_warmup=True, est_item_counts=0):
    print(f"Started. Args: threads-{thread_num} value_size-{value_size} memcached_mem-{memcached_mem} server_thread-{server_thread} "
          f"trace-{trace_file} memc_suffix-{memc_suffix} earle_stop-{early_stop} threads_sync-{threads_sync} has_warmup-{has_warmup}")
    params = [f"/tmp/memcached/memcached-{memc_suffix}", "-m", f"{memcached_mem}", "-o", "no_lru_crawler", "-o", "no_lru_maintainer", "-t", f"{server_thread}"]
    if est_item_counts != 0:
        params.append("-E")
        params.append(f"{est_item_counts}")
    memc_process = subprocess.Popen(params)
    # print(1)
    time.sleep(3)
    # print(5)
    bench_process = subprocess.Popen(["build/rocksdb_use_multi_threads", f"{thread_num}", f"{value_size}", f"{trace_file}",
                                      f"{1 if early_stop else 0}", f"{1 if threads_sync else 0}", f"{1 if has_warmup else 0} 1", "10"],
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, errors = bench_process.communicate()
    if len(errors) > 0:
        print("Error occurred!", errors)
    else:
        try:
            result = output.decode('utf-8')
            # print(result)
            result = result.split("\n")[-110:]
            result_str = ''
            for item in result:
                result_str += item + '\n'
            log = f"Time: {time.time()} Args: threads-{thread_num} value_size-{value_size} memcached_mem-{memcached_mem} server_thread-{server_thread} "\
                  f"trace-{trace_file} memc_suffix-{memc_suffix} earle_stop-{early_stop} threads_sync-{threads_sync} has_warmup-{has_warmup} \n{result_str}"
            # results.append(log)
            # result_str = log + result_str
            print("result: ", log)
            with open(f"local/{LOGFILE}.txt", "a") as f:
                f.write(f"{log}\n")
        except Exception as e:
            with open(f"local/{LOGFILE}.txt", "a") as f:
                f.write(f"Error occurred: {e}\n")
            print(f"Error occurred: {e}")
    memc_process.terminate()
    memc_process.wait()
    print("Finished.")

def bench_memtier( memcached_mem, memc_suffix, threads, data_size, server_thread, est_item_counts=0):
    args = f"memcached_mem-{memcached_mem} memc_suffix-{memc_suffix} threads-{threads} server_thread-{server_thread}"
    print(f"args: {args}")
    params = [f"/tmp/memcached/memcached-{memc_suffix}", "-m", f"{memcached_mem}", "-o", "no_lru_crawler", "-o", "no_lru_maintainer", "-u", "ubuntu22", "-t", f"{server_thread}"]
    if est_item_counts != 0:
        params.append("-E")
        params.append(f"{est_item_counts}")
    memc_process = subprocess.Popen(params)
    time.sleep(3)
    # tier = subprocess.Popen(["memtier_benchmark", "-t", f"{threads}", "-c", "1", f"--requests={300000}", "--ratio=10:1", f"--data-size={data_size}", "--protocol=memcache_binary",
    #                          "--key-pattern=G:G", "--key-maximum", "3000000", "--key-stddev=2500", "-s", "127.0.0.1", "-p", "11211", "--distinct-client-seed"], stdout=subprocess.PIPE)
    tier = subprocess.Popen(["memtier_benchmark", "-t", f"{threads}", "-c", "1", f"--requests={300000}", "--ratio=5:5", f"--data-size={data_size}", "--protocol=memcache_binary",
                             "--key-pattern=R:R", "--key-maximum", "65536", "-s", "127.0.0.1", "-p", "11211", "--distinct-client-seed"], stdout=subprocess.PIPE)
    output, errors = tier.communicate()
    output = output.decode('utf-8')

    print(output.split('\n')[7:13])
    outputs = output.split('\n')[7:13]
    with open(f"local/{LOGFILE}_memtier.txt", "a") as f:
        f.write(f"{args}\n")
        for o in outputs:
            f.write(f"{o}\n")
        f.write("\n")
    memc_process.terminate()
    memc_process.wait()
    print("Finished.")

if __name__ == "__main__":


    # bench_memtier(4, "rgc", 128, 1024)
    # bench_memtier(4, "lru", 128, 1024)
    # for thread in [16, 32, 64, 128, 256]:
    #     bench_memtier(128, "lru", thread, 32768)
    #     bench_memtier(128, "rgc", thread, 32768, 4096)

    # for thread in [16, 32, 64, 128, 256]:
    #     bench_memtier(4, "lru", thread, 1024)
    #     bench_memtier(4, "rgc", thread, 1024, 4096)

    # for server_thread in [2, 4, 6, 8, 10, 12]:
    #     for thread in [16]:
    #         bench_memtier(64, "lru", thread, 1024, server_thread)
    #         bench_memtier(64, "rgc", thread, 1024, server_thread, 65536)

    # for thread in THREADS_LIST:
    # # for thread in [1]:
    #     bench_memtier(4, "lru", thread, 1024)
    #     bench_memtier(4, "rgc", thread, 1024, 4096)


    # """ online 24M + 1K """
    # # for threads in THREADS_LIST:
    # for size in [24]:
    #     for threads in [512, 256, 128, 64]:
    #         for server_thread in [2, 4, 6, 8, 10, 12]:
    #             for trace_file in ["online"]:
    #                 bench(threads, 1 * 1024, 24, trace_file, "lru2", server_thread, early_stop=False, has_warmup=False)
    #                 bench(threads, 1 * 1024, 24, trace_file, "rgc2", server_thread, early_stop=False, has_warmup=False, est_item_counts=32768)

    """ online 72M + 1K """
    # for threads in THREADS_LIST:
    for size in [24, 48, 72]:
        for threads in [128]:
            for server_thread in [8]:
                for trace_file in ["online"]:
                    bench(threads, 1 * 1024, size, trace_file, "lru2", server_thread, early_stop=False, has_warmup=False)
                    bench(threads, 1 * 1024, size, trace_file, "rgc2", server_thread, early_stop=False, has_warmup=False, est_item_counts=int(size / 24 * 32768))


    # """ online 768M + 32K """
    # for threads in THREADS_LIST:
    # for size in [24]:
    #     for threads in [512, 256, 128]:
    #         est = 32768 / 24 * size
    #         for trace_file in ["online"]:
    #             bench(threads, 32 * 1024, 768, trace_file, "lru", early_stop=True, has_warmup=False)
    #             bench(threads, 32 * 1024, 768, trace_file, "rgc", early_stop=True, has_warmup=False, est_item_counts=est)


    # """ Home3 24M + 1K """
    # for threads in THREADS_LIST:
    #     for trace_file in ["Home3"]:
    #         bench(threads, 1 * 1024, 24, trace_file, "lru", early_stop=False, has_warmup=False)
    #         bench(threads, 1 * 1024, 24, trace_file, "rgc", early_stop=False, has_warmup=False, est_item_counts=32768)

    # """ OLTP 216M+256K """
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 256 * 1024, 216, trace_file, "lru", early_stop=False, has_warmup=False)
    #         bench(threads, 256 * 1024, 216, trace_file, "rgc10", early_stop=False, has_warmup=False)

    # """ OLTP 3M+8K """
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 8 * 1024, 3, trace_file, "lru", early_stop=False, has_warmup=False)
    #         bench(threads, 8 * 1024, 3, trace_file, "rgc10", early_stop=False, has_warmup=False)

    # """ OLTP 32M+32K """
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 32 * 1024, 32, trace_file, "lru", early_stop=False, has_warmup=False)
    #         bench(threads, 32 * 1024, 32, trace_file, "rgc10", early_stop=False, has_warmup=False)

    # """ OLTP 4M+4K """
    # for threads in [4]:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 4 * 1024, 4, trace_file, "lru", early_stop=False, has_warmup=False)
    #         bench(threads, 4 * 1024, 4, trace_file, "rgc10", early_stop=False, has_warmup=False)

    # """ OLTP 8M+32K """
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 32 * 1024, 8, trace_file, "lru", early_stop=False, has_warmup=False)
    #         bench(threads, 32 * 1024, 8, trace_file, "rgc10", early_stop=False, has_warmup=False)
    #
    # """ OLTP 8M+32K """
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 32 * 1024, 8, trace_file, "lru", early_stop=False, has_warmup=False)
    #         bench(threads, 32 * 1024, 8, trace_file, "rgc10", early_stop=False, has_warmup=False)

    # """ OLTP 12M+32K """
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 256 * 1024, 96, trace_file, "lru", early_stop=False, has_warmup=False)
    #         bench(threads, 256 * 1024, 96, trace_file, "rgc10", early_stop=False, has_warmup=False)


    # """ OLTP 3M+8K """
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 8 * 1024, 3, trace_file, "lru", early_stop=False, has_warmup=False)
    #         bench(threads, 8 * 1024, 3, trace_file, "rgc10", early_stop=False, has_warmup=False)
    #
    # """ OLTP 12M+32K """
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 32 * 1024, 12, trace_file, "lru", early_stop=False, has_warmup=False)
    #         bench(threads, 32 * 1024, 12, trace_file, "rgc10", early_stop=False, has_warmup=False)

    # """ OLTP 32M+32K """
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 32 * 1024, 32, trace_file, "lru", early_stop=False, has_warmup=False)
    #         bench(threads, 32 * 1024, 32, trace_file, "rgc10", early_stop=False, has_warmup=False)

    # """ P1 64M+1K """
    # for trace_file in ["P1"]:
    #     for threads in THREADS_LIST:
    #         bench(thread_num=threads, value_size=1024, memcached_mem=64, trace_file=trace_file, memc_suffix="lru", early_stop=True)
    #         bench(thread_num=threads, value_size=1024, memcached_mem=64, trace_file=trace_file, memc_suffix="rgc16", early_stop=True)

    # """ P1 2048M+32K """
    # for trace_file in ["P1"]:
    #     for threads in THREADS_LIST:
    #         bench(thread_num=threads, value_size=32*1024, memcached_mem=2048, trace_file=trace_file, memc_suffix="lru", early_stop=True)
    #         bench(thread_num=threads, value_size=32*1024, memcached_mem=2048, trace_file=trace_file, memc_suffix="rgc16", early_stop=True)

    # for threads in [128, 32, 16, 4, 64, 8, 1, 2]:
    #     bench(threads, 1024, 6144, "DS1", "rgc20", early_stop=False)
    #     bench(threads, 1024, 6144, "DS1", "lru", early_stop=False)
    #
    # for threads in [128, 32, 16, 4, 64, 8, 1, 2]:
    #     bench(threads, 1024, 7168, "DS1", "rgc20", early_stop=False)
    #     bench(threads, 1024, 7168, "DS1", "lru", early_stop=False)

    # bench(256, 32 * 1024, 32 * 64, "P1", "rgc")
    #
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 32 * 1024, 12, trace_file, "lru")
    #         bench(threads, 32 * 1024, 12, trace_file, "rgc")
    #
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 32 * 1024, 24, trace_file, "lru")
    #         bench(threads, 32 * 1024, 24, trace_file, "rgc")
    #
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 32 * 1024, 48, trace_file, "lru")
    #         bench(threads, 32 * 1024, 48, trace_file, "rgc")
    #
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 32 * 1024, 72, trace_file, "lru")
    #         bench(threads, 32 * 1024, 72, trace_file, "rgc")
    #
    # for threads in THREADS_LIST:
    #     for trace_file in ["OLTP"]:
    #         bench(threads, 32 * 1024, 96, trace_file, "lru")
    #         bench(threads, 32 * 1024, 96, trace_file, "rgc")
    #
    # for threads in [128, 32, 16, 4, 64, 8, 1, 2]:
    #     bench(threads, 1024, 6144, "DS1", "lru", False)
    #     bench(threads, 1024, 6144, "DS1", "rgc-DS1", False)
