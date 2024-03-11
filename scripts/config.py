### configurations for prototype ###
config = {
    'trace_list': [
        # Example #
        'example',
        # TPC-C #
        # 'tpcc1_12_51', 'tpcc1_01_44', 'tpcc2_9_49', 'tpcc2_9_37', 
        # 'tpcc2_9_43', 'tpcc2_9_56', 'tpcc2_10_02', 'tpcc2_10_09',
        # CloudVPS #
        # 'vps26107', 'vps26391', 'vps26136', 'vps26148', 
        # 'vps26215', 'vps26255', 'vps26330', 'vps26511',
        # FIU #
        # 'webmail', 'websearch', 'webusers', 'online', 
        # 'Home1', 'Home2', 'Home3', 'Home4',
        # MSR #
        # 'msr_usr_0', 'msr_proj_0', 'msr_prn_0', 'msr_hm_0', 'msr_rsrch_0', 'msr_prxy_0',
        # 'msr_src2_0', 'msr_stg_0', 'msr_ts_0', 'msr_web_0', 'msr_mds_0', 'msr_wdev_0',
    ],
    # buffer size list #
    'buffer_size_list': [0.01, 0.05, 0.1],
    'buffer_size_list_for_tpcc': [0.1, 0.2, 0.4],
    # proto executable file path #
    'execution_path': './build/bin/proto',
    # cache used to store results #
    'cache_file_path': 'local/single_test_runner_cache.db',
    # policies configuration (except LRU for comparison) #
    'policies': [
        # policy, parameters #
        ['LRFU', [1e-5]],
        ['ARC', None],
        ['LIRS', [2]],
        ['DLIRS', [2]],
        ['CACHEUS', None],
        ['S3FIFO', None],
        ['HILL', [16, 1, 6, 4, 1.0, 0.67, 0.05, 0.01, 1, 64, 10000]],
        ['HILL', [16, 1, 2, 4, 1.0, 0.67, 0.05, 0.01, 1, 64, 10000]], # HC-L
        ['HILL', [16, 1, 6, 4, 1.0, 0.67, 0.05, 0.01, 1, 0, 10000]], # HC-M
        ['HILL', [16, 1, 6, 0, 1.0, 0.67, 0.05, 0.01, 1, 64, 10000]], # HC-G
        ['OPT', None]
    ],
    # prefix for figures #
    'figure_prefix': 'runner',
    # save figures to the path#
    'figure_path': './local',
    'runner_threads': 6,
}

### configurations for memtier_benchmark ###
config_memtier = {
    'client_thread_list': [16],
    'server_thread_list': [8],
    'requests_per_client_thread': 300000,
    'data_size_list': [1024],   # B
    'mem_size_list': [64],      # MB
    'memcached_executable': './local/memcached',
    'memhc_executable': './local/memhc',
    'log_file': "./local/memtier.log",
}
    
### configurations for bench ###
config_bench = {
    'client_thread_list': [128],
    'server_thread_list': [8],
    'size_list': [24],
    'estimated_items_count': [32768, 65536, 98304],
    'max_data_length': 1024,
    'trace_file_list': ['./traces/example.lis'],
    'early_stop_access': 1000000,
    'sync_threads': True,
    'warm_up_access': 200000,
    'manual_remote_latency': 10,
    'bench_executable': './build/bin/bench',
    'rocksdb_path': './local/rocksdb',
    'memcached_executable': './local/memcached',
    'memhc_executable': './local/memhc',
    'log_file': "./local/bench.log",
    'report_interval': 100000,
}
    