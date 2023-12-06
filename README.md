# Hill-Cache
We provide a prototype and a benchmark for MemHC.
## Prototype

### Build
```
mkdir build
cd build
cmake ..
make proto
```

## Usage
```
./bin/proto -h
Usage: ./proto [options]
Options:
  -h, --help             Show this help message
  -c, --cache-policy     Cache policy (LRU, LFU, ARC, SRRIP, DRRIP, LRFU, LIRS, DLIRS, CACHEUS, HILL, OPT)
  -b, --buffer-size      Buffer size
  -t, --trace-file       Trace file path
  -s, --stats-interval   Statistics reporting interval
```

## Example
```
./bin/proto -c HILL -b 0.1 -t ../traces/online.lis -s 50000
Hill-Cache: buffer_size:19681 hit_count:2602593 miss_count:3097905 hit_rate:45.6555%
```


## Scripts

**Installation**
```
pip install -r requirement
```
**Runner**
```
python ./scripts/prototype/runner.py
```

Check scripts/config.py to configure parameters for prototype running.
