/*
 * Copyright (C) 2011-2020 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "Enclave_t.h" /* print_string */
#include "baddtree.h"
#include "data_ebuffer.h"
#include "encode.h"
#include "kdtree.h"
#include "BloomFilter.h"
#include "BIGSI.h"
#include "CSCBF.h"
#include "Ocall_wrappers.h"

#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <vector>
#include <time.h>

#include <functional>
#include <memory>
#include <unordered_map>

using namespace BAT;

string cbc_key1 = "12345678901234561234567890123456";
string cbc_key2 = "98765432101234569876543210123456";
string iv0 = "1000000000000000";
string iv1 = "1111111111111111";
string iv2 = "2000000000000000";
string iv3 = "2111111111111111";
string iv4 = "3000000000000000";
string iv5 = "3111111111111111";
string iv6 = "4000000000000000";
string iv7 = "4111111111111111";
string iv8 = "5000000000000000";
string iv9 = "5111111111111111";
long long liv1 = atoll(iv0.c_str());

BAddTree<int,k_r>* tree;

void ecall_init(int order)
{
    printf("ecall_init\n");
    for(int i=1;i<=EBUFFER_SIZE;i++)
    {
        MBuf_id mbuf_id;
        mbuf_id.page_id = (i-1)/4 + 1;
        mbuf_id.offset = (i-1)%4;
        node2page[i] = mbuf_id;
    }
    for(int i = 0; i < 16; i++)
    {
        key[i] = 32 + i;
    }

    tree = new BAddTree<int,k_r>(order);

}

int ecall_search(int key,void* mbdes,char** mbpool)
{
    printf("ecall_search\n");
    return tree->find(key,(MBuf_des*)mbdes,mbpool)->rid;
    
}

void ecall_insert(void* key_rid,void* mbdes,char** mbpool)
{
    printf("ecall_insert\n");
    k_r* kr = (k_r*)key_rid;
    tree->insert(*kr,(MBuf_des*)mbdes,mbpool);
    
}

auto itf = [&](deque <k_r*>&e) {
	int _s = e.size();
	for (int i = 0; i < _s; ++i) {
		printf("%d ", e[i]->k);
	}
};

void ecall_traversal()
{
    printf("ebuffer size: %d\n",tree->ebuffer->size);
    printf("tree size: %d\n",tree->size());
    printf("start traversal:\n");
    tree->list_traversal(itf);
}

Data_EBuffer *Def;
Data_EBuffer *Def1;

void ecall_data_init()
{
    printf("ecall_data_init\n");

    for(int i = 0; i < 16; i++)
    {
        key2[i] = 32 + i;
    }

    Def = new Data_EBuffer(-1);
}
char* ecall_data_search(int rid,void* mbdes,char** mbpool)
{
    printf("ecall_data_search\n");
    return Def->SearchData(rid,(MBuf_des*)mbdes,mbpool);
    
}

void ecall_data_insert(char* newdata,void* mbdes,char** mbpool)
{
    printf("ecall_data_insert\n");
    Def->InsertData(newdata,(MBuf_des*)mbdes,mbpool);
    
}

