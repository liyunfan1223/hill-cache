import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), '../..'))
import matplotlib.pyplot as plt
from utils import MultiTestRunner, StatisticsCompareLRU
from scripts.config import config

trace_list = config['trace_list']
buffer_size_list = config['buffer_size_list']
figure_prefix = config['figure_prefix']
figure_path = config['figure_path']
policies = config['policies']
stats = StatisticsCompareLRU()

if __name__ == '__main__':
    print(f'Start running prototype tests. \nTrace List: {trace_list}.\nBuffer Size List : {buffer_size_list}.')
    print('====================================================================')
    for trace in trace_list:
        trace_file = f'traces/{trace}.lis'
        fig, ax = plt.subplots(figsize=(8, 4))
        ax.set_xlim(buffer_size_list[0] // 2, buffer_size_list[-1] * 2)
        ax.set_xticks(buffer_size_list)
        ax.set_xticklabels(buffer_size_list)
        ax.set_xlabel('Buffer Size')
        ax.set_ylabel('Hit Ratio(%)')
        ax.set_title(trace_file)

        lru_runner = MultiTestRunner(['LRU'], buffer_size_list, trace_file, None)
        lru_result = lru_runner.get_hit_rate_list()
        ax.plot(buffer_size_list, lru_result, label='LRU', marker='+', linestyle=':')

        results = []
        for policy, parameters in policies:
            runner = MultiTestRunner([policy], buffer_size_list, trace_file, parameters)
            result = runner.get_hit_rate_list()
            results.append(result)
            ax.plot(buffer_size_list, result, label=policy, marker='+', linestyle='dashed' if policy != 'HILL' else '-')
            stats.statistic(lru_result, result, policy)

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
