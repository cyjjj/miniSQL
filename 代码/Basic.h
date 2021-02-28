#ifndef _BASIC_H_
#define _BASIC_H_

#include "Exception.h"

using namespace std;

typedef enum {
	eq, leq, l, geq, g, neq
} WHERE;

struct where {
	struct Data* d;
	WHERE flag;
};

struct Data {
	int type;          //data type, -1 for int, 0 for float, 1~255 for char
	int data_int;      //data of int type
	float data_float;  //data of float type
	string data_str;   //data of string type
};

struct Attribute {
	int type[32];      //data type, -1 for int, 0 for float, 1~255 for char
	string name[32];   //name of attribute
	int primary_key;   //order number of primary key
	bool unique[32];   //judge whether attribute is unique or not
	bool index[32];    //judge whether attribute has index or not
	int num;           //total number of attributes
};

struct Index {
	int location[10];  //location of index
	string name[10];   //name of index
	int num;           //total number of index
};

class Tuple
{
public:
	Tuple() :deleted(false) {}
	Tuple(const Tuple& tuple);
	void add_data(Data d) { data.push_back(d); }    //add data into tuple
	vector<Data> get_data() const { return data; }  //get data from tuple
	int get_size() const { return (data.size()); }  //get size of tuple
	bool is_deleted() { return deleted; }           //judge whether tuple is deleted or not
	void set_deleted() { deleted = true; }          //set tuple deleted
	void show_tuple();                              //show data of tuple
	Data operator[](unsigned short i);	            //overload symbol []
private:
	vector<Data> data;  //data of tuple
	bool deleted;       //judge whether tuple is deleted or not
};

class Table
{
public:
	Table() {}
	Table(string table_name, Attribute attribute, int num);
	Table(const Table& table);
	int set_index(int i, string index_name);           //set index on table
	int drop_index(string index_name);                 //drop index of table
	string get_title() { return title; }               //get title of table
	Attribute get_attr() { return attr; }              //get attr of table
	vector<Tuple>& get_tuple() { return tuple; }       //get tuple from table
	int get_tuple_size() const;                        //get tuple size of table
	Index get_index() { return index; }                //get index of table
	void show_table();                                 //show data of table
	void add_tuple(Tuple* t) { tuple.push_back(*t); }  //add tuple into table
	void show_index();                                 //show index of table
public:
	int blockNum;         //block num occupied by table
	Attribute attr;		  //attribute of table
	string title;		  //title of table
	vector<Tuple> tuple;  //tuple of table
	Index index;		  //index of table
};

#endif  //!_BASIC_H_