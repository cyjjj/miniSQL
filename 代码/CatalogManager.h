#ifndef _CATALOG_MANAGER_H_
#define _CATALOG_MANAGER_H_

#include <math.h>
#include <stdlib.h>
#include "Basic.h"
#include "BufferManager.h"

#define TABLE_MANAGER_PATH "catalog_file.txt"

using namespace std;


class CatalogManager
{
public:
	void create_table(string input_title, Attribute input_attr, Index input_index, int input_primary);  //input the information of a table into catalog file
	void drop_table(string input_title);                                           //drop a table from catalog file
	bool has_table(string input_title);                                            //search for table with same name
	bool has_attribute(string input_title, string input_attr);                     //search for attribute with same name in a given table
	Attribute get_attribute(string input_title);                                   //return all the attributes of a table
	void create_index(string input_title, string input_attr, string input_index);  //create an index in the catalog file
	void drop_index(string input_title, string input_index);                       //drop an index from catalog file
	string index_to_attr(string input_title, string input_index);                  //find attribute name according to index name
	string index_to_title(string input_index);                                     //find table name according to index name
	void show_table(string input_title);                                           //print the information of table
	int get_block_num(string input_title);                                         //get the size of file
	Index get_index(string input_title);                                           //get index of a table
private:
	string num_to_str(int num, int bit);                      //convert number to string
	int str_to_num(string str);                               //convert string to number
	string get_table_name(string buf, int start, int& rear);  //get table name from buffer
	int get_table_place(string input_title, int& block_id);   //get start place of table
	BufferManager buffer;
};

#endif  //!_CATALOG_MANAGER_H_
