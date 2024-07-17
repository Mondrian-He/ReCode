#include <iostream>
#include <cstring>
#include <algorithm>
#include <immintrin.h>
#include <cstdlib>
#include <chrono>
#include <cmath>

#include "bitarray.h"
#include "CSCBF.h"
#include "MurmurHash3.h"

using namespace std;

/*
1、将输入的字符串 key 分区成 r 个不同的哈希值，然后取模 b，以便将它们映射到一个较小的范围内
2、将数据分布到r个分区的b位置，即每个分区都需要插入标识
*/
uint32_t concatenate(uint16_t i, uint16_t j) {
    // 使用位移和按位或操作来合并两个数
    return (uint32_t)i << 16 | j;
}

//g函数
vector<size_t> CSCBF::partitionHash(size_t set_id , string key){
    vector<size_t> Hash;
    size_t op;
    for(int i=0; i<this->r; i++){
        // MurmurHash3_x86_32(key.c_str(), key.size(), i, &op);
        uint32_t i_j_combined = concatenate(i, set_id);
        MurmurHash3_x86_32(key.c_str(), key.size(), i_j_combined, &op);
        Hash.push_back(op);
    }
    return Hash;
}

/*
1、将输入的字符串 key 生成 k 个不同的位置哈希值以便将它们映射到一个较小的范围内
2、得到键值在分区中的位置
*/
//h(x)%m
vector<size_t> CSCBF::locationHash(string key){ 
    vector<size_t> Locations;
    size_t op;
    //key.c_str()返回字符串key的C风格表示
    for(int i=0; i<this->k; i++){
        MurmurHash3_x86_128(key.c_str(), key.size(), this->seed[i], &op);
        Locations.push_back(op % this->single_capacity);
    }
    return Locations;
}

//初始化相关参数
CSCBF::CSCBF(size_t R, size_t B, size_t total_usage, int k, int m){
    this->r = R;
    this->b = B;
    this->capacity = total_usage;
    this->single_capacity = size_t(this->capacity / (this->r));
    this->k = k;
    this->mask = 1ULL; //设置成员变量mask为1的64位无符号整数
    this->set_num = m;

    this->seed = new size_t[this->k];
    this->partition_seed = new size_t[this->r];

    //生成伪随机数作为哈希种子值
    random_device bf;
    mt19937 seed_ge(bf());
    uniform_int_distribution<size_t> dis(0, 1000000);
    for(int i = 0; i < this->k; i++){
        this->seed[i] = dis(seed_ge);
    }
    for(int i = 0; i < this->r; i++){
        this->partition_seed[i] = dis(seed_ge);
    }

    //创建一个指向 bitarray 指针的数组，长度为 r，用于存储每个分区的位数组
    this->CSCBF_array = new bitarray*[this->r];
    for(int r=0; r < this->r; r++){
        this->CSCBF_array[r] = new bitarray(this->single_capacity);
    }
    CSCBF_partition = new set<size_t>[this->r * this->b];//存储数据集标识与分区之间关联关系的数据结构
    // CSCBF::createPartition();

    this->array_length = (this->b + 63) >> 6;//表示一个数组所需要的长度，使其能够容纳 b 位
    this->copy_array = new size_t[this->array_length];
    this->mask_array = new size_t[this->array_length];
    this->result = new size_t[this->array_length];

    this->al_size = this->array_length * 8;//总位数
    memset(this->copy_array, 0, this->al_size);
    memset(this->mask_array, 255, this->al_size);
    memset(this->result, 0, this->al_size);
    this->time = 0.0;
}

//根据集合标识符和哈希值，将不同的集合分配到不同的分区中，将一组元素分布到不同的分区中
// void CSCBF::createPartition(){
//     for(int i=0; i<this->set_num; i++){
//         vector<size_t> partition = partitionHash(to_string(i));
//         for(int r=0; r < this->r; r++){
//             CSCBF_partition[partition[r] + this->b * r].insert(i);
//         }
//     }
// }

