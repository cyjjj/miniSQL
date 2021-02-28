#include "Basic.h"

using namespace std;

//Function: Tuple
//Description: copy constructor of Tuple
Tuple::Tuple(const Tuple& tuple)
{
	int i = 0;
	while (i < tuple.data.size())
		data.push_back(tuple.data[i++]);
}

//Function: show_tuple
//Description: show data of tuple
void Tuple::show_tuple()
{
	int i = 0;
	while (i < get_size()) {
		//data of int type
		if (data[i].type == -1)
			cout << data[i].data_int << '\t';
		//data of float type
		else if (data[i].type == 0)
			cout << data[i].data_float << '\t';
		//data of string type
		else if (data[i].type >= 1 && data[i].type <= 255)
			cout << data[i].data_str << '\t';
		i++;
	}
	cout << endl;
}

//Function: operator[]
//Description: overload symbol []
Data Tuple::operator[](unsigned short i)
{
	if (i >= data.size())
		throw out_of_range("out of range!");
	return data[i];
}

//Function: Table
//Description: constructor of Table
Table::Table(string table_name, Attribute attribute, int num)
{
	title = table_name;
	attr = attribute;
	index.num = 0;
	blockNum = num;
}

//Function: Table
//Description: copy constructor of Table
Table::Table(const Table& table)
{
	title = table.title;
	attr = table.attr;
	index = table.index;
	blockNum = table.blockNum;
	int i = 0;
	while (i < table.tuple.size())
		tuple.push_back(table.tuple[i++]);
}

//Function: set_index
//Description: set index on table
//Input: order number of attribute, name of index
//Output: 1 for success, 0 for fail
int Table::set_index(int i, string index_name)
{
	int temp = 0;
	try {
		while (temp < index.num) {
			//check whether index has existed or not
			if (i == index.location[temp])
				throw Exception("Error: Index has already existed!");
			//check whether index name has existed or not
			if (index_name == index.name[temp])
				throw Exception("Error: Index name has already existed!");
			temp++;
		}
	}
	//print exception message
	catch (Exception e) {
		cerr << e.msg << endl;
		return 0;
	}
	//set index
	index.location[index.num] = i;
	index.name[index.num] = index_name;
	//update number of index
	index.num++;
	return 1;
}

//Function: drop_index
//Description: drop index of table
//Input: name of index
//Output: 1 for success, 0 for fail
int Table::drop_index(string index_name)
{
	int temp = 0;
	try {
		//check whether index exists or not
		while (temp < index.num) {
			if (index_name == index.name[temp])
				break;
			temp++;
		}
		if (temp == index.num) {
			throw Exception("Error: Index doesn't exist!");
		}
	}
	//print exception message
	catch (Exception e) {
		cerr << e.msg << endl;
		return 0;
	}
	//check whether number of index is smaller than 10
	if (index.num >= 1 && index.num <= 10) {
		index.location[temp] = index.location[index.num - 1];
		index.name[temp] = index.name[index.num - 1];
	}
	else {
		return 0;
	}
	//update number of index
	index.num--;
	return 1;
}

//Function: get_tuple_size
//Description: get tuple size of table
int Table::get_tuple_size() const
{
	int res = 0;
	for (int i = 0; i < attr.num; i++) {
		switch (attr.type[i]) {
		case -1:
			res += sizeof(int);
			break;
		case 0:
			res += sizeof(float);
			break;
		default:
			res += attr.type[i];
			break;
		}
	}
	return res + 1;
}

//Function: show_table
//Description: show data of table
void Table::show_table()
{
	int i = 0;
	while (i < attr.num)
		cout << attr.name[i++] << '\t';
	cout << endl;
	i = 0;
	while (i < tuple.size())
		tuple[i++].show_tuple();
}

//Function: show_index
//Description: show index of table
void Table::show_index()
{
	for (int i = 0; i < index.num; i++) cout << index.location[i] << " ";
	cout << endl;
}