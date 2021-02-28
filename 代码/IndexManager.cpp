#include "IndexManager.h"

int IndexManager::getKeySize(int type)
{
    if(type==-1) return sizeof(int);
    else if(type==0) return sizeof(float);
    else if(type>0) return type;
    else return 0;
}

int IndexManager::getOrder(int type)
{
    return BlockSize/(getKeySize(type)+sizeof(int));
}

IndexManager::IndexManager()
{
}

IndexManager::~IndexManager()  //add to map
{
    for(map<string,BPTree<int>*>::iterator iteint = intMap.begin(); iteint != intMap.end(); iteint ++)
    {
        if(iteint->second)
        {
            iteint -> second->writealltodisk(-1);
            delete iteint->second;
        }
    }
    for(map<string,BPTree<string>*>::iterator itestring = stringMap.begin(); itestring != stringMap.end(); itestring ++)
    {
        if(itestring->second)
        {
            itestring ->second->writealltodisk(1);
            delete itestring->second;
        }
    }
    for(map<string,BPTree<float>*>::iterator itefloat = floatMap.begin(); itefloat != floatMap.end(); itefloat ++)
    {
        if(itefloat->second)
        {
            itefloat ->second->writealltodisk(0);
            delete itefloat->second;
        }
    }
}

void IndexManager::writetodisk(string filePath, int type)
{
    if(type==-1)
    {
        map<string,BPTree<int>*>::iterator iteint = intMap.find(filePath);
        if(iteint != intMap.end())
        {
            iteint->second->writealltodisk(-1);
            return;
        }
    }
    else if(type==0)
    {
        map<string,BPTree<float>*>::iterator itefloat = floatMap.find(filePath);
        if(itefloat != floatMap.end())
        {
            itefloat->second->writealltodisk(0);
            return;
        }
    }
    else if(type>0)
    {
        map<string,BPTree<string>*>::iterator itestring = stringMap.find(filePath);
        if(itestring != stringMap.end())
        {
            itestring->second->writealltodisk(1);
            return;
        }
    }
}

void IndexManager::createIndex(string filePath,int type)
{
    string s = /*indexpath +*/ filePath;
    FILE* fp;
    fp = fopen(s.c_str(),"a");
    fclose(fp);
    int keysize = getKeySize(type);
    int order = getOrder(type);
    if(type==-1)
    {
        BPTree<int>* T = new BPTree<int>(s, keysize, order);
        intMap.insert(map<string,BPTree<int> *>::value_type(filePath,T));
    }
    else if(type==0)
    {
        BPTree<float>* T = new BPTree<float>(s, keysize, order);
        floatMap.insert(map<string,BPTree<float> *>::value_type(filePath,T));
    }
    else if(type>0)
    {
        BPTree<string>* T = new BPTree<string>(s, keysize, order);
        stringMap.insert(map<string,BPTree<string> *>::value_type(filePath,T));
    }
    return;
}

void IndexManager::dropIndex(string filePath,int type)  //delete in memory and delete file 
{
    string s = /*indexpath + */filePath;
    if(type==-1)
    {
        map<string,BPTree<int>*>::iterator iteint;
        if(findIndex(filePath,type,iteint))
        {
            delete iteint->second;
            intMap.erase(iteint);
            remove(s.c_str());
        }
    }
    else if(type==0)
    {
        map<string,BPTree<float>*>::iterator itefloat;
        if(findIndex(filePath,type,itefloat))
        {
            delete itefloat->second;
            floatMap.erase(itefloat);
            remove(s.c_str());
        }
    }
    else if(type>0)
    {
        map<string,BPTree<string>*>::iterator itestring;
        if(findIndex(filePath,type,itestring))
        {
            delete itestring->second;
            stringMap.erase(itestring);
            remove(s.c_str());
        }
    }
    return;
}

int IndexManager::findIndex(string filePath, int type, map<string,BPTree<int>*>::iterator &ite)  //read from disk or memory
{
    string s = /*indexpath +*/ filePath;
    ite = intMap.find(filePath);
    if(ite==intMap.end())
    {
        FILE* fp = fopen(s.c_str(),"r");
        if(fp==NULL)
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            fclose(fp);
            return 0;
        }
        else
        {
            fclose(fp);
            createIndex(filePath, type);
            ite = intMap.find(filePath);
            ite->second->readallfromdisk(type);
            return 1;
        }
    }
    else return 1;
}