void ecall_joinsearch2(char** ein0,char** ein1,char** ein2,char** ein3,char** ein4,char** ein5,char** ein6,char** ein7,char** ein8,char** ein9,void* mbdes,char** mbpool)
{ //参数为加密的输入数据，每个输入数据都是一个字符串数组
    ocall_open_result();//从受信环境进入到不受信的环境中去
    ocall_open_enquery();

    char *enumber = new char[25];//字符数组
    ocall_read_s(enumber,25);//读取加密的表数量
    string senumber = enumber;
    string number_decode_base64 = base64_decode(senumber);
    //第一个参数是秘钥，第二个是初始化向量
	string sn = aes_256_cbc_decode(cbc_key1,iv0,number_decode_base64);
    int num = atoi(sn.c_str());//将字符串转换为整数
    liv1++;//保证每次用的iv都不一样

    vector<int> table;//table就是一个int类型的vector
    for(int i=0;i<num;i++){
        char *en = new char[25];//分配一个25字节的字符数组
        ocall_read_s(en,25);//enquery>>s
        string sen = en;
        string niv1 = to_string(liv1);
        liv1++;
        string n_decode_base64 = base64_decode(sen);
        //因为niv1还没被用过，上一次liv1++完后，都没有被用过
	    string n = aes_256_cbc_decode(cbc_key1,niv1,n_decode_base64);
        table.push_back(atoi(n.c_str()));
    }

    CSCBF cscbf(REP_TIME, PART_NUM, TOTAL_CAP, HASH_NUM, num);
    vector< pair<int,int> > storage2;//存储连接操作结果的键值对向量。

    int len0 = 0;
    ocall_read_eneq0(&len0,ein0);
    // vector<string> index;
    map<string,int> result0;    

    int len1 = 0;
    ocall_read_eneq1(&len1,ein1);
    map<string,int> result1;

    for(int p=0;p<len0;p++)
    {
        string enindex = ein0[p];
        string p_decode_base64 = base64_decode(enindex);
	    string p_decode = aes_256_cbc_decode(cbc_key2,iv0,p_decode_base64);
        // index.push_back(p_decode);   
        //似乎就是Def1只存了第0号集合的，ein0
        Def1->InsertData((char*)p_decode.c_str(),(MBuf_des*)mbdes,mbpool);
        //插入到第0号集合中
        cscbf.insertion(to_string(0),p_decode);//整数0转为字符串，这个参数表示setid，后面表示key
    }
    

    if(num == 2){
        for(int p=0;p<len1;p++)
        {
            string enindex = ein1[p];
            string p_decode_base64 = base64_decode(enindex);
            string p_decode = aes_256_cbc_decode(cbc_key2,iv1, p_decode_base64);      	    
            cscbf.insertion(to_string(1),p_decode);

            int res = 1;
            for (int set_ID = 1; set_ID < num; set_ID++)//所以num就是表的数目
            //即为table的数目
            {
                // int cscbf_res = cscbf.query(set_ID,p_decode);
                int cscbf_res = cscbf.obfquery(set_ID,p_decode);
                res &= cscbf_res;//进行按位与
                if(!res){//但凡有一个不在，就直接表示不存在于交集中
                    break;
                }
                    
            }
            if(res){
                result1[p_decode] = p;//表示在所有集合中都存在
            }
        }
    }       

    if(num == 3 || num == 5 || num == 10){
        int len2 = 0;
        ocall_read_eneq2(&len2,ein2);
        for(int p=0;p<len1;p++)
        {
            string enindex = ein1[p];
            string p_decode_base64 = base64_decode(enindex);
            string p_decode = aes_256_cbc_decode(cbc_key2,iv1, p_decode_base64);      	    
            cscbf.insertion(to_string(1),p_decode);
        }
        if(num == 3){
            for(int p=0;p<len2;p++)
            {
                string enindex = ein2[p];
                string p_decode_base64 = base64_decode(enindex);
                string p_decode = aes_256_cbc_decode(cbc_key2,iv2,p_decode_base64);    	    
                cscbf.insertion(to_string(2),p_decode);

                int res = 1;
                for (int set_ID = 1; set_ID < num; set_ID++)
                {
                    // int cscbf_res = cscbf.query(set_ID,p_decode);
                    int cscbf_res = cscbf.obfquery(set_ID,p_decode);
                    res &= cscbf_res;
                    if(!res){
                        break;
                    }
                        
                }
                if(res){
                    result1[p_decode] = p;
                }
            }
        } else{
            int len3 = 0;
            ocall_read_eneq3(&len3,ein3);
            int len4 = 0;
            ocall_read_eneq4(&len4,ein4);
            for(int p=0;p<len2;p++)
            {
                string enindex = ein2[p];
                string p_decode_base64 = base64_decode(enindex);
                string p_decode = aes_256_cbc_decode(cbc_key2,iv2,p_decode_base64);     	    
                cscbf.insertion(to_string(2),p_decode);
            }
            for(int p=0;p<len3;p++)
            {
                string enindex = ein3[p];
                string p_decode_base64 = base64_decode(enindex);
                string p_decode = aes_256_cbc_decode(cbc_key2,iv3,p_decode_base64);    	    
                cscbf.insertion(to_string(3),p_decode);
            }            
            if(num == 5){
                for(int p=0;p<len4;p++)
                {
                    string enindex = ein4[p];
                    string p_decode_base64 = base64_decode(enindex);
                    string p_decode = aes_256_cbc_decode(cbc_key2,iv4,p_decode_base64);     	    
                    cscbf.insertion(to_string(4),p_decode);

                    int res = 1;
                    for (int set_ID = 1; set_ID < num; set_ID++)
                    {
                        // int cscbf_res = cscbf.query(set_ID,p_decode);
                        int cscbf_res = cscbf.obfquery(set_ID,p_decode);
                        res &= cscbf_res;
                        if(!res){
                            break;
                        }
                            
                    }
                    if(res){
                        result1[p_decode] = p;
                    }
                }
            } else{
                int len5 = 0;
                ocall_read_eneq5(&len5,ein5);
                int len6 = 0;
                ocall_read_eneq6(&len6,ein6);
                int len7 = 0;
                ocall_read_eneq7(&len7,ein7);
                int len8 = 0;
                ocall_read_eneq8(&len8,ein8);
                int len9 = 0;
                ocall_read_eneq9(&len9,ein9);
                for(int p=0;p<len4;p++)
                {
                    string enindex = ein4[p];
                    string p_decode_base64 = base64_decode(enindex);
                    string p_decode = aes_256_cbc_decode(cbc_key2,iv4,p_decode_base64);   	    
                    cscbf.insertion(to_string(4),p_decode);
                }                
                for(int p=0;p<len5;p++)
                {
                    string enindex = ein5[p];
                    string p_decode_base64 = base64_decode(enindex);
                    string p_decode = aes_256_cbc_decode(cbc_key2,iv5,p_decode_base64);      	    
                    cscbf.insertion(to_string(5),p_decode);
                }
                for(int p=0;p<len6;p++)
                {
                    string enindex = ein6[p];
                    string p_decode_base64 = base64_decode(enindex);
                    string p_decode = aes_256_cbc_decode(cbc_key2,iv6,p_decode_base64);     	    
                    cscbf.insertion(to_string(6),p_decode);
                }
                for(int p=0;p<len7;p++)
                {
                    string enindex = ein7[p];
                    string p_decode_base64 = base64_decode(enindex);
                    string p_decode = aes_256_cbc_decode(cbc_key2,iv7,p_decode_base64);      	    
                    cscbf.insertion(to_string(7),p_decode);
                }
                for(int p=0;p<len8;p++)
                {
                    string enindex = ein8[p];
                    string p_decode_base64 = base64_decode(enindex);
                    string p_decode = aes_256_cbc_decode(cbc_key2,iv8,p_decode_base64);      	    
                    cscbf.insertion(to_string(8),p_decode);
                }
                for(int p=0;p<len9;p++)
                {
                    string enindex = ein9[p];
                    string p_decode_base64 = base64_decode(enindex);
                    string p_decode = aes_256_cbc_decode(cbc_key2,iv9,p_decode_base64);      	    
                    cscbf.insertion(to_string(9),p_decode);

                    int res = 1;
                    for (int set_ID = 1; set_ID < num; set_ID++)
                    {
                        // int cscbf_res = cscbf.query(set_ID,p_decode);
                        int cscbf_res = cscbf.obfquery(set_ID,p_decode);
                        res &= cscbf_res;
                        if(!res){
                            break;
                        }
                            
                    }
                    if(res){
                        result1[p_decode] = p;
                    }
                }
            }
        }
    }

    //我感觉是为了进一步确认集合的准确性
    for(int j=0;j<len0;j++)
    {
        string str = Def1->SearchData(j,(MBuf_des*)mbdes,mbpool);//Def是data buffer
	    if(result1.find(str) != result1.end())
            result0[str] = j;
        
    }
    //其中first表示这个关键词（字符串）
    //其中second表示位置，即result1[p_decode] = p;
    unordered_map<string,int> HTE;//哈希表
    for(auto h1 : result0)
    {
        HTE[h1.first] = h1.second;
    }
    for(auto h2 : result1)
    {   //注意这个逻辑，是不存在，才会指向end
        if(HTE.find(h2.first) != HTE.end())//检查哈希表中是否存在该键值
        {
            storage2.push_back(pair<int,int>(HTE[h2.first],h2.second));
            ocall_write_result(atoi(h2.first.c_str()));
            ocall_writeendl_result();
        }
    }

    ocall_close_enquery();
    ocall_close_result();
}
