# Preparation	

Before conducting TRA and SBF tests, you should run the  'data.py'  file located in the  'cmp_data'  directory to ensure the generation of both the data set and the query set.

```cmd
python data.py
```

By adjusting the common_entries variable in data.py, you can control the total number of intersecting elements across the ten sets.

# Code Structure

├── data
│    ├── cmp_data            # Save sets element and query
│    │    ├── data.py        # To generate ten sets and create common_entries
│    │    ├── query          # The query file
│    │    ├── set_0 
|    |    |    ……
│    │    |    ……
|    |    └── set_9 
|
├── TRA			     # To run TRA test
│    ├── BIGSI_MAIN.cpp
├── SBF		             # To run SBF test
│    ├── SBF.cpp
├── CSCBF                    # CSC-Simgod2021 source code
│    ├── BIGSI.cpp
│    ├── BIGSI.h
│    ├── bitarray.cpp
│    ├── bitarray.h
│    ├── BloomFilter.cpp
│    ├── BloomFilter.h
│    ├── CSCBF.cpp
│    ├── CSCBF.h
│    ├── MurmurHash3.cpp
│    ├── MurmurHash3.h

# How to  Test?

To run TRA, one can compile it using

```cmd
g++ BIGSI_MAIN.cpp  MurmurHash3.cpp BloomFilter.cpp BIGSI.cpp bitarray.cpp  -o tra
```

and then run ./tra

To run SBF, one can compile it using

```cmd
g++ SBF.cpp  MurmurHash3.cpp  CSCBF.cpp bitarray.cpp  -o sbf
```

and then run ./sbf