int IndexManager::findIndex(string filePath, int type, map<string,BPTree<float>*>::iterator &ite)
{
    string s = /*indexpath + */filePath;
    ite = floatMap.find(filePath);
    if(ite==floatMap.end())
    {
        FILE* fp = fopen(s.c_str(),"r");
        if(fp==NULL)
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            fclose(fp);
            return 0;
        }
        else
        {
            fclose(fp);
            createIndex(filePath, type);
            ite = floatMap.find(filePath);
            ite->second->readallfromdisk(type);
            return 1;
        }
    }
    else return 1;
}

int IndexManager::findIndex(string filePath, int type, map<string,BPTree<string>*>::iterator &ite)
{
    string s = /*indexpath + */filePath;
    ite = stringMap.find(filePath);
    if(ite==stringMap.end())
    {
        FILE* fp = fopen(s.c_str(),"r");
        if(fp==NULL)
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            fclose(fp);
            return 0;
        }
        else
        {
            fclose(fp);
            createIndex(filePath, type);
            ite = stringMap.find(filePath);
            ite->second->readallfromdisk(type);
            return 1;
        }
    }
    else return 1;
}

int IndexManager::searchIndex(string filePath,string key,int type)  //search an offset with a key value
{
    map<string,BPTree<string>*>::iterator ite;
    if(findIndex(filePath,type,ite))
    {
        int offset;
        if(!ite->second->search(key, offset))
        {
            return -1;
        }
        return offset;
    }
    return -1;
}

int IndexManager::searchIndex(string filePath,int key,int type)
{
    map<string,BPTree<int>*>::iterator ite;
    if(findIndex(filePath,type,ite))
    {
        int offset;
        if(!ite->second->search(key, offset))
        {
            return -1;
        }
        return offset;
    }
    return -1;
}

int IndexManager::searchIndex(string filePath,float key,int type)
{
    map<string,BPTree<float>*>::iterator ite;
    if(findIndex(filePath,type,ite))
    {
        int offset;
        if(!ite->second->search(key, offset))
        {
            return -1;
        }
        return offset;
    }
    return -1;
}

void IndexManager::insertIndex(string filePath,string key,int Offset,int type)  //insert a key value and offset
{
    map<string,BPTree<string>*>::iterator ite;
    if(findIndex(filePath,type,ite))
    {
        if(!ite->second->insert(key, Offset))
        {
            cout << "Error:in insert index, key " << key <<" already exits" << endl;
            return;
        }
        return;
    }
    return;
}

void IndexManager::insertIndex(string filePath,int key,int Offset,int type)
{
    map<string,BPTree<int>*>::iterator ite;
    if(findIndex(filePath,type,ite))
    {
        if(!ite->second->insert(key, Offset))
        {
            cout << "Error:in insert index, key " << key <<" already exits" << endl;
            return;
        }
        return;
    }
    return;
}

void IndexManager::insertIndex(string filePath,float key,int Offset,int type)
{
    map<string,BPTree<float>*>::iterator ite;
    if(findIndex(filePath,type,ite))
    {
        if(!ite->second->insert(key, Offset))
        {
            cout << "Error:in insert index, key " << key <<" already exits" << endl;
            return;
        }
        return;
    }
    return;
}

void IndexManager::deleteIndex(string filePath,string key,int type)  //delete a key value
{
    map<string,BPTree<string>*>::iterator ite;
    if(findIndex(filePath,type,ite))
    {
        if(!ite->second->Delete(key))
        {
            cout << "Error:in delete index, no key " << key <<" exits" << endl;
            return;
        }
        return;
    }
    return;
}

void IndexManager::deleteIndex(string filePath,int key,int type)
{
    map<string,BPTree<int>*>::iterator ite;
    if(findIndex(filePath,type,ite))
    {
        if(!ite->second->Delete(key))
        {
            cout << "Error:in delete index, no key " << key <<" exits" << endl;
            return;
        }
        return;
    }
    return;
}

void IndexManager::deleteIndex(string filePath,float key,int type)
{
    map<string,BPTree<float>*>::iterator ite;
    if(findIndex(filePath,type,ite))
    {
        if(!ite->second->Delete(key))
        {
            cout << "Error:in delete index, no key " << key <<" exits" << endl;
            return;
        }
        return;
    }
    return;
}