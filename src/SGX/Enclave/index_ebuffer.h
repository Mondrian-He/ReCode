#ifndef EBUFFER_H
#define EBUFFER_H

#include "ebuf_header.h"
#include "index_mbuffer.h"

using std::string;
using std::to_string;
using std::unordered_map;
using std::map;
using namespace std;

struct EBuffer {
public:
    unordered_map<int,int> node2buffer;
    int buffer2node[EBUFFER_NUM_MAX_SIZE];
    EBuf_des ebuf_des[EBUFFER_NUM_MAX_SIZE];
    EBuf_pool *ebuf_pool;
    int head_bid, tail_bid, size;

    MBuffer* mbuffer;

    int tot_page_update_num;
    int hit_num;

    int SelectVictim() {
        return tail_bid;
    }
    void SetDirty(int buffer_id) {
        ebuf_des[buffer_id].dirty = true;
    }
    void WriteBuffer(int buffer_id,MBuf_des* mbdes,char** mbpool) {
        printf("EBuffer.WriteBuffer\n");
        mbuffer->UpdatePage(buffer2node[buffer_id], ebuf_pool[buffer_id],mbdes,mbpool);      
    }
    void ReadPage(int node_id, EBuf_pool &thebuffer,MBuf_des* mbdes,char** mbpool) {
        mbuffer->FixPage(node_id, thebuffer,mbdes,mbpool);
    }
    void LRUUpdate(int readed_bid) {
        if (tail_bid == readed_bid) { 

            tail_bid = ebuf_des[readed_bid].prev_buffer_id;
            ebuf_des[tail_bid].next_buffer_id = tail_bid;
        } else {
            int prev = ebuf_des[readed_bid].prev_buffer_id;
            int next = ebuf_des[readed_bid].next_buffer_id;

            ebuf_des[prev].next_buffer_id = next;
            ebuf_des[next].prev_buffer_id = prev;
        }
        ebuf_des[head_bid].prev_buffer_id = readed_bid;
        ebuf_des[readed_bid].next_buffer_id = head_bid;
        ebuf_des[readed_bid].prev_buffer_id = readed_bid;
        head_bid = readed_bid;
    }
    void LRUInsert(int inserted_bid) {
        if (size == 0) { 
            head_bid = tail_bid = inserted_bid;
            ebuf_des[head_bid].next_buffer_id = ebuf_des[head_bid].prev_buffer_id = inserted_bid;
        } else { 
            ebuf_des[head_bid].prev_buffer_id = inserted_bid;
            ebuf_des[inserted_bid].next_buffer_id = head_bid;
            ebuf_des[inserted_bid].prev_buffer_id = inserted_bid;
            head_bid = inserted_bid;
        }
    }

    void IncHitNum() {
        hit_num += 1;
    }
    void IncTotPageUpdateNum() {
        tot_page_update_num += 1;
    }

public:
    EBuffer() {
        memset(buffer2node, -1, sizeof(buffer2node));
        memset(ebuf_des, -1, sizeof(ebuf_des));
        head_bid = tail_bid = -1;
        size = 0;
        mbuffer = new MBuffer;
        ebuf_pool = new EBuf_pool[EBUFFER_NUM_MAX_SIZE];
        hit_num = tot_page_update_num = 0;
    }
    ~EBuffer() {
        delete[] ebuf_pool;
    }
    void WriteDirtys(MBuf_des* mbdes,char** mbpool) {
        for (int bid = 0; bid < size; bid++) {
            if (ebuf_des[bid].dirty) {
                mbuffer->UpdatePage(buffer2node[bid], ebuf_pool[bid],mbdes,mbpool);
                ebuf_des[bid].dirty = false;
            }
        }
        mbuffer->WriteDirtys(mbdes,mbpool);
    }
};

#endif // BUFFER_MGR_H
