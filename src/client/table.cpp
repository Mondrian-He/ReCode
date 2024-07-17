#include "table.h"
#include "encode.h"
#include <sstream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include "time.h"

Table::Table(string fileName, string tableName) : _fileName(fileName), 
  _tableName(tableName) {
  _file.open(_fileName.c_str(), fstream::in);
  if (!_file.good()) {
    cerr << "Table reading error for file : " << _fileName << endl; 
  }
  Parse();
}

vector<pair<string,int>> Table::ExtractStringColumn(int colId, const Table& table) {
  vector<pair<string,int>> column;
  for (int i = 0; i < table._table.size(); i++) {
    column.push_back(pair<string,int>(table._table[i][colId], i));
  }
  return column;
}

int Table::FindColumn(string colName) const {
  int columnId = -1;
  for (int i = 0; i < _headers.size(); i++) {
    if(_headers[i] == colName) {
      columnId = i;
    }
  }
  if (columnId == -1) {
    cerr << "Column " << colName << " not found." << endl;
  }
  return columnId;
}

void Table::Parse() {
  string line;
  string temp;

  getline(_file, line);
  istringstream iss(line);
  while (getline(iss, temp, ',')) {
    _headers.push_back (temp);
  }
  
  while (getline(_file, line)) {
    istringstream iss(line);
    vector<string> values;
    while (getline(iss, temp, ',')) {
      values.push_back(temp);
    }
    _table.push_back (values);
  }
}


//伪代码3，索引建立
void Table::BuildIndex(string key_file,string eneq_file,string EqColName)
//key_file是存储密钥的文件路径，eneq_file是存储加密索引的文件路径，EqColName是需要构建索引的列的名
{
  int EqColId = FindColumn(EqColName);
  if (EqColId == -1) {
    return;
  }

  vector<pair<string, int>> Col = ExtractStringColumn(EqColId, *this);//Col向量中是提取的列中数据，包含列值和行索引

  unordered_map<string,Pos> HT;
  int len = Col.size();
  Pos pos(len);//存储位置信息

  for (auto m : Col) {
    if(HT.find(m.first) != HT.end())
    {
      HT[m.first].count += 1;//如果已经存在
    }
    else
    {
      HT[m.first] = pos;//不存在则初始化，start和end以及count都是1
    }
  }

  //处理了所有的start
  int ptr = 0;
  for (auto m : Col) {

    if(HT[m.first].count != 1)//对于所有不止1个的eq
    {
      if(HT[m.first].flag == 0)
      {
        HT[m.first].set(ptr,ptr);
        ptr += HT[m.first].count;
        HT[m.first].flag = 1;
      }      
    }
    else
    {
      HT[m.first].set(ptr,ptr);
      ptr += HT[m.first].count;
    }
  }

  for (auto m : Col) {
    int temp_end = HT[m.first].end;
    HT[m.first].arr[temp_end] = m.second;
    HT[m.first].end++;
  }

  ofstream eneq(eneq_file);
  ifstream keyfile(key_file);
  string key;
  string iv;
  keyfile >> key >> iv;

  size_t ll=0;  
  //p是HT中的每个元素，应该是唯一值
  for(auto p : HT) {
    
    ll++;
    string index_encode;
    string index_encode_base64;
    index_encode = aes_256_cbc_encode(key,iv,p.first);
    index_encode_base64 = base64_encode(index_encode.c_str(), index_encode.length());
    eneq << index_encode_base64;
    if(ll < HT.size())
      eneq << endl;
      
  }

  eneq.close();

}
