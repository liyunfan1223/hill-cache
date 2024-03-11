import os
import json
import sys
root_path = os.path.join(os.path.dirname(__file__), '../..')
sys.path.append(root_path)
from collections import defaultdict
from scripts.config import config
import threading
import shelve
import concurrent.futures

lock = threading.Lock()

class SingleTestRunner:
    # EXECUTION_PATH = './build/src/main'
    # execution_path = '../etc/'
    START_POSITION = len('hit_rate:')
    # CACHE_FILE_PATH = 'local/single_test_runner_cache.json'

    def __init__(self, cache_policy=None, buffer_size=None, trace_file=None, params=None, execution_path=config['execution_path'], cache_file_path=config['cache_file_path']):
        self.cache_policy = cache_policy
        self.buffer_size = buffer_size
        self.params = params
        self.trace_file = os.path.abspath(os.path.join(root_path, trace_file))
        self.cache_file_path = os.path.abspath(os.path.join(root_path, cache_file_path))
        self.execution_path = os.path.abspath(os.path.join(root_path, execution_path))

    def make_cache_key_string(self):
        return str(self.cache_policy) + '_' + str(self.buffer_size) + '_' + \
            str(self.trace_file) + '_' + str(self.params)

    def get_hit_rate(self, read_cache=True, write_cache=True):
        if read_cache and os.path.exists(self.cache_file_path):
            with shelve.open(self.cache_file_path, 'r') as db:
                if self.make_cache_key_string() in db:
                    print(f'Read {db[self.make_cache_key_string()]:.2f}% from cache. key: {self.make_cache_key_string()}')
                    return db[self.make_cache_key_string()]
            # with open(self.cache_file_path, 'r') as f:
            #     data: dict = json.load(f)
            # if self.make_cache_key_string() in data.keys():
            #     print(f'Read {data[self.make_cache_key_string()]:.2f}% from cache. key: {self.make_cache_key_string()}')
            #     return data[self.make_cache_key_string()]
        cmdline = self.execution_path + f" -c {self.cache_policy} -b {self.buffer_size} -t {self.trace_file}"
        if self.params is not None:
            for param in self.params:
                cmdline += f" {param}"
        print('Running', cmdline)
        res = os.popen(cmdline).read().strip()
        result = float(res.split()[-1][self.START_POSITION:-1])
        if write_cache:
            with lock:
                with shelve.open(self.cache_file_path) as db:
                    db[self.make_cache_key_string()] = result
                print(f'Result {result:.2f}% saved in cache file. key: {self.make_cache_key_string()}')

            # if os.path.exists(self.cache_file_path):
            #     with open(self.cache_file_path, 'r') as f:
            #         data: dict = json.load(f)
            # else:
            #     data = dict()
            # data[self.make_cache_key_string()] = result
            # with open(self.cache_file_path, 'w') as f:
            #     json.dump(data, f)
            # print(f'Result {result:.2f}% saved in cache file. key: {self.make_cache_key_string()}')
        return result

    def get_outputs(self):
        cmdline = self.execution_path + f" {self.cache_policy} {self.buffer_size} {self.trace_file}"
        if self.params is not None:
            for param in self.params:
                cmdline += f" {param}"
        print('cmdline: ', cmdline)
        res = os.popen(cmdline).read().strip()
        return res

        
class MultiTestRunner:

    def __init__(self, cache_policy_list=None, buffer_size_list=None, trace_file=None, params_list=None):
        self.cache_policy_list = cache_policy_list
        self.buffer_size_list = buffer_size_list
        self.trace_file = trace_file
        self.params_list = params_list
        self.hit_rate_list = []

    def get_hit_rate_list(self):
        with concurrent.futures.ThreadPoolExecutor(max_workers=config['runner_threads']) as executor:  # 设置最大线程数为5
            threads = []
            for cache_policy, params in zip(self.cache_policy_list, self.params_list):
                for buffer_size in self.buffer_size_list:
                    thread = executor.submit(self._run_single_test_runner, cache_policy, buffer_size, params)
                    threads.append(thread)

            # 获取结果
            concurrent.futures.wait(threads)

        self.hit_rate_list = sorted(self.hit_rate_list, key=lambda x: (x[0], x[1]))
        self.hit_rate_list = [float(hit_rate[2]) for hit_rate in self.hit_rate_list]

        print('\tcache policy list:', self.cache_policy_list,
              '\n\tbuffer size list:', self.buffer_size_list,
              '\n\ttrace file:', self.trace_file,
              '\n\tparams_list:', self.params_list,
              '\n\thit rate list:', self.hit_rate_list)
        print('====================================================================')
        return self.hit_rate_list

    def _run_single_test_runner(self, cache_policy, buffer_size, params):
        single_test_runner = SingleTestRunner(cache_policy, buffer_size, self.trace_file, params)
        hit_rate = single_test_runner.get_hit_rate()
        self.hit_rate_list.append((cache_policy, buffer_size, hit_rate))

    def get_miss_rate_list(self):
        hit_rate_list = self.get_hit_rate_list()
        miss_rate_list = [(100 - x) for x in hit_rate_list]
        return miss_rate_list
    
class StatisticsCompareLRU:

    def __init__(self):
        self.data = defaultdict()
        self.count = defaultdict()
        self.perf = defaultdict()
        self.lru = None

    def statistic(self, lru_result, compared_result, compared_label='default'):
        self.lru = lru_result
        for lru, compared in zip(lru_result, compared_result):
            if lru == 0:
                ratio = 0
            else:
                ratio = (compared - lru) / lru
            perf = (100 - lru) / (100 - compared) - 1
            if compared_label in self.data:
                self.data[compared_label] += ratio
                self.perf[compared_label] += perf
                self.count[compared_label] += 1
            else:
                self.data[compared_label] = ratio
                self.perf[compared_label] = perf
                self.count[compared_label] = 1

    def print_result(self):
        for label in self.data.keys():
            print(f"Average Miss rate of {label}: {self.perf[label] / self.count[label] * 100:.2f}% lower than lru.")
        # for label in self.data.keys():
        #     print(f"Hit rate of {label}: {self.data[label] / self.count[label] * 100}% average higher than lru.")
