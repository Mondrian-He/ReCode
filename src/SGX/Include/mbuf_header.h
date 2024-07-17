#ifndef MHEADER_H
#define MHEADER_H

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <time.h>
#include <unordered_map>
#include <map>
#include <vector>
#include <iostream>

using std::string;
using std::to_string;
using std::unordered_map;
using std::map;
using namespace std;

// #define NUM_DB 10
#define HASH_NUM 3
#define REP_TIME 3
#define PART_NUM 62
#define TOTAL_CAP 60000

const int MBUFFER_SIZE = 4 * 1024;
const int MBUFFER_NUM_MAX_SIZE = 1024;
const int MBUFFER_DATA_NUM_MAX_SIZE = 6 * 1024;

struct k_r{
    int k;
    int rid;
    int key()const{
        return k;
    }
    k_r(){}
    k_r(int k,int rid){this->k=k;this->rid=rid;}
};
struct MBuf_id{
    int page_id;
    int offset;
    MBuf_id(){}
    MBuf_id(int page_id,int offset){this->page_id=page_id;this->offset=offset;}
};

struct MBuf_des {
    int page_id;
    int buffer_id;
    int count;
    bool dirty;
    int next_buffer_id, prev_buffer_id;

    MBuf_des() { page_id = buffer_id = next_buffer_id = prev_buffer_id = 1, count = 0, dirty = false; }
    void update(int pid, int bid, int cnt, bool dirty) {
        page_id = pid, buffer_id = bid, count = cnt, this->dirty = dirty;
    }
};

struct HeapData{
    int pageid;
    int offset;
    int rid;
    char field[1024] = {};

    void set(int p, int o, int r, char* f) {
        pageid = p, offset = o, rid = r;
        memcpy(field, f, 1024);
    }
};

#endif // MHEADER_H
