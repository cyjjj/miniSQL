#ifndef _BPLUSTREE_H_
#define _BPLUSTREE_H_
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "BufferManager.h"

using namespace std;

//static BufferManager bm;

template <class T>
class BPTreeNode
{
    public:
        BPTreeNode* parent;              //parent
        BPTreeNode* next;                //next sibling
        int order;                       //order of the tree (order/2--order)
        int isleaf;                      //whether the node is a leaf
        int num;                         //number of values in this node
        vector<T> key;                   //key value
        vector<BPTreeNode*> child;       //pointers to children
        vector<int> offset;              //offset value

        BPTreeNode(int Order, int Isleaf);                           //ctor
        ~BPTreeNode();                                               //dtor
        int search(T& value, int& index);                            //search value in a node, with index returned
        int insert_value(T& value, int Offset);                      //insert a value with offset to leaf
        int insert_value(T& value, BPTreeNode* newchild);            //insert a value with child to nonleaf
        int insert_pointer(BPTreeNode* firstchild);                  //insert first child
        int delete_value(T& value);                                  //delete a value
        BPTreeNode* split(T& value, int Offset);                     //split for leaf, with right newnode returned
        BPTreeNode* split(T& value, BPTreeNode* newchild, T& p);     //split for nonleaf, with right newnode returned and p is the deleted first key in newnode
        int merge(BPTreeNode* node);                                 //merge this and node
        int borrowleft(BPTreeNode*& node);                           //borrow from left: node -> this
        int borrowright(BPTreeNode*& node);                          //norrow from right: this <- node
        void print();                                                //just for test
};

template <class T>
class BPTree
{
    public:
        string filename;
        BPTreeNode<T>* root;                                        //root
        BPTreeNode<T>* leafHead; // the head of the leaf node
       // size_t keyCount;
        //size_t level;
        //size_t nodeCount;
        //fileNode* file; // the filenode of this tree
        int keysize; // the size of key
        int order;                                                  //order
        BufferManager bm;
        
        BPTree(string fn, int Keysize, int Order);                                          //ctor
        ~BPTree();                                                  //dtor
        int search(T& value, int& offset);                          //search a value with offset returned
        int search(T& value, int& index, BPTreeNode<T>*& node);     //search a value with index and node returned(used in insert and delete)
        int insert(T& value, int offset);                           //insert
        int Delete(T& value);                                       //delete
        int readfromdisk(int blockoffset, int type);
        void readallfromdisk(int type);
        void writealltodisk(int type);
};

template <class T>
BPTreeNode<T>::BPTreeNode(int Order, int IsLeaf) : parent(NULL), next(NULL), order(Order), isleaf(IsLeaf), num(0)
{
    offset.push_back(0);                                            //offset[0] is useless
    child.push_back(NULL);                                          //child[0] is initialized to be NULL
}

template <class T>
BPTreeNode<T>::~BPTreeNode() {}

template <class T>
int BPTreeNode<T>::search(T& value, int& index)
{
    if(!num || value < key[0])     //less than the smallest key
    {
        index = 0;
        return 0;
    }
    else
    {
        if(value > key[num-1])     //more thah the largest key
        {
            index = num;
            return 0;
        }
        else if(num <= 20)         //sequential search
        {
            index = 0;
            while(index < num && key[index] <= value) index++;
            return (key[index-1] == value);
        }
        else if(num > 20)          //binary search
        {
            int left = 0, right = num-1, pos; 
            while(right > left+1)  //get position
            {
                pos = (right + left) / 2;
                if(key[pos] == value)
                {
                    index = pos + 1;
                    return 1;
                }
                else if(key[pos] < value) left = pos;
                else if(key[pos] > value) right = pos;
            }
            if(value < key[right])        //right = left + 1
            {
                index = right;
                return (key[left] == value);
            }
            else if(key[right] <= value)
            {
                index = right+1;
                return (key[right] == value);
            }
        }
    }
    return 0;
}

template <class T>
int BPTreeNode<T>::insert_value(T& value, int Offset)
{
    if(num == order-1) return 0;
    int index;
    if(search(value, index)) return 0;    //find index
    typename vector<T>::iterator pdT;
    vector<int>::iterator pdint;
    pdT = key.begin() + index;            //find insertion position
    pdint = offset.begin() + index + 1;
    key.insert(pdT, value);               //insert
    offset.insert(pdint, Offset);
    num++;                                //update num
    return 1;
}

