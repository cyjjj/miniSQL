#ifndef _INDEXMANAGER_H_
#define _INDEXMANAGER_H_

#include <iostream>
#include <map>
#include <string>
#include <cstdio>
#include <direct.h>
#include "BPlusTree.h"

//static string indexpath ("D:\\index_minisql\\");

class IndexManager
{
    public:
        map<string,BPTree<int> *> intMap;
        map<string,BPTree<string> *> stringMap;
        map<string,BPTree<float> *> floatMap;

    int getOrder(int type);

    int getKeySize(int type);
   
    void setKey(int type,string key);

    IndexManager();
    ~IndexManager();

    void createIndex(string filePath,int type);  //type: -1 for int, 0 for float, >0 for string
    
    void dropIndex(string filePath,int type);

    void writetodisk(string filePath, int type);
    
    int findIndex(string filePath, int type, map<string,BPTree<int>*>::iterator &ite);
    int findIndex(string filePath, int type, map<string,BPTree<float>*>::iterator &ite);
    int findIndex(string filePath, int type, map<string,BPTree<string>*>::iterator &ite);

    int searchIndex(string filePath,string key,int type);
    int searchIndex(string filePath,int key,int type);
    int searchIndex(string filePath,float key,int type);

    void insertIndex(string filePath,string key,int blockOffset,int type);
    void insertIndex(string filePath,int key,int blockOffset,int type);
    void insertIndex(string filePath,float key,int blockOffset,int type);
    
    void deleteIndex(string filePath,string key,int type);
    void deleteIndex(string filePath,int key,int type);
    void deleteIndex(string filePath,float key,int type);
};

#endif
