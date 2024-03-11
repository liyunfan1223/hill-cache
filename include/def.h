//
// Created by Yunfan Li on 2022/9/28.
//

#pragma once

#include <sys/time.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// defined for LIRS
#define NEED_PRUNING(n) ((n)->type != LIR)
#define NONEVALUE ((long long)-11)
#define INVALID (NONEVALUE)
#define IS_VALID(value) ((value) != NONEVALUE && (value) != INVALID)
enum lirs_type {
    LIR = 101,
    HIR,
    NHIR,
};
// end

struct trace_line {
    int starting_block;
    int number_of_blocks;
    int ignore;
    int request_number;
};

enum class RC {
    DEFAULT,
    SUCCESS,
    HIT,
    MISS,
    FAILED,
    UNIMPLEMENT,
};

enum class CachePolicy {
    LRU,
    LFU,
    ARC,
    OPT,
    SRRIP,
    DRRIP,
    EFSW,
    LRFU,
    LIRS,
    DLIRS,
    CACHEUS,
    S3FIFO,
    HILL,
    UNKNOWN,
};

typedef int32_t Key;
typedef std::string Value;

typedef int32_t PageId;
typedef int32_t FrameId;

const int32_t BASIC_MAIN_ARG_NUM = 0;
const double EPSILON = 1e-10;
