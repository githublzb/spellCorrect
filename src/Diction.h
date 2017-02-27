#ifndef INCLUDE_DICTION_H
#define INCLUDE_DICTION_H
#include <string>
#include <map>
#include <set>
#include "MutexLock.h"


class Diction
{
public:
    static Diction *getInstance();
    
    std::map<std::string, int> & getDictMap()
    {
        return dict_map; 
        
    }
private:
    Diction();
    ~Diction() {}
    void createCHDict();
    void createENDict();
    void getExcludeSet();
    void getSelfDat();
    void traverseDir(const char *dir);
    void readFileToMap(const char *file);
private:
    std::map<std::string, int> dict_map;
    std::set<std::string> exclude_set;
    static Diction *instance;
    static MutexLock lock;
     
    
}; 

#endif