template <class T>
int BPTreeNode<T>::insert_value(T& value, BPTreeNode* newchild)
{
    if(num == order-1) return 0;
    int index;
    if(search(value, index)) return 0;       //find index
    if(child[index]->next!=newchild)
    {
        newchild->next = child[index]->next;     //update next, parent
        child[index]->next = newchild;
    }
    newchild->parent = this;
    typename vector<T>::iterator pdT;
    typename vector<BPTreeNode*>::iterator pdptr;
    pdT = key.begin() + index;               //find insertion position
    pdptr = child.begin() + index + 1;
    key.insert(pdT, value);                  //insert
    child.insert(pdptr, newchild);
    num++;                                   //update num
    return 1;
}

template <class T>
int BPTreeNode<T>::insert_pointer(BPTreeNode* firstchild)
{
    child[0] = firstchild;        //insert child[0] 
    child[0]->next = NULL;
    child[0]->parent = this;      //update parent
    return 1;
}

template <class T>
int BPTreeNode<T>::delete_value(T& value)
{
    if(!num) return 0;
    int index;
    if(!search(value,index)) return 0;    //find index, if no value found, then deletion failed
    typename vector<T>::iterator pdT;
    pdT = key.begin() + index - 1;        //find position
    key.erase(pdT);                       //delete key
    if(!isleaf)                           //delete offset for nonleaf
    {
        child[index-1]->next = child[index]->next;
        typename vector<BPTreeNode*>::iterator pdptr;
        pdptr = child.begin() + index;
        child.erase(pdptr);
    }
    else                                  //delete child for leaf
    {
        vector<int>::iterator pdint;
        pdint = offset.begin() + index;
        offset.erase(pdint);
    }
    num--;                                //update num
    return 0;
}

template <class T>
BPTreeNode<T>* BPTreeNode<T>::split(T& value, int Offset)    //split as this->newnode with newnode returned (for leaf)
{
    int index;
    if(search(value, index)) return 0;
    BPTreeNode* newnode = new BPTreeNode(this->order,this->isleaf);   //new memory
    newnode->parent = this->parent;      //set parent and next
    newnode->next = this->next;
    this->next = newnode;
    int i = this->order/2;   //split into 2 parts and the number of first part(this)
    if(index < i) i--;       //if value is inserted into this
    newnode->key.insert(newnode->key.begin(), this->key.begin()+i, this->key.end());    //insert key
    newnode->offset.insert(newnode->offset.end(), this->offset.begin()+i+1, this->offset.end());    //insert offset
    newnode->num = this->order-i-1;    //update num
    this->key.erase(this->key.begin()+i, this->key.end());    //erase key
    this->offset.erase(this->offset.begin()+i+1, this->offset.end());   //erase offset
    this->num = i;    //update num
    if(index < this->order/2) this->insert_value(value, Offset);   //insert value into this or newnode
    else newnode->insert_value(value, Offset);
    return newnode;
}

template <class T>
BPTreeNode<T>* BPTreeNode<T>::split(T& value, BPTreeNode* newchild, T& p)  //split nonleaf into 2 parts and p is the deleted value
{
    int index;
    if(search(value, index)) return 0;
    BPTreeNode* newnode = new BPTreeNode(this->order,this->isleaf);   //the same as leaf split
    newnode->parent = this->parent;
    newnode->next = this->next;
    this->next = newnode;
    int i = this->order/2;
    if(index < i) i--;
    newnode->key.insert(newnode->key.begin(), this->key.begin()+i, this->key.end());
    newnode->child.insert(newnode->child.end(), this->child.begin()+i+1, this->child.end());  //insert child
    newnode->num = this->order-i-1;
    this->key.erase(this->key.begin()+i, this->key.end());
    this->child.erase(this->child.begin()+i+1, this->child.end());   //erase child
    this->num = i;
    if(index < this->order/2) this->insert_value(value, newchild);
    else newnode->insert_value(value, newchild);
    p = *(newnode->key.begin());   //p is the deleted value
    newnode->key.erase(newnode->key.begin());
    newnode->child.erase(newnode->child.begin());
    newnode->num--;
    //this->child[this->num]->next = newnode->child[0];
    for(i=0;i<newnode->num;i++) newnode->child[i]->parent = newnode;   //update inserted children's parent
    return newnode;
}

template <class T>
int BPTreeNode<T>::merge(BPTreeNode<T>* node)  //merge this->node
{
    if(this->num+node->num>this->order-1) return 0;
    int index;
    if(this->isleaf)   //for leaf
    {
        this->key.insert(this->key.end(), node->key.begin(), node->key.end());
        this->offset.insert(this->offset.end(), node->offset.begin()+1, node->offset.end());
        this->num += node->num;
        this->next = node->next;
        index = 0;
        while(this->parent->child[index]!=node) index++;
        this->parent->delete_value(this->parent->key[index-1]);   //delete index in parent
        delete node;
        return 1;
    }
    else   //for nonleaf
    {
        index = 0;
        while(this->parent->child[index]!=node) index++;
        this->key.push_back(this->parent->key[index-1]);   //push parent's index
        this->key.insert(this->key.end(), node->key.begin(), node->key.end());
        this->child.insert(this->child.end(), node->child.begin(), node->child.end());
        int i;
        for(i=0; i<1+node->num; i++) this->child[i+this->num+1]->parent = this;  //update parent
        this->num += (1+node->num);
        this->next = node->next;
        this->parent->delete_value(this->parent->key[index-1]);  //delete index in parent
        delete node;
        return 1;
    }
}