void print(vector<size_t> hash_res, vector<size_t> locations)
{
    std::cout << "hash_res: ";
    for (size_t hash_value : hash_res)
    {
        std::cout << hash_value << " ";
    }
    std::cout << std::endl;
    std::cout << "locations: ";
    for (size_t location : locations)
    {
        std::cout << location << " ";
    }
    std::cout << std::endl;
}

//将一组键插入到布隆过滤器数据结构中。它首先根据数据集标识和键生成相应的分区和位置哈希值，然后将每个键插入到相应的分区中
void CSCBF::insertion(string setID, string keys){

    // printf("goto the insert funciton\n");
    size_t offset_location = 0;
    int set_id = atoi(setID.c_str());
    vector<size_t> hash_res = partitionHash(set_id,keys);//生成一组分区哈希值，这些值将用于确定键在哪些分区中插入
    vector<size_t> locations = locationHash(keys);//生成一组位置哈希值，这些值表示要在布隆过滤器的哪些位置插入键
    // print(hash_res,locations);//调试
    for(int r=0; r < this->r; r++){
        for(auto &location : locations){
            offset_location = (location + hash_res[r]) % this->single_capacity;
            // printf("offset_location = %d\n",offset_location);
            this->CSCBF_array[r]->setbit(offset_location);
            // if(offset_location < this->single_capacity){
            //     this->CSCBF_array[r]->setbit(offset_location);
            // }
            // else{
            //     this->CSCBF_array[r]->setbit(offset_location - this->single_capacity);
            // }

        }
        // printf("%d round exit\n",r);
    }
}


int16_t CSCBF::query(size_t set_id,string query_key){

    // set<size_t> res; res.clear();
    int res=1,ret = 1;
    bitarray k1(this->set_num);
    int count;
    size_t offset_location = 0;

    //获取当前时间戳并存储在 t0 中，用于测量查询所需的时间
    chrono::time_point<chrono::high_resolution_clock> t0 = chrono::high_resolution_clock::now();
    vector<size_t> check_locations = locationHash(query_key);//h(x)%m
    vector<size_t> offset = partitionHash(set_id,query_key); //g函数
    // print(offset,check_locations);//debug
    for(int r = 0;r<this->r;r++){
        for(auto &location:check_locations){
            offset_location = (location + offset[r]) % this->single_capacity;
            res = res&this->CSCBF_array[r]->checkbit(offset_location);
            // printf("offset = %d,flag = %d\n",offset_location,this->CSCBF_array[r]->checkbit(offset_location));
        }
        ret  = ret &res;
    }
    chrono::time_point<chrono::high_resolution_clock> t1 = chrono::high_resolution_clock::now();
    this->time += ((t1-t0).count()/1000000000.0);
    return ret;
}

//无越界
void CSCBF::CopyArray(size_t* src_array, size_t* tgt_array, size_t begin_location){

    //确定从位数组的哪个块开始复制数据，以及在起始块中有多少位可供复制
    size_t start = begin_location / 64; //计算是哪一个块
    size_t start_offset = begin_location & 63;//计算在该块中的偏移量
    size_t end_offset = 64 - start_offset;

    int i = 0;
    while(i < this->array_length){
        tgt_array[i] = (src_array[start+1] << end_offset) | (src_array[start] >> start_offset);
        i++; start++; 
    }

}

//有越界
void CSCBF::CopyArray2(size_t* src_array, size_t* tgt_array, size_t begin_location){

    size_t start = begin_location / 64;
    size_t start_offset = begin_location & 63;
    size_t end_offset = 64 - start_offset;
    size_t dis = this->array_length - 1;

    int i = 0;
    while(i < this->array_length){

        if(start < dis){
            tgt_array[i] = (src_array[start+1] << end_offset) | (src_array[start] >> start_offset);
            start ++;   
        }
        else if(start == dis){
            tgt_array[i] = (src_array[0] << end_offset) | (src_array[start] >> start_offset);
            start = 0;
        }
        i++;
    }
}
