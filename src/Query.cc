#include "Query.h"
#include "Index.h"
#include "Cache.h"
#include "editdistance.h"
#include "Configure.h"
#include "Diction.h"
#include "ChToPinyin.h"
#include  <queue>
#include <utility>
#include "MySplit.h"
using namespace std;
Query * Query::instance = NULL;
MutexLock Query::lock;

Query * Query::getInstance()
{
    if (instance == NULL) {
        lock.lock();
        if (instance == NULL) {
            instance = new Query();
        }
        lock.unlock();
    }
    return instance;
}

bool searchFromCache(const std::string &keyword, Cache &cache, CacheData &result)
{
    Cache::cache_map_type & cache_map = cache.getCacheDataMap();
    Cache::cache_map_type::iterator iter;
    if ((iter = cache_map.find(keyword)) != cache_map.end()) {
        result = iter->second;
        return true;
    } else {
        return false;
    }
}

std::vector<std::pair<std::string, int>> Query::getSimilarWords(const std::string &keyword, Cache &cache) {
    CacheData result_data;
    std::vector<std::pair<std::string, int>> result_vector;

    if (searchFromCache(keyword, cache, result_data)) {
        return result_data.getDataVec();
    } 

    Index *pIndex = Index::getInstance();
    Index::IndexVecType & index_vec = pIndex->getIndexVec();
    Index::IndexMapType & index_map = pIndex->getIndexMap();
    Index::IndexMapType & pinyin = pIndex->getPinyin();
    std::set<Index::IndexVecType::size_type> allIndexes;
    std::vector<std::string> words = splitWords(keyword);
    std::vector<std::string>::iterator iter = words.begin();
    for (; iter != words.end(); ++iter) {
        std::string tmp = (*iter);
        Index::IndexMapType::iterator iter = index_map.find(tmp);
        if (iter != index_map.end()) {
            allIndexes.insert(iter->second.begin(), iter->second.end());
        }
    }
    Configure *p = Configure::getInstance();
    std::string topK = p->getConfigByName("topk");
    std::string maxdistance = p->getConfigByName("maxdistance");
    int distance = atoi(maxdistance.c_str());
    int topk_int = atoi(topK.c_str());           
            //  {    start
    
    std::string kw = keyword;
    int size_kw = kw.size();
    int one_eq = 0;
    int one_ueq = 0;
    for (int index = 0; index <  size_kw; ++index) {
        if (nBytesCode(kw[index]) == 1)  {
            ++one_eq;
        } else {
            ++one_ueq;
        }
    }
    if (one_ueq == 0) {  // all 1
        std::cout << "all 1 " << std::endl;
        std::unordered_map<std::string, std::set<int>>::iterator iter;
        if ((iter = pinyin.find(kw)) != pinyin.end()) {  // pinyin
            std::priority_queue<Data>  result_queue;
            std::set<int>::iterator iter2 = pinyin[kw].begin();
            for (; iter2 != pinyin[kw].end(); ++iter2) {
                std::string word = (index_vec[*iter2]).first;
                if (word.size() != 0) {
                    Data  data(1, index_vec[*iter2].second, word);
                    result_queue.push(data);
                }                
 
            }           
            result_vector.clear();
            CacheData cache_data;
            std::vector<std::pair<std::string, int>> &cache_data_vec = cache_data.getDataVec();
            while (topk_int-- && (!result_queue.empty())) {
                Data data = result_queue.top();
                result_vector.push_back(make_pair(data.word, data.freq));
                cache_data_vec.push_back(make_pair(data.word, data.freq));
                result_queue.pop();
            }
        
            cache.addQueryResultToCache(keyword, cache_data);
            return result_vector;

        }  else {  // English
            result_vector.clear();
            CacheData cache_data;
            std::vector<std::pair<std::string, int>> &cache_data_vec = cache_data.getDataVec();
            result_vector.push_back(make_pair(kw, 1));
            cache_data_vec.push_back(make_pair(kw, 1));
            //cache.addQueryResultToCache(kw, cache_data); 
            return result_vector;
        }
         
    } else if (one_eq != 0 && one_ueq != 0) {  // some =1  some != 1
        std::cout << "some 1  some not 1" << std::endl;
        std::string  pinyin_s = ChineseToPinyin(kw);
        std::unordered_map<std::string, std::set<int>>::iterator iter;
        if ((iter = pinyin.find(pinyin_s)) != pinyin.end()) {  // pinyin
            std::priority_queue<Data>  result_queue;
            std::set<int>::iterator iter2 = pinyin[pinyin_s].begin();
            for (; iter2 != pinyin[pinyin_s].end(); ++iter2) {
                std::string word = (index_vec[*iter2]).first;
                if (word.size() != 0) {
                    int dis = calcDistance(kw, word);
                    if (dis < distance) {
                        Data data(dis, index_vec[*iter2].second, word);
                        result_queue.push(data);
                    }
                }
            }
            CacheData cache_data;
            result_vector.clear();
            std::vector<std::pair<std::string, int>> &cache_data_vec = cache_data.getDataVec();
            while (topk_int-- && (!result_queue.empty())) {
                Data data = result_queue.top();
                result_vector.push_back(make_pair(data.word, data.freq));
                cache_data_vec.push_back(make_pair(data.word, data.freq));
                result_queue.pop();
            }
            cache.addQueryResultToCache(kw, cache_data);
            return result_vector;
        } else {

            result_vector.clear();
         //   CacheData cache_data;
        //    std::vector<std::pair<std::string, int>> &cache_data_vec = cache_data.getDataVec();
            result_vector.push_back(make_pair(kw, 1));
        //    cache_data_vec.push_back(make_pair(kw, 1));
         //   cache.addQueryResultToCache(kw, cache_data); 
            return result_vector;
        }

    } else if (one_eq == 0) {
        std::cout << " all not 1" << std::endl;
        MySplit *sp = MySplit::getInstance();
        std::vector<std::string> words = sp->wordSplit(kw);
        std::vector<std::string>::iterator iter = words.begin();
        int one_eq2 = 0;
        for (; iter != words.end(); ++iter) {
            if (length(*iter) == 1) {
                ++one_eq2;
            }
        }
        if (one_eq2 >= 1) {
 
            std::string pinyin_s = ChineseToPinyin(kw);
            std::unordered_map<std::string, std::set<int>>::iterator iter;
            if ((iter = pinyin.find(pinyin_s)) != pinyin.end()) {  // pinyin
                std::priority_queue<Data>  result_queue;
                std::set<int>::iterator iter2 = pinyin[pinyin_s].begin();
                for (;iter2 != pinyin[pinyin_s].end(); ++iter2) {
                    std::string word = (index_vec[*iter2]).first;
                    if (word.size() != 0) {
                        int dis = calcDistance(kw, word);
                        if (dis < distance) {
                            Data data(dis, index_vec[*iter2].second, word);
                            result_queue.push(data);
                        }
                    }
                }
                CacheData cache_data;
                result_vector.clear();
                std::vector<std::pair<std::string, int>> &cache_data_vec = cache_data.getDataVec();
                while (topk_int-- && (!result_queue.empty())) {
                    Data data = result_queue.top();
                    result_vector.push_back(make_pair(data.word, data.freq));
                    cache_data_vec.push_back(make_pair(data.word, data.freq));
                    result_queue.pop();
                }
                cache.addQueryResultToCache(kw, cache_data);
                return result_vector;
            } else {
             
                result_vector.clear();
                result_vector.push_back(make_pair(kw, 1));
                return result_vector;
            }  
        } else {
            result_vector.clear();
            CacheData cache_data;
            std::vector<std::pair<std::string, int>> &cache_data_vec = cache_data.getDataVec();
            result_vector.push_back(make_pair(kw, 1));
            cache_data_vec.push_back(make_pair(kw, 1));
            cache.addQueryResultToCache(kw, cache_data); 
            return result_vector;
        } 
    }




    //    }   end   
}

////////////////////////////////////

/* 
    std::priority_queue<Data> result_queue;
    std::set<Index::IndexVecType::size_type>::iterator iter_set = allIndexes.begin();
    for (; iter_set != allIndexes.end(); ++iter_set) {
        std::string word = (index_vec[*iter_set]).first;
        if (word.size() != 0) {
            int dis = calcDistance(keyword, word);
            std::cout << keyword << " " << word << " " << dis << std::endl;
            if (dis < distance) {
                Data data(dis, index_vec[*iter_set].second, word);
                result_queue.push(data);
            } 
        }
    }

    CacheData cache_data;
    std::vector<std::pair<std::string, int>> &cache_data_vec = cache_data.getDataVec();
    while (topk_int-- && (!result_queue.empty())) {
        Data data = result_queue.top();
        result_vector.push_back(make_pair(data.word, data.freq));
        cache_data_vec.push_back(make_pair(data.word, data.freq));
        result_queue.pop();
    }
    cache.addQueryResultToCache(keyword, cache_data);
    return result_vector;
 
}
*/