template <class T>
int BPTreeNode<T>::borrowleft(BPTreeNode<T>*& node)  //node -> this
{
    if(this->isleaf)   //for leaf
    {
        this->insert_value(node->key[node->num-1], node->offset[node->num]);
        node->delete_value(node->key[node->num-1]);
        BPTreeNode<T>* p;
        p = this->parent;
        int index;
        p->search(node->key[0], index);
        p->key[index] = this->key[0];  //update index in parent
    }
    else   //for nonleaf
    {
        BPTreeNode<T>* p;
        p = node->parent;
        int index;
        index = 0;
        while(p->child[index]!=this) index++;
        this->key.insert(this->key.begin(), p->key[index-1]);
        this->child.insert(this->child.begin(), *(node->child.end()-1));
        this->num++;
        this->child[0]->parent = this;
        p->key[index-1] = *(node->key.end()-1);  //update index in parent
        node->key.erase(node->key.end()-1);
        node->child.erase(node->child.end()-1);
        node->num--;
    }
    return 1;
}

template <class T>
int BPTreeNode<T>::borrowright(BPTreeNode<T>*& node)  //this <- node (the same as borrowleft)
{
    if(this->isleaf)
    {
        this->insert_value(node->key[0], node->offset[1]);
        node->delete_value(node->key[0]);
        BPTreeNode<T>* p;
        p = this->parent;
        int index;
        p->search(node->key[0], index);
        p->key[index-1] = node->key[0];
    }
    else
    {
        BPTreeNode<T>* p;
        p = node->parent;
        int index;
        index = 0;
        while(p->child[index]!=node) index++;
        this->insert_value(p->key[index-1], node->child[0]);
        //this->child[this->num]->parent = this;
        p->key[index-1] = node->key[0];
        node->key.erase(node->key.begin());
        node->child.erase(node->child.begin());
        //this->child[this->num]->next = node->child[0];
        node->num--;
    }
    return 1;
}

template<class T>
void BPTreeNode<T>::print()   //just for test
{
    int i;
    for(i=0;i<num;i++) cout<<key[i]<<" ";
    cout<<endl;
    if(!isleaf)
    {
        for(i=0;i<=num;i++) child[i]->print();
    }
}


////////////////////////////////////////////
template <class T>
BPTree<T>::BPTree(string fn, int Keysize, int Order) : filename(fn), keysize(Keysize), order(Order)  //ctor
{
    root = new BPTreeNode<T>(Order,1);
    leafHead = root;
}

template <class T>   //dtor
BPTree<T>::~BPTree() {}


template <class T>
int BPTree<T>::search(T& value, int& offset)  //search with offset returned
{
    if(root==NULL) return 0;
    BPTreeNode<T>* cur;
    cur = root;
    int index;
    while(!cur->isleaf)   //search from root to leaf
    {
        cur->search(value, index);
        cur = cur->child[index];
    }
    if(!cur->search(value, index)) return 0;  //find failed
    offset = cur->offset[index];
    return 1;
}

template <class T>
int BPTree<T>::search(T& value, int& index, BPTreeNode<T>*& node)  //search for insert and delete
{
    if(root==NULL)
    {
        index = 0;
        node = root;
        return 0;
    }
    node = root;
    while(!node->isleaf)  //search from root to leaf
    {
        node->search(value, index);
        node = node->child[index];
    }
    return node->search(value, index);
}

template <class T>
int BPTree<T>::insert(T& value, int offset)
{
    int index;
    BPTreeNode<T>* node;
    BPTreeNode<T>* newnode;
    if(search(value, index, node)) return 0;
    if(node->num<node->order-1) return node->insert_value(value, offset);  //if not full, insert directly
    newnode = node->split(value, offset);   //split
    T tmp, p;
    tmp = newnode->key[0];
    while(node->parent!=NULL)  //adjust from leaf to root
    {
        if(node->parent->num < node->parent->order-1) return node->parent->insert_value(tmp, newnode);  //if in this step the parent is not full, insert directly and return
        node = node->parent;
        newnode = node->split(tmp, newnode, p);   //split
        tmp = p;
    }
    root = new BPTreeNode<T>(order,0);  //root need to be updated
    root->insert_pointer(node);
    root->insert_value(tmp,newnode);
    return 1;
}

