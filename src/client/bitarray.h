#ifndef _BITARRAY_
#define _BITARRAY_
#include <cstdint>
#include <iostream>
#include <cstring>

using namespace std;

class bitarray{
    public:
        size_t array_size;
        size_t bf_size;
        size_t* bit_array;

        bitarray(size_t size){
            this->array_size = size;
            this->bf_size = (this->array_size + 63) >> 6;
            this->bit_array = new size_t[this->bf_size];

            size_t allocate_size = this->bf_size * 8;
            for(int i = 0; i < this->bf_size; i++){
                this->bit_array[i] = 0;
            }
        }

        void setbit(size_t k){
            this->bit_array[(k>>6)] |= (1ULL << (k&63));
        }

        void clearbit(size_t k){
            this->bit_array[(k>>6)] &= ~(1ULL << (k&63));
        }

        bool checkbit(size_t k){
            return (this->bit_array[(k>>6)] & (1ULL << (k&63)));
        }

        size_t getcount(){
            size_t countx = 0;
            size_t x;
            for (size_t i = 0; i < this->bf_size; i++){
                x = this->bit_array[i];
                while(x){
                    x = x & (x - 1);
                    countx ++;
                }
            }
            return countx;
        }

        void andop(size_t* B){
            for(size_t i = 0; i < this->bf_size; i++){
                this->bit_array[i] &= B[i];
            }
        }
};

#endif