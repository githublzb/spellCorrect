#ifndef INCLUDE_INDEX_H
#define INCLUDE_INDEX_H

#include <vector>
#include <unordered_map>
#include <string>
#include <set>
#include "MutexLock.h"

class Index
{
public:
    static Index * getInstance();
    typedef std::vector<std::pair<std::string, int>> IndexVecType;
    typedef std::unordered_map<std::string, std::set<int>> IndexMapType;
    void buildIndexFromDiction();

    IndexMapType &getIndexMap() {
        return index_map;
    }
    IndexVecType & getIndexVec() {
        return index_vec;
    }
    IndexMapType & getPinyin() {
        return pinyin;
    }
    
private:
    Index();
    ~Index();

    static MutexLock lock;
    static Index *instance; 
    IndexVecType index_vec;
    IndexMapType index_map;
    IndexMapType pinyin;
};


#endif

 
