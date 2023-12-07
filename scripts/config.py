#### configurations for prototype ####
config = {
    'trace_list': [
        # Example #
        'example'
        # # CloudVPS #
        # 'vps26107', 'vps26391', 'vps26136', 'vps26148', 'vps26215', 'vps26255', 'vps26330', 'vps26511',
        # # FIU #
        # 'webmail', 'websearch', 'webusers', 'online', 'Home1', 'Home2', 'Home3', 'Home4',
        # # MSR #
        # 'msr_usr_0', 'msr_proj_0', 'msr_prn_0', 'msr_hm_0', 'msr_rsrch_0', 'msr_prxy_0',
        # 'msr_src2_0', 'msr_stg_0', 'msr_ts_0', 'msr_web_0', 'msr_mds_0', 'msr_wdev_0',
    ],
    # buffer size list #
    'buffer_size_list': [0.01, 0.05, 0.1],
    # proto executable file path #
    'execution_path': './build/bin/proto',
    # cache used to store results #
    'cache_file_path': 'local/single_test_runner_cache.json',
    # policies configuration (besides LRU for comparison) #
    'policies': [
        # policy, parameters #
        ['LRFU', [1e-5]],
        ['ARC', None],
        ['LIRS', [2]],
        ['DLIRS', [2]],
        ['CACHEUS', None],
        ['HILL', [16, 1, 6, 4, 1.0, 0.67, 0.05, 0.01, 1, 64, 10000]],
        ['OPT', None]
    ],
    # prefix for figures #
    'figure_prefix': 'runner',
    # save figures to the path#
    'figure_path': './local',
}

#### configurations for memtier_benchmark ####
config_memtier = {

}
    
#### configurations for bench ####
config_bench = {
    'client_thread': 128,
    'server_thread': 8,
    'size_list': [24, 48, 72],
    'max_data_length': 1024,
    'trace_file': './traces/example.lis',
    'early_stop_access': 0,
    'sync_threads': True,
    'warm_up_access': 0,
    'manual_remote_latency': 10,
    'rocksdb_path': './local/rocksdb',
    'memcached_executable': './local/memcached',
    'memhc_executable': './local/memhc'
}
    