template <class T>
int BPTree<T>::Delete(T& value)
{
    int index;
    BPTreeNode<T>* node;
    BPTreeNode<T>* p;
    if(!search(value, index, node)) return 0;
    node->delete_value(value);
    while(node->parent!=NULL)  //adjust from leaf to root
    {
        p = node->parent;
        if(node->num >= (order-1)/2) return 1;  //if more than half
        index = 0;
        while(p->child[index]!=node) index++;
        if(index > 0)
        {
            if(p->child[index-1]->num > (order-1)/2)  //if it's left sibling is nore than half
            {
                node->borrowleft(p->child[index-1]);  //then borrow and return
                return 1;
            }
        }
        if(index < p->num)
        {
            if(p->child[index+1]->num > (order-1)/2)  //if it's right sibling is more than half
            {
                node->borrowright(p->child[index+1]);  //then borrow and return
                return 1;
            }
        }
        if(index == p->num)   //if both left and right are not enough, then merge
        {
            p->child[index-1]->merge(node);  //merge with left sibling
        }
        else
        {
            node->merge(p->child[index+1]);  //merge with right sibling
        }
        node = p;
    }
    if(!node->num && root != leafHead)  //if no key is in the root, then update root
    {
        root = node->child[0];
		node->child[0]->parent = NULL;
    }
    return 1;
}

template <class T>
void BPTree<T>::writealltodisk(int type)
{
    int blockoffset = 0;
    BPTreeNode<T>* cur;
    cur = leafHead;
    char *c;
    int j;
    int i, blocknum;
    while(cur!=NULL)
    {
        blocknum = bm.GiveMeABlock(filename, blockoffset);
        memset(bm.Block[blocknum].content,0,BlockSize);
        bm.Block[blocknum].filename = filename;
        bm.Block[blocknum].valid = 1;
        c = bm.Block[blocknum].content;
        j = 0;
        for(i=0;i<cur->num;i++)
        {
            char *k;
            if(type>0)
            {
                void *a = &(cur->key[i]);
                string *s = (string *)a;
                k = (char*)s->c_str();
            }
            else
            {
                k = (char *)&(cur->key[i]);
            }
            char *o = (char *)&(cur->offset[i+1]);
            //strncpy_s(c,sizeof(c),k,keysize);
            for(int x = 0;x<keysize;x++) c[x] = k[x];
            c += keysize;
            //strncpy_s(c,sizeof(c),o,sizeof(int));
            for(int x = 0;x<sizeof(int);x++) c[x] = o[x];
            c += sizeof(int);
            j += (keysize+sizeof(int));
        }
        j += (keysize+sizeof(int));
        if(j<=BlockSize)
        {
            c += keysize;
            j = -1;
            char *end = (char*)&j;
            //strncpy_s(c,sizeof(c),end,sizeof(int));
            for(int x = 0;x<sizeof(int);x++) c[x] = end[x];
        }
        cur = cur->next;
        bm.writeBlock(blocknum);
        bm.FlashBack(blocknum);
        blockoffset++;
    }
    blocknum = bm.GiveMeABlock(filename, blockoffset);
    memset(bm.Block[blocknum].content,0,BlockSize);
    c = bm.Block[blocknum].content + keysize;
    j = -1;
    char *end = (char*)&j;
    //strncpy_s(c,sizeof(c),end,sizeof(int));
    for(int x = 0;x<sizeof(int);x++) c[x] = end[x];
    bm.Block[blocknum].filename = filename;
    bm.Block[blocknum].valid = 1;
    bm.writeBlock(blocknum);
    bm.FlashBack(blocknum);
    return;
}

template <class T>
int BPTree<T>::readfromdisk(int blockoffset, int type)
{
    int blocknum;
    blocknum = bm.GiveMeABlock(filename, blockoffset);
    bm.readBlock(filename,blockoffset,blocknum);
    char* keystart;
    char* offsetstart;
    int i = 0;
    T k;
    int o;
    while(i+keysize+sizeof(int) <= BlockSize)
    {
        keystart = bm.Block[blocknum].content+i;
        offsetstart = keystart+keysize;
        o = *(int*)offsetstart;
        if(o<0)break;
        if(type>0)
        {
            string s(keystart);
            void *a = &s;
            T* b = (T*)a;
            insert(*b,o);
        }
        else
        {
           // if(*keystart==)
            k = *(T*)keystart;
            insert(k,o);
        } 
        i += (keysize+sizeof(int));
    }
    if(!i) return 0;
    else return 1;
}

template <class T>
void BPTree<T>::readallfromdisk(int type)
{
    int i=0;
    while(readfromdisk(i++,type));
}

#endif

