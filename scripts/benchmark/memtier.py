import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), '../..'))
import subprocess
import time
from scripts.config import config_memtier

def bench_memtier( memcached_mem, memc_executable, threads, data_size, server_thread, est_item_counts=0 ):
    args = f"mem-{memcached_mem} memc_executable-{memc_executable} threads-{threads} server_thread-{server_thread}"
    print(f"args: {args}")
    params = [f"{memc_executable}", "-m", f"{memcached_mem}", "-o", "no_lru_crawler",
              "-o", "no_lru_maintainer", "-u", "ubuntu22", "-t", f"{server_thread}"]
    if est_item_counts != 0:
        params.append("-E")
        params.append(f"{est_item_counts}")
    memc_process = subprocess.Popen(params)
    time.sleep(3)
    tier = subprocess.Popen(["memtier_benchmark", "-t", f"{threads}", "-c", "1",
                             f"--requests={config_memtier['requests_per_client_thread']}", "--ratio=5:5",
                             f"--data-size={data_size}", "--protocol=memcache_binary",
                             "--key-pattern=R:R", "--key-maximum", "65536", "-s", "127.0.0.1",
                             "-p", "11211", "--distinct-client-seed"], stdout=subprocess.PIPE)
    output, errors = tier.communicate()
    output = output.decode('utf-8')

    print(output.split('\n')[7:13])
    outputs = output.split('\n')[7:13]
    with open(f"{config_memtier['log_file']}", "a") as f:
        f.write(f"{args}\n")
        for o in outputs:
            f.write(f"{o}\n")
        f.write("\n")
    memc_process.terminate()
    memc_process.wait()
    print("Finished.")


if __name__ == '__main__':
    for thread in config_memtier['client_thread_list']:
        for server_thread in config_memtier['server_thread_list']:
            for mem_size in config_memtier['mem_size_list']:
                for data_size in config_memtier['data_size_list']:
                    bench_memtier(mem_size, config_memtier['memcached_executable'], thread, data_size, server_thread)
                    bench_memtier(mem_size, config_memtier['memhc_executable'], thread, data_size, server_thread)
