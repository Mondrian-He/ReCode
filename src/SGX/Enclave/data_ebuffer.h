#ifndef EBUFFER_D_H
#define EBUFFER_D_H

#include "ebuf_header.h"
#include "data_mbuffer.h"

using std::string;
using std::to_string;
using std::unordered_map;
using std::map;
using namespace std;



struct Data_EBuffer {
public:
    unordered_map<int,int> r2buffer;//哈希表
    int buffer2r[EBUFFER_DATA_NUM_MAX_SIZE];
    EBuf_pool *ebuf_pool;
    int pageid,offset,rid;
    int size;

    Data_MBuffer* mbuffer;

    void WriteBuffer(int buffer_id,MBuf_des* mbdes,char** mbpool) {
        printf("EBuffer.WriteData\n");
        mbuffer->UpdatePage(buffer2r[buffer_id],ebuf_pool[buffer_id],mbdes,mbpool);      
    }
    void ReadPage(int rid, EBuf_pool &thebuffer,MBuf_des* mbdes,char** mbpool) {
        mbuffer->FixPage(rid,thebuffer,mbdes,mbpool);
    }

public:
    Data_EBuffer(int r) {
        memset(buffer2r, -1, sizeof(buffer2r));
        size = 0;
        pageid = 1;
        offset = -1;
        rid = r;
        mbuffer = new Data_MBuffer;
        ebuf_pool = new EBuf_pool[EBUFFER_DATA_NUM_MAX_SIZE];
    }
    ~Data_EBuffer() {
        delete[] ebuf_pool;
    }

    char* SearchData(int rid,MBuf_des* mbdes,char** mbpool) {

		if (r2buffer.find(rid) != r2buffer.end()) {
			int bfid = r2buffer[rid];
			return ebuf_pool[bfid].field;	
		}
		else{
			int target_bid = size;
            bool do_insert = false;
			if (size >= EBUFFER_DATA_NUM_MAX_SIZE) { 
				target_bid = 1;
				WriteBuffer(target_bid,mbdes,mbpool);
				r2buffer.erase(buffer2r[target_bid]);

			} else {
				target_bid = size;
                do_insert = true;
			}
			ReadPage(rid,ebuf_pool[target_bid],mbdes,mbpool);
            do_insert ? size += 1 : 0;
			r2buffer[rid] = target_bid;
			buffer2r[target_bid] = rid;
			return ebuf_pool[target_bid].field;
		}

    }

    void InsertData(char* newdata,MBuf_des* mbdes,char** mbpool) {

        rid++;
        offset++;
        if(offset == 4)
        {
            offset = 0;
            pageid++;
        }
        HeapData hd;
        hd.set(pageid,offset,rid,newdata);
        r2page[rid] = hd;

		int bid = size;
        bool do_insert = false;
		if (size >= EBUFFER_DATA_NUM_MAX_SIZE) { 
			bid = 0;
            WriteBuffer(bid,mbdes,mbpool);
            r2buffer.erase(buffer2r[bid]);
		} else { 
			bid = size;
            do_insert = true;
		}
		memcpy(ebuf_pool[bid].field,newdata,EBUFFER_SIZE);
        do_insert ? size += 1 : 0;
        r2buffer[rid] = bid;
		buffer2r[bid] = rid;
    }
};

#endif // EBUFFER_D_H
