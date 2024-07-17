#include <string>
#include <fstream>
#include <set>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>

#include "BIGSI.h"
#include "CSCBF.h"

#define NUM_DB 10
#define HASH_NUM 3
#define REP_TIME 3
#define PART_NUM 62
#define TOTAL_CAP 60000000

using namespace std;

int main(){

    //initial
    CSCBF cscbf(REP_TIME, PART_NUM, TOTAL_CAP, HASH_NUM, NUM_DB); // CSC-RAMBO
    // data insertion
    string file_prefix = "./data/cmp_data/set_";
    string lines; lines.clear(); int j = 0;
    // printf("-----------------------------------------\n");
    // cout << "data insertion" << endl;
    // printf("-----------------------------------------\n");
    for(int i=0; i<NUM_DB; i++){
        string file_name = file_prefix + to_string(i);
        ifstream file(file_name);
        if(!file.is_open()){
            cout << "open file error!" << endl;
            return 0;
        }
        while(getline(file, lines)){
            if(file.eof()){
                break;
            }
            // printf("%s\n", lines.c_str()); // 使用c_str()获取C风格字符串
            cscbf.insertion(to_string(i), lines);
            lines.clear();
        }
        // printf("%d set insert successful\n",i);
        file.close();
        file_name.clear();
    }
    // printf("-----------------------------------------\n");
    // printf("\n");



    /*** query  ***/
    string query_file1 = "./data/cmp_data/query";
    string line; line.clear();
    ifstream new_query(query_file1);
    stringstream sstr;
    set<string> query_element; query_element.clear();                       
    set<string> s0; s0.clear();                       
    set<string> s1; s1.clear();                       
    set<string> s2; s2.clear();                                                   
    set<string> s3; s3.clear();                                                   
    string element; element.clear(); size_t set_ID;                            
    map< string, set<size_t> > ground_truth;                            
    while (getline(new_query, line))                            
    {                            
        if(new_query.eof()){                            
            break;                            
        }                            
        sstr << line;
        sstr >> element;
        query_element.insert(element);
        sstr.clear(); 
    }


/*----------------------------------------------------------------------------------------------------*/
    chrono::time_point<chrono::high_resolution_clock> t0 = chrono::high_resolution_clock::now();
    set<string> query_result;query_result.clear();
    for(auto &ele : query_element){
        int res = 1;
        for (int set_ID = 1; set_ID < NUM_DB; set_ID++)
        {
            int cscbf_res = cscbf.query(set_ID, ele);
            res &= cscbf_res;
            if(!res){
                break;
            }
                
        }
        if(res){
            query_result.insert(ele);
        }
    }
    chrono::time_point<chrono::high_resolution_clock> t1 = chrono::high_resolution_clock::now();
    float time = ((t1-t0).count()/1000000.0);
    printf("SBF:The  query time is %f ms\n",time);
    
/*----------------------------------------------------------------------------------------------------*/
    // for (const auto &element : query_result)
    // {
    //     std::cout << element << std::endl;
    // }
    system("pause");
    return 0;
}
