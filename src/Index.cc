#include "Index.h"
#include "Diction.h"
#include <sstream>
#include <string.h>
#include <utility>
#include "editdistance.h"
#include "ChToPinyin.h"

using namespace std;
Index *Index::instance = NULL;
MutexLock Index::lock;

Index * Index::getInstance()
{
    if (instance == NULL) {
        lock.lock();
        if (instance == NULL) {
            instance = new Index();
        }
        lock.unlock();
    }
    return instance;
}

void Index::buildIndexFromDiction()
{
    Diction *dict = Diction::getInstance();
    std::map<std::string, int>&  dict_map = dict->getDictMap();
    std::map<std::string, int>::iterator iter = dict_map.begin();
    for (; iter != dict_map.end(); ++iter) {
        index_vec.push_back(make_pair(iter->first, iter->second));
        
    }
         
    for (int index = 0; index != index_vec.size(); ++index) {
        std::string ret = ChineseToPinyin(index_vec[index].first);
        pinyin[ret].insert(index);
        std::vector<std::string> words = splitWords(index_vec[index].first); 
        std::vector<std::string>::iterator iter = words.begin();
        for (; iter != words.end(); ++iter) {
            index_map[*iter].insert(index);
        }
    }
         
}

Index::Index()
{
    buildIndexFromDiction();
}

Index::~Index()
{
}

