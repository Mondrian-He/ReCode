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
#include "BloomFilter.h"

#define NUM_DB 10
#define HASH_NUM 3
#define REP_TIME 3
#define PART_NUM 62
#define TOTAL_CAP 60000000

using namespace std;

int main()
{

    // initial
    BIGSI baseline_bigsi(NUM_DB, HASH_NUM, TOTAL_CAP);
    // data insertion
    string file_prefix = "./data/cmp_data/set_";
    string lines;
    lines.clear();
    int j = 0;
    // printf("-----------------------------------------\n");
    // cout << "data insertion" << endl;
    // printf("-----------------------------------------\n");
    for (int i = 0; i < NUM_DB; i++)
    {
        string file_name = file_prefix + to_string(i);
        ifstream file(file_name);
        if (!file.is_open())
        {
            cout << "open file error!" << endl;
            return 0;
        }
        while (getline(file, lines))
        {
            if (file.eof())
            {
                break;
            }
            baseline_bigsi.insertion(i, lines);
            lines.clear();
        }
        // printf("%d set insert successful\n", i);
        file.close();
        file_name.clear();
    }
    // printf("-----------------------------------------\n");

    /*** query  ***/
    string query_file1 = "./data/cmp_data/query";
    string line;
    line.clear();
    ifstream new_query(query_file1);
    stringstream sstr;
    set<string> query_element;
    query_element.clear();
    string element;
    element.clear();
    size_t set_ID;
    map<string, set<size_t>> ground_truth;
    while (getline(new_query, line))
    {
        if (new_query.eof())
        {
            break;
        }
        sstr << line;
        sstr >> element;
        query_element.insert(element);
        sstr.clear();
    }

    /*------------------------------------------------------------------------------*/
    chrono::time_point<chrono::high_resolution_clock> start = chrono::high_resolution_clock::now();
    for (size_t i = 1; i < 10; i++)
    {
        // chrono::time_point<chrono::high_resolution_clock> t0 = chrono::high_resolution_clock::now();
        std::vector<bool> Cmp_Array(baseline_bigsi.single_capacity, false);
        // chrono::time_point<chrono::high_resolution_clock> t0 = chrono::high_resolution_clock::now();
        Cmp_Array = baseline_bigsi.query(0, i);
        set<string> query_result;query_result.clear();
        for (auto &ele : query_element)
        {
            int res = 1;
            vector<size_t> check_locations = BF_Hash(ele, baseline_bigsi.k, baseline_bigsi.seed, baseline_bigsi.single_capacity);
            for (auto &location : check_locations)
            {
                res &= Cmp_Array[location];
            }
            if (res)
            {
                query_result.insert(ele);
            }
        }
        // chrono::time_point<chrono::high_resolution_clock> t1 = chrono::high_resolution_clock::now();
        // float time = ((t1 - t0).count() / 1000000.0);
        // printf("The  query time of set %d and %d is %f ms\n", 0, i, time);
    }
    chrono::time_point<chrono::high_resolution_clock> end = chrono::high_resolution_clock::now();
    float time = ((end - start).count() / 1000000.0);
    printf("BF:The  query total time is %f ms\n", time);
    /*------------------------------------------------------------------------------*/

    return 0;
}
