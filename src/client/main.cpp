#include "table.h"
#include "encode.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <functional>
#include <memory>
#include <map>

using namespace std;

shared_ptr<Table> FindTable(string table, vector<shared_ptr<Table>>& tables) {
  shared_ptr<Table> ptr;
  for (auto t : tables) {
    if (t->GetName() == table) {
      ptr = t;
      break;
    }
  }
  return ptr;
}

int main(int argc, char *argv[]) {
  string line;
  vector<shared_ptr<Table>> tables;

  cout << "Please load csv files, sample command: "
       << "load a test1.csv" << endl;
  while (getline(cin, line)) {
    istringstream iss(line);
    string operation;
    iss >> operation;

    if (operation == "load") {
      int num = 0;
      iss >> num;
      for(int i=0;i<num;i++){
        string file;
        iss >> file;
        shared_ptr<Table> tablePtr(new Table(file, to_string(i)));
        tables.push_back(tablePtr);
      }    
      
    } else if (operation == "buildindex") {
      string table;
      string file1, file2;
      string eqcol;
      string on;
      iss >> table >> file1 >> file2 >> on >> eqcol;
   
      shared_ptr<Table> tablePtr = FindTable(table, tables);
      if (!tablePtr) {
        cerr << "Table: " << table << " not found!" << endl;
      } else {
        tablePtr->BuildIndex(file1,file2,eqcol);
      }
    } else if (operation == "rangejoin") {
        //写入加密查询中
        ofstream enquery("/var/lib/mysql/file/enquery.txt");
        // AES key
        string key = "12345678901234561234567890123456";
        string iv = "1000000000000000";
        long long longiv = atoll(iv.c_str());//ASCII转换成longlong
        longiv++;//保证每次参与加密的iv都是不一样的

        string num,On;
        iss >> num >> On;
        string num_encode = aes_256_cbc_encode(key,iv,num);
        string num_encode_base64 = base64_encode(num_encode.c_str(), num_encode.length());
        enquery << num_encode_base64 << " ";

        for(int i=0;i<atoi(num.c_str());i++){
          string table;
          iss >> table;
          shared_ptr<Table> tablePtr = FindTable(table, tables);
          if (!tablePtr) {
            cerr << "Table: " << table << " not found!" << endl;
          } else{
            string newiv = to_string(longiv);
            longiv++;
            string table_encode = aes_256_cbc_encode(key,newiv,table);
            string table_encode_base64 = base64_encode(table_encode.c_str(), table_encode.length());
            enquery << table_encode_base64 << " ";
          }
        }
        enquery.close();
    } else if (operation == "exit") {
      return 0;
    } else {
      cerr << "Unknown operation!" << endl;
      cerr << "Supported operations are: load, select, join, compute, aggregation" 
        << endl;
    }
  }
  return 0;
}
