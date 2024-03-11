import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '../..'))

RAW_TRACE_FOLDER = './local/raw_trace/'
OUTPUT_FOLDER = './traces/'
TRACE_FILE_LIST = [f"casa-110108-112108.{i}.blkparse" for i in range(1, 10)] # Home1
TRACE_FILE_LIST = [f"ikki-110108-112108.{i}.blkparse" for i in range(1, 21)] # Home2
TRACE_FILE_LIST = [f"webresearch-030409-033109.{i}.blkparse" for i in range(1, 29)] # websearch
TRACE_FILE_LIST = [f"webusers-030409-033109.{i}.blkparse" for i in range(1, 29)] # webusers
TRACE_FILE_LIST = [f"topgun-110108-112108.{i}.blkparse" for i in range(1, 21)] # Home4
WRITE_TRACE_PATH = './traces/websearch.lis'
WRITE_TRACE_PATH = './traces/Home4.lis'


def process_blkparse(trace_file_list, write_trace_path):
    access = []
    for raw_trace_file in trace_file_list:
        raw_trace_path = RAW_TRACE_FOLDER + raw_trace_file
        if not os.path.exists(raw_trace_path):
            print(f"{raw_trace_path} not found, skip.")
            continue
        f = open(raw_trace_path, 'r')
        for lines in f.readlines():
            ts, pid, user, offset, size, _1, _2, _3, _4 = lines.split(' ')
            # print(offset, size)
            access.append((int(offset) // 8, int(size) // 8))
        f.close()
    fw = open(write_trace_path, 'w')
    print(len(access))
    last = -1
    last_len = 0
    idx = 0
    for order, size in access:
        if order != last + last_len and last_len != 0:
            if last != -1:
                fw.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
            idx += 1
            last_len = 0
            last = order
        # last = order
        last_len += size
    if last_len != 0:
        fw.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
        idx += 1
    print(idx)
    fw.close()

def process_ssd_trace(input=None, output=None):
    f = open(input)
    fw = open(output, 'w')
    line = f.readline()
    idx = 0
    while line:
        ele = line.split(' ')
        while '' in ele:
            ele.remove('')
        if len(ele) != 11:
            line = f.readline()
            continue
        if ele[5] != 'D':
            line = f.readline()
            continue
        if ele[10].find('rocksdb') == -1:
            line = f.readline()
            continue
        offset = int(ele[7]) // 8
        size = int(ele[9]) // 8
        # print(int(ele[7]), int(ele[9]))
        fw.write(f'{offset} {size} 0 {idx}\n')
        # print(offset, size)
        idx += 1
        line = f.readline()
    fw.close()
    f.close()
    print(idx)

def process_msr(input=None, output=None, block=512):
    raw = open(input)
    processed = open(output, 'w')
    access = []
    for idx, line in enumerate(raw.readlines()):
        ts, host, disk_number, type, offset, size, response_time = line.split(',')
        access.append((int(offset) // block, int(size) // block))
        # processed.write(f'{int(offset) // block} {int(size) // block} 0 {idx}\n')
    raw.close()
    print(len(access))
    last = -1
    last_len = 0
    idx = 0
    for order, size in access:
        if order != last + last_len:
            if last != -1:
                processed.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
            idx += 1
            last_len = 0
            last = order
        # last = order
        last_len += size
    if last_len != 0:
        processed.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
        idx += 1
    print(idx)
    processed.close()

def process_fiu(input=None, output=None, block=512):
    raw = open(input)
    processed = open(output, 'w')
    access = []
    for idx, line in enumerate(raw.readlines()):
        ts, host, disk_number, type, offset, size, response_time = line.split(',')
        access.append((int(offset) // block, int(size) // block))
        # processed.write(f'{int(offset) // block} {int(size) // block} 0 {idx}\n')
    raw.close()
    print(len(access))
    last = -1
    last_len = 0
    idx = 0
    for order, size in access:
        if order != last + last_len:
            if last != -1:
                processed.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
            idx += 1
            last_len = 0
            last = order
        # last = order
        last_len += size
    if last_len != 0:
        processed.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
        idx += 1
    print(idx)
    processed.close()

def process_cloudvps(trace_file_list, write_trace_path, block=8):
    access = []
    for raw_trace_file in trace_file_list:
        raw_trace_path = RAW_TRACE_FOLDER + raw_trace_file
        if not os.path.exists(raw_trace_path):
            print(f"{raw_trace_path} not found, skip.")
            continue
        f = open(raw_trace_path, 'r')
        for lines in f.readlines():
            if lines.startswith('CPU'):
                break
            offset, size = lines.split(' ')
            # print(offset, size)
            access.append((int(offset) // block, int(size) // block))
        f.close()
    fw = open(write_trace_path, 'w')
    print(len(access))
    last = -1
    last_len = 0
    idx = 0
    for order, size in access:
        if order != last + last_len and last_len != 0:
            if last != -1:
                fw.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
            idx += 1
            last_len = 0
            last = order
        # last = order
        last_len += size
    if last_len != 0:
        fw.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
        idx += 1
    print(idx)
    fw.close()

def process_spc(trace_file_list, write_trace_path, block=512, max_lba=1000000):
    access = []
    for raw_trace_file in trace_file_list:
        raw_trace_path = RAW_TRACE_FOLDER + raw_trace_file
        if not os.path.exists(raw_trace_path):
            print(f"{raw_trace_path} not found, skip.")
            continue
        f = open(raw_trace_path, 'r')
        for lines in f.readlines():
            if lines.startswith('CPU'):
                break
            # offset, size = lines.split(' ')
            asu, lba, size, op, ts = lines.split(',')
            access.append((int(asu) * max_lba + int(lba), int(size) // block))
        f.close()
    fw = open(write_trace_path, 'w')
    print(len(access))
    last = -1
    last_len = 0
    idx = 0
    for order, size in access:
        if order != last + last_len and last_len != 0:
            if last != -1:
                fw.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
            idx += 1
            last_len = 0
            last = order
        # last = order
        last_len += size
    if last_len != 0:
        fw.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
        idx += 1
    print(idx)
    fw.close()

def process_tpc(trace_file_list, write_trace_path, block=8192, record_limit=-1):
    access = []
    for raw_trace_file in trace_file_list:
        raw_trace_path = raw_trace_file
        if not os.path.exists(raw_trace_path):
            print(f"{raw_trace_path} not found, skip.")
            continue
        print("Opening", raw_trace_file)
        f = open(raw_trace_path, 'r')
        print("Success open")
        start = False
        record_sz = 0
        lines = f.readline()
        while lines:
        # for lines in f.readlines():
            if lines.startswith('EndHeader'):
                start = True
            if not start:
                lines = f.readline()
                continue
            lines = lines.replace(' ','')
            splited = lines.split(',')
            if splited[0] != 'DiskRead' and splited[0] != 'DiskWrite':
                lines = f.readline()
                continue
            offset = int(splited[5], 16)
            size = int(splited[6], 16)
            # print(offset, size)
            # print(splited)
            access.append((int(offset) // block, int(size) // block))
            record_sz += 1
            # print(record_sz)
            if record_limit != -1 and record_sz >= record_limit:
                break
            # offset, size = lines.split(' ')
            # asu, lba, size, op, ts = lines.split(',')
            # access.append((int(asu) * max_lba + int(lba), int(size) // block))
            lines = f.readline()
        f.close()
    print('Record size', record_sz)
    fw = open(write_trace_path, 'w')
    print(len(access))
    last = -1
    last_len = 0
    idx = 0
    for order, size in access:
        if order != last + last_len and last_len != 0:
            if last != -1:
                fw.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
            idx += 1
            last_len = 0
            last = order
        # last = order
        last_len += size
    if last_len != 0:
        fw.write(f"{str(last)} {str(last_len)} 0 {idx}\n")
        idx += 1
    print(idx)
    fw.close()

    
if __name__ == "__main__":
    # SQLServer
    process_tpc(["/mnt/e/W2K8.TPCC.02-26-2008.09-37-AM.trace.csv/W2K8.TPCC.02-26-2008.09-37-AM.trace.csv"], "./traces/tpcc2_9_37.lis")
    process_tpc(["/mnt/e/W2K8.TPCC.02-26-2008.09-43-AM.trace.csv/W2K8.TPCC.02-26-2008.09-43-AM.trace.csv"], "./traces/tpcc2_9_43.lis")
    process_tpc(["/mnt/e/W2K8.TPCC.02-26-2008.09-49-AM.trace.csv/W2K8.TPCC.02-26-2008.09-49-AM.trace.csv"], "./traces/tpcc2_9_49.lis")
    process_tpc(["/mnt/e/W2K8.TPCC.02-26-2008.09-56-AM.trace.csv/W2K8.TPCC.02-26-2008.09-56-AM.trace.csv"], "./traces/tpcc2_9_56.lis")
    process_tpc(["/mnt/e/W2K8.TPCC.02-26-2008.10-02-AM.trace.csv/W2K8.TPCC.02-26-2008.10-02-AM.trace.csv"], "./traces/tpcc2_10_02.lis")
    process_tpc(["/mnt/e/W2K8.TPCC.02-26-2008.10-09-AM.trace.csv/W2K8.TPCC.02-26-2008.10-09-AM.trace.csv"], "./traces/tpcc2_10_09.lis")
    process_tpc(["/mnt/e/W2K8.TPCC.10-19-2007.12-51-PM.trace.csv/W2K8.TPCC.10-19-2007.12-51-PM.trace.csv"], "./traces/tpcc1_12_51.lis")
    process_tpc(["/mnt/e/W2K8.TPCC.10-19-2007.01-44-PM.trace.csv/W2K8.TPCC.10-19-2007.01-44-PM.trace.csv"], "./traces/tpcc1_01_44.lis")
    # MSR
    process_msr(RAW_TRACE_FOLDER + 'CAMRESHMSA01-lvm0.csv', OUTPUT_FOLDER + 'msr_hm_0.lis')
    process_msr(RAW_TRACE_FOLDER + 'CAMRESISAA02-lvm0.csv', OUTPUT_FOLDER + 'msr_prxy_0.lis')
    process_msr(RAW_TRACE_FOLDER + 'CAM-02-SRV-lvm0.csv', OUTPUT_FOLDER + 'msr_proj_0.lis')
    process_msr(RAW_TRACE_FOLDER + 'CAMRESWEBA03-lvm0.csv', OUTPUT_FOLDER + 'msr_web_0.lis')
    process_msr(RAW_TRACE_FOLDER + 'CAMRESWMSA03-lvm0.csv', OUTPUT_FOLDER + 'msr_mds_0.lis')
    process_msr(RAW_TRACE_FOLDER + 'CAMRESSTGA01-lvm0.csv', OUTPUT_FOLDER + 'msr_stg_0.lis')
    process_msr(RAW_TRACE_FOLDER + 'CAM-02-SRV-lvm1.csv', OUTPUT_FOLDER + 'msr_proj_1.lis')
    process_msr(RAW_TRACE_FOLDER + 'CAM-01-SRV-lvm1.csv', OUTPUT_FOLDER + 'msr_usr_1.lis', 4096)
    process_msr(RAW_TRACE_FOLDER + 'CAMRESIRA01-lvm0.csv', OUTPUT_FOLDER + 'msr_rsrch_0.lis', 4096)
    process_msr(RAW_TRACE_FOLDER + 'CAM-01-SRV-lvm0.csv', OUTPUT_FOLDER + 'msr_usr_0.lis', 4096)
    process_msr(RAW_TRACE_FOLDER + 'CAMRESTSA01-lvm0.csv', OUTPUT_FOLDER + 'msr_ts_0.lis', 4096)
    process_msr(RAW_TRACE_FOLDER + 'CAM-USP-01-lvm0.csv', OUTPUT_FOLDER + 'msr_prn_0.lis', 4096)
    process_msr(RAW_TRACE_FOLDER + 'CAMRESSDPA03-lvm0.csv', OUTPUT_FOLDER + 'msr_src2_0.lis', 4096)
    process_msr(RAW_TRACE_FOLDER + 'CAMWEBDEV-lvm0.csv', OUTPUT_FOLDER + 'msr_wdev_0.lis', 4096)
    # FIU
    process_blkparse([f"webmail.cs.fiu.edu-110108-113008.{i}.blkparse" for i in range(1, 20 + 1)], './traces/webmail.lis')
    process_blkparse([f"casa-110108-112108.{i}.blkparse" for i in range(1, 21 + 1)], './traces/Home1.lis')
    process_blkparse([f"madmax-110108-112108.{i}.blkparse" for i in range(1, 20 + 1)], './traces/Home3.lis')
    process_blkparse([f"topgun-110108-112108.{i}.blkparse" for i in range(1, 20 + 1)], './traces/Home4.lis')
    process_blkparse([f"online.cs.fiu.edu-110108-113008.{i}.blkparse" for i in range(1, 20 + 1)], './traces/online.lis')
    process_blkparse([f"cheetah.cs.fiu.edu-110108-113008.{7}.blkparse" for i in range(1, 21 + 1)], './traces/mail.lis')
    # CloudVPS
    process_cloudvps(["vps26391.blkparse"], './traces/cloudvps26391.lis')
    process_cloudvps(["vps26107.blkparse"], './traces/cloudvps26107.lis')
    process_cloudvps(["vps26136.blkparse"], './traces/cloudvps26136.lis')
    process_cloudvps(["vps26148.blkparse"], './traces/cloudvps26148.lis')
    process_cloudvps(["vps26215.blkparse"], './traces/cloudvps26215.lis')
    process_cloudvps(["vps26255.blkparse"], './traces/cloudvps26255.lis')
    process_cloudvps(["vps26330.blkparse"], './traces/cloudvps26330.lis')
    process_cloudvps(["vps26511.blkparse"], './traces/cloudvps26511.lis')
