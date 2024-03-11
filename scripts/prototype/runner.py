import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), '../..'))
import matplotlib.pyplot as plt
from utils import MultiTestRunner, StatisticsCompareLRU
from scripts.config import config

trace_list = config['trace_list']
kbuffer_size_list = config['buffer_size_list']
kbuffer_size_list_for_tpcc = config['buffer_size_list_for_tpcc']
figure_prefix = config['figure_prefix']
figure_path = config['figure_path']
policies = config['policies']
stats = StatisticsCompareLRU()

if __name__ == '__main__':
    print(f'Start running prototype tests. \nTrace List: {trace_list}.\nBuffer Size List : {kbuffer_size_list}.')
    print('====================================================================')
    for trace in trace_list:
        buffer_size_list = kbuffer_size_list_for_tpcc if trace.startswith('tpcc') else kbuffer_size_list
        trace_file = f'traces/{trace}.lis'
        fig, ax = plt.subplots(figsize=(8, 4))
        ax.set_xlim(buffer_size_list[0] // 2, buffer_size_list[-1] * 2)
        ax.set_xticks(buffer_size_list)
        ax.set_xticklabels(buffer_size_list)
        ax.set_xlabel('Buffer Size')
        ax.set_ylabel('Hit Ratio(%)')
        ax.set_title(trace_file)

        lru_runner = MultiTestRunner(['LRU'], buffer_size_list, trace_file, [None])
        lru_result = lru_runner.get_hit_rate_list()
        ax.plot(buffer_size_list, lru_result, label='LRU', marker='+', linestyle=':')

        results = []
        for policy, parameters in policies:
            runner = MultiTestRunner([policy], buffer_size_list, trace_file, [parameters])
            result = runner.get_hit_rate_list()
            results.append(result)
            ax.plot(buffer_size_list, result, label=policy, marker='+', linestyle='dashed' if policy != 'HILL' else '-')
            stats.statistic(lru_result, result, policy + '-' + str(parameters))
        

        ###########
        # lrfu_runner = MultiTestRunner(['LRFU'], buffer_size_list, trace_file, [[1e-5]])
        # lrfu_result = lrfu_runner.get_hit_rate_list()

        # arc_runner = MultiTestRunner(['ARC'], buffer_size_list, trace_file, [None])
        # arc_result = arc_runner.get_hit_rate_list()

        # params_list = [2]
        # lirs2_runner = MultiTestRunner(['LIRS'], buffer_size_list, trace_file, [params_list])
        # lirs2_result = lirs2_runner.get_hit_rate_list()

        # params_list = [2]
        # dlirs2_runner = MultiTestRunner(['DLIRS'], buffer_size_list, trace_file, [params_list])
        # dlirs2_result = dlirs2_runner.get_hit_rate_list()

        # params_list = None
        # cacheus_runner = MultiTestRunner(['CACHEUS'], buffer_size_list, trace_file, [params_list])
        # cacheus_result = cacheus_runner.get_hit_rate_list()

        # params_list = None
        # s3fifo_runner = MultiTestRunner(['S3FIFO'], buffer_size_list, trace_file, [params_list])
        # s3fifo_result = s3fifo_runner.get_hit_rate_list()

        # params_list = [16, 1, 6, 4, 1.0, 0.67, 0.05, 0.01, 1, 64, 10000]
        # hill_runner = MultiTestRunner(['HILL'], buffer_size_list, trace_file, [params_list])
        # hill_result = hill_runner.get_hit_rate_list()

        # stats.statistic(arc_result, hill_result, "New-RGC4(3b)-arc")
        # stats.statistic(lirs2_result, hill_result, "New-RGC4(3b)-lirs")
        # stats.statistic(dlirs2_result, hill_result, "New-RGC4(3b)-dlirs")
        # stats.statistic(cacheus_result, hill_result, "New-RGC4(3b)-cacheus")
        # stats.statistic(lrfu_result, hill_result, "New-RGC4(3b)-lrfu")
        # stats.statistic(s3fifo_result, hill_result, "New-RGC4(3b)-s3fifo")
        ################
        
        ax.set_xscale('log')
        ax.set_xticks(buffer_size_list)
        ax.set_xticklabels(buffer_size_list)
        ax.set_xlim(buffer_size_list[0] / 2, buffer_size_list[-1] * 2)
        plt.legend(loc=0)
        if not os.path.exists('local'):
            os.mkdir('local')
        fig_path = f'{figure_path}/{figure_prefix}_{trace}.png'
        plt.savefig(fig_path)
        print(f'Fig generated path: {fig_path}. ')
        
        
        stats.print_result()
        print('====================================================================')
