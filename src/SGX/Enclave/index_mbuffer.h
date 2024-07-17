#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H

#include "mbuf_header.h"
#include "ebuf_header.h"
#include "BAddTreeNode.h"
#include "Enclave_t.h"
#include "Ocall_wrappers.h"

#include <algorithm>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unordered_map>
#include <map>
#include <vector>
#include <iostream>
#include <queue>


using namespace BATN;

map<int,MBuf_id> node2page;
char key[AES_BLOCK_SIZE];

struct MBuffer {
public:
    unordered_map<int, int> page2buffer;
    int buffer2page[MBUFFER_NUM_MAX_SIZE];

    int head_bid, tail_bid, size;

    int SelectVictim() {
        return tail_bid;
    }
    void SetDirty(int buffer_id,MBuf_des* mbdes) {
        mbdes[buffer_id].dirty = true;
    }
    void WriteBuffer(int buffer_id,char** mbpool) {
        ocall_writepage(buffer2page[buffer_id], mbpool[buffer_id]);
    }
    void ReadPage(int page_id, char* thebuffer) {
        ocall_readpage(page_id, thebuffer);
    }
    void LRUUpdate(int readed_bid,MBuf_des* mbdes) {
        if (tail_bid == readed_bid) {

            tail_bid = mbdes[readed_bid].prev_buffer_id;
            mbdes[tail_bid].next_buffer_id = tail_bid;
        } else { 
            int prev = mbdes[readed_bid].prev_buffer_id;
            int next = mbdes[readed_bid].next_buffer_id;

            mbdes[prev].next_buffer_id = next;
            mbdes[next].prev_buffer_id = prev;
        }
        mbdes[head_bid].prev_buffer_id = readed_bid;
        mbdes[readed_bid].next_buffer_id = head_bid;
        mbdes[readed_bid].prev_buffer_id = readed_bid;
        head_bid = readed_bid;
    }
    void LRUInsert(int inserted_bid,MBuf_des* mbdes) {
        if (size == 0) {
            head_bid = tail_bid = inserted_bid;
            mbdes[head_bid].next_buffer_id = mbdes[head_bid].prev_buffer_id = inserted_bid;
        } else { 
            mbdes[head_bid].prev_buffer_id = inserted_bid;
            mbdes[inserted_bid].next_buffer_id = head_bid;
            mbdes[inserted_bid].prev_buffer_id = inserted_bid;
            head_bid = inserted_bid;
        }
    }

public:
    MBuffer() {
        memset(buffer2page, -1, sizeof(buffer2page));

        head_bid = tail_bid = -1;
        size = 0;
    }

    int FixPage(int node_id, EBuf_pool &thebuffer,MBuf_des* mbdes,char** mbpool) {
        MBuf_id mbuf_id;
        mbuf_id = node2page[node_id];
        int page_id = mbuf_id.page_id;
        int offset = mbuf_id.offset;

        char dstStringTemp[1024];
        memset((char*)dstStringTemp, 0 ,1024);

        if (page2buffer.find(page_id) != page2buffer.end()) {
            printf("MBuffer HIT!\n");
            int mbid = page2buffer[page_id];

            memcpy(dstStringTemp, mbpool[mbid]+offset*1024, 1024);
            if(!aes_decrypt(dstStringTemp,key,thebuffer.field,1024))
            {
                printf("decrypt error\n");
            }
            return 0;
        }
        
        int target_bid = size;
        bool do_insert = false; 

        if (size >= MBUFFER_NUM_MAX_SIZE) { 
            target_bid = SelectVictim();
            if (mbdes[target_bid].dirty == 1) {
                WriteBuffer(target_bid,mbpool);
            }
        } else {
            target_bid = size;
            do_insert = true;
        }

        ReadPage(page_id, mbpool[target_bid]);
        mbdes[target_bid].update(page_id, target_bid, 0, false);
        do_insert ? LRUInsert(target_bid,mbdes) : LRUUpdate(target_bid,mbdes);
        do_insert ? size += 1 : 0;

        memcpy(dstStringTemp, mbpool[target_bid]+offset*1024, 1024);
        if(!aes_decrypt(dstStringTemp,key,thebuffer.field,1024))
        {
            printf("decrypt error\n");
        }

        page2buffer[page_id] = target_bid;
        buffer2page[target_bid] = page_id;
        return 0;
    }

    void UpdatePage(int node_id, EBuf_pool &thebuffer,MBuf_des* mbdes,char** mbpool) {
        
        MBuf_id mbuf_id;
        mbuf_id = node2page[node_id];
        int page_id = mbuf_id.page_id;
        int offset = mbuf_id.offset;

        char dstStringTemp[1024];
        memset((char*)dstStringTemp, 0 ,1024);
        if(!aes_encrypt(thebuffer.field,key,dstStringTemp,1024))
        {
            printf("encrypt error\n");
        }

        if (page2buffer.find(page_id) != page2buffer.end()) {
            printf("MBuffer HIT!\n");
            int mbid = page2buffer[page_id];
            memcpy(mbpool[mbid]+offset*1024, dstStringTemp, 1024);

            SetDirty(mbid,mbdes);
            LRUUpdate(mbid,mbdes);

        } else { 
            int target_bid = -1, ret = 0;
            bool do_insert = false;
            if (size >= MBUFFER_NUM_MAX_SIZE) { 
                target_bid = SelectVictim();
                if (mbdes[target_bid].dirty) {
                    ocall_writepage(buffer2page[target_bid], mbpool[target_bid]);

                }
            } else { 
                target_bid = size;
                do_insert = true;
            }

            memcpy(mbpool[target_bid]+offset*1024, dstStringTemp, 1024);
            mbdes[target_bid].update(page_id, target_bid, 0, false);
            SetDirty(target_bid,mbdes);
            do_insert ? LRUInsert(target_bid,mbdes) : LRUUpdate(target_bid,mbdes);
            do_insert ? size += 1 : 0;
            page2buffer[page_id] = target_bid;
            buffer2page[target_bid] = page_id;
        }
    }
    void UnfixPage(int node_id,MBuf_des* mbdes) {

        MBuf_id mbuf_id;
        mbuf_id = node2page[node_id];
        int page_id = mbuf_id.page_id;

        if (page2buffer.find(page_id) != page2buffer.end())
            mbdes[page2buffer[page_id]].count -= 1;
    }

    void WriteDirtys(MBuf_des* mbdes,char** mbpool) {
        for (int bid = 0; bid < size; bid++) {
            if (mbdes[bid].dirty) {
                ocall_writepage(buffer2page[bid], mbpool[bid]);

                mbdes[bid].dirty = false;
            }
        }
    }
};

#endif // BUFFER_MGR_H
