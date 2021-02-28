#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include "Basic.h"
#include "CatalogManager.h"
#include "API.h"
#include <time.h>

using namespace std;

class Interpreter
{
public:
	void get_query();      //get a query and normalize
	int interpret();       //judge keyword of query
	void create_table();   //create a table in minisql
	void drop_table();     //drop a table from minisql
	void create_index();   //create index on table
	void drop_index();     //drop index of table
	void select_from();    //select data from table
	void insert_into();    //insert data into table
	void delete_from();    //delete data from table
	int execfile();        //execute sql file
private:
	int get_bit(int i);    //get bit of data with int type
	int get_bit(float f);  //get bit of data with float type
	string query;
	API API;
	clock_t start, stop;
    double duration;
};

template <class Type>
Type string_to_num(const string& str)
{
	istringstream iss(str);
	Type number;
	iss >> number;
	return number;
}

#endif  //!_INTERPRETER_H_