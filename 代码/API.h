#ifndef _API_H_
#define _API_H_

#include "Basic.h"
#include "RecordManager.h"
#include "BufferManager.h"
#include <string>

class API
{
public:
	bool create_table(string table_name, Attribute attribute, int primary, Index index);  //create a table
	bool drop_table(string table_name);                                                   //drop a table
	bool create_index(string table_name, string attr_name, string index_name);            //create index on table
	bool drop_index(string index_name);                                                   //drop index of table
	void select_from(string table_name, vector<string> target_attr, vector<where> where_vector, vector<string> target_name, int operation);  //select record of table
	void insert_into(string table_name, Tuple& tuple);                                    //insert record into table
	int delete_from(string table_name, string attr_name, where wh);                       //delete record from table
	RecordManager record;
	CatalogManager catalog;
};

#endif  //!_API_H_