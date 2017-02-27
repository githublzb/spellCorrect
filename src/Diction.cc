#include "Diction.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "MySplit.h"
#include "Configure.h"
#include <unistd.h>
#include <string.h>

using namespace std;
Diction *Diction::instance = NULL;
MutexLock Diction::lock;

Diction* Diction::getInstance()
{
    if (instance == NULL) {
        lock.lock();
        if (instance == NULL) {
            instance = new Diction();
        }
        lock.unlock();
    }
    return instance; 
    
}

Diction::Diction()
{
    getSelfDat();
    getExcludeSet();
    createCHDict();
    //createENDict();

}


void Diction::createENDict()
{
    Configure *p = Configure::getInstance();
    std::string dict_en = p->getConfigByName("dict_en");
    std::string home = p->getConfigByName("home");
    traverseDir((home + dict_en).c_str());

}

void Diction::createCHDict()
{
    Configure *p = Configure::getInstance();
    std::string dict_ch = p->getConfigByName("dict_ch");
    std::string home = p->getConfigByName("home");
    traverseDir((home + dict_ch).c_str());

}
void Diction::getSelfDat()
{
    Configure *p = Configure::getInstance();
    std::string self_file = p->getConfigByName("self_file");
    std::string home = p->getConfigByName("home");
    ifstream ifs((home+self_file).c_str());
    if (!ifs.is_open()) {
        throw runtime_error("open file error");
    }
    std::string line;
    while (getline(ifs, line)) {
        std::istringstream iss(line);
        std::string word;
        while( iss >> word) {
            ++dict_map[word];
        }
    }

}

void Diction::traverseDir(const char *dir)
{
//    std::cout << dir << std::endl;
    DIR  *dp = opendir(dir); 
    if (NULL == dp) {
        throw std::runtime_error("cannot open dir");
    }
    struct dirent *entry;
    struct stat statbuf;
    while ((entry = readdir(dp)) != NULL) {
        //stat(entry->d_name, &statbuf);
//        std::cout << entry->d_name << std::endl;
        std::string d(dir);
        std::string file(entry->d_name);
        d = d + '/' + file;
        stat(d.c_str(), &statbuf);
//        std::cout << d << std::endl; 
        if (S_ISDIR(statbuf.st_mode)) {
            if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0) {
                continue;
            }
 //           std::cout << "traver dir :" << d << std::endl;
            traverseDir(d.c_str()); 
        } else {
            std::cout << "readfile : " << d << std::endl;
            readFileToMap(d.c_str());
        }
    }

}

void Diction::readFileToMap(const char *file)
{
    ifstream ifs(file);
    if (!ifs.is_open()) {
        throw runtime_error("open file error");
    }
    std::vector<string> words;
    std::string line;
    MySplit *split = MySplit::getInstance();
    while (getline(ifs, line)) {
        words = split->wordSplit(line);
        for (std::vector<std::string>::iterator iter = words.begin(); iter != words.end(); ++iter) {
            if (!exclude_set.count((*iter))) {
                ++dict_map[(*iter)];
            }
        }
    }

}


void Diction::getExcludeSet()
{
    Configure *p = Configure::getInstance();
    std::string exclude = p->getConfigByName("exclude_set");
    std::string home = p->getConfigByName("home");
    ifstream ifs((home + exclude).c_str());
    if (!ifs.is_open()) {
        throw runtime_error("open file fail");
    }
    std::string line;
    while (getline(ifs, line)) {
        istringstream iss(line);
        std::string token;
        while (iss >> token) {
            exclude_set.insert(token);
        }   
    }
}



