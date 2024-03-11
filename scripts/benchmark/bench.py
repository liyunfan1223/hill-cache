import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), '../..'))
import subprocess
import time
from scripts.config import config_bench

def bench(thread_num, value_size, memcached_mem, trace_file, executable, server_thread,
          early_stop=config_bench['early_stop_access'], threads_sync=config_bench['sync_threads'],
          warmup=config_bench['warm_up_access'], est_item_counts=0):
    print(f"Started. Args: threads:{thread_num} value_size:{value_size} mem:{memcached_mem} server_thread:{server_thread} "
          f"trace:{trace_file} executable:{executable} earle_stop:{early_stop} threads_sync:{threads_sync} has_warmup:{warmup}")
    params = [f"{executable}", "-m", f"{memcached_mem}", "-o", "no_lru_crawler",
              "-o", "no_lru_maintainer", "-t", f"{server_thread}"]
    if est_item_counts != 0:
        params.append("-E")
        params.append(f"{est_item_counts}")
    # print(params)
    memc_process = subprocess.Popen(params)
    # print(1)
    time.sleep(3)
    # print(5)
    args = ["build/bin/bench", "-t", f"{thread_num}", "-l", f"{value_size}", "-f", f"{trace_file}",
            "-e", f"{early_stop}", "-s", f"{1 if threads_sync else 0}", "-w", f"{warmup}",
            "-d", config_bench['rocksdb_path'], "-m", f"{config_bench['manual_remote_latency']}",
            "-r", f"{config_bench['report_interval']}"]
    # print(args)
    bench_process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, errors = bench_process.communicate()
    if len(errors) > 0:
        print("Error occurred!", errors)
    else:
        try:
            result = output.decode('utf-8')
            result = result.split("\n")[-110:]
            result_str = ''
            for item in result:
                result_str += item + '\n'
            log = f"Time: {time.time()} Args: threads-{thread_num} value_size-{value_size} memcached_mem-{memcached_mem} server_thread-{server_thread} "\
                  f"trace-{trace_file} executable-{executable} earle_stop-{early_stop} threads_sync-{threads_sync} has_warmup-{warmup} \n"
            print("result: ", log)
            with open(config_bench['log_file'], "a") as f:
                f.write(f"{log}\n{result_str}")
        except Exception as e:
            with open(config_bench['log_file'], "a") as f:
                f.write(f"Error occurred: {e}\n")
            print(f"Error occurred: {e}")
    memc_process.terminate()
    memc_process.wait()
    print("Finished.")


if __name__ == "__main__":

    for size, estimated_item in zip(config_bench['size_list'], config_bench['estimated_items_count']):
        for threads in config_bench['client_thread_list']:
            for server_thread in config_bench['server_thread_list']:
                for trace_file in config_bench['trace_file_list']:
                    bench(threads, config_bench['max_data_length'], size, trace_file,
                          config_bench['memcached_executable'], server_thread,
                          early_stop=config_bench['early_stop_access'], warmup=config_bench['warm_up_access'])
                    bench(threads, config_bench['max_data_length'], size, trace_file,
                          config_bench['memhc_executable'], server_thread,
                          early_stop=config_bench['early_stop_access'], warmup=config_bench['warm_up_access'],
                          est_item_counts=estimated_item)
