#ifndef _TABLE_
#define _TABLE_

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <fstream>

using namespace std;

class Table {

public:
  Table (string name, string tableName);
  string GetName() {return _tableName;}
  void BuildIndex(string key_file,string eneq_file,string EqColName);
    
private:
  Table();
  string _tableName;
  string _fileName;
  fstream _file;
  vector<string> _headers;
  vector< vector<string>> _table;
  vector<pair<string, int>> ExtractStringColumn(int colId, const Table& table);
  int FindColumn(string colName) const;
  void Parse();
};

class Pos{

public:  
  int count;
  int start;
  int end;
  int flag;
  int *arr;

public:
  Pos(){}
  Pos(int len)
  {
    count = 1;
    start = 0;
    end = 0;
    flag = 0;
    arr = new int[len];
  }
  void set(int s,int e)
  {
    start = s;
    end = e;
  }

};

#endif
