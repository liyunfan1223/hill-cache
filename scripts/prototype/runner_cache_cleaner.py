import json
import os
import sys

root_path = os.path.join(os.path.dirname(__file__), '../..')
sys.path.append(root_path)

cache_file_path = os.path.abspath(os.path.join(root_path, 'local/single_test_runner_cache.json'))


def invalid_cache(cache_policy=None, trace_file=None):
    if os.path.exists(cache_file_path):
        with open(cache_file_path, 'r') as f:
            data: dict = json.load(f)
        remove_key_list = []
        for key in data.keys():
            cache_policy_check = True if cache_policy == '.' else False
            trace_file_check = True if trace_file == '.' else False
            if key.find(cache_policy) != -1:
                cache_policy_check = True
            if key.find(trace_file) != -1:
                trace_file_check = True
            if cache_policy_check and trace_file_check:
                remove_key_list.append(key)
        for key in remove_key_list:
            del data[key]
        with open(cache_file_path, 'w') as f:
            json.dump(data, f)
        print(f'Cache {cache_policy} {trace_file} invalid.')
    else:
        print('Cache file doesn\'t exists.')


if __name__ == "__main__":
    argv = sys.argv
    if argv[1] == '--help':
        print('python reload_cache.py <CACHE_POLICY_TO_INVALID> <TRACE_FILE_TO_INVALID> (use "." to present any) ')
        exit(0)
    invalid_cache(argv[1], argv[2])
