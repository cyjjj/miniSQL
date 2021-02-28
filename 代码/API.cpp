#include "API.h"

//Function: create_table
//Description: create a table
bool API::create_table(string table_name, Attribute attribute, int primary, Index index)
{
	int num = 0;
	Table table(table_name, attribute, num);
	//set index
	table.set_index(primary, "INDEX_FILE_" + attribute.name[primary] + "_" + table_name);
	//write into record and catalog
	record.CreateTable(table);
	catalog.create_table(table_name, attribute, table.index, primary);
	return true;
}

//Function: drop_table
//Description: drop a table
bool API::drop_table(string table_name)
{
	Attribute attribute = catalog.get_attribute(table_name);
	int num = record.get_block_num(table_name + ".table");
	Table table(table_name, attribute, num);
	//set index
	table.index = catalog.get_index(table_name);
	//update record and catalog
	record.DropTable(table);
	catalog.drop_table(table_name);
	return true;
}

//Function: create_index
//Description: create index on table
bool API::create_index(string table_name, string attr_name, string index_name)
{
	string file = "INDEX_FILE_" + attr_name + "_" + table_name;
	Attribute attr = catalog.get_attribute(table_name);
	//check whether attribute is unique or not
	for (int i = 0; i < attr.num; i++) {
		if (attr_name == attr.name[i] && attr.unique[i] == false) {
			throw Exception("Error: Attribute is not unique!");
		}
	}
	//write into catalog file
	catalog.create_index(table_name, attr_name, index_name);
	int num = record.get_block_num(table_name + ".table");
	Table table(table_name, attr, num);
	//set index
	table.index = catalog.get_index(table_name);
	//calculate mask
	int mask = 0;
	for (; mask < attr.num; mask++) {
		if (attr.name[mask] == attr_name)
			break;
	}
	//write into record
	record.createindex(table, mask);
	return true;
}

//Function: drop_index
//Description: drop index of table
bool API::drop_index(string index_name)
{
	//get table name
	string table_name = catalog.index_to_title(index_name);
	//get attribute name
	string attr_name = catalog.index_to_attr(table_name, index_name);
	string file = "INDEX_FILE_" + attr_name + "_" + table_name;
	Attribute attr = catalog.get_attribute(table_name);
	//get block num of table
	int num = record.get_block_num(table_name + ".table");
	Table table(table_name, attr, num);
	//set index
	table.index = catalog.get_index(table_name);
	//calculate mask
	int mask = 0;
	for (; mask < attr.num; mask++) {
		if (attr.name[mask] == attr_name)
			break;
	}
	//update catalog and record
	catalog.drop_index(table_name, index_name);
	record.dropindex(table, mask);
	return true;
}

//Function: select_from
//Description: select record of table
void API::select_from(string table_name, vector<string> target_attr, vector<where> where_vector, vector<string> target_name, int operation)
{
	Attribute attribute = catalog.get_attribute(table_name);
	//get block num
	int num = record.get_block_num(table_name + ".table");
	Table table1(table_name, attribute, num);
	//get index
	table1.index = catalog.get_index(table_name);
	vector<int> attrSelect;
	vector<int> mask;
	vector<int> temp_mask1, temp_mask2;
	vector<where> temp_where1, temp_where2;
	//get number of attributes
	int n = catalog.get_attribute(table_name).num;
	//get attrSelect and mask
	for (int j = 0; j < target_attr.size(); j++) {
		for (int i = 0; i < n; i++) {
			if (target_attr[j] == catalog.get_attribute(table_name).name[i]) {
				attrSelect.push_back(i);
				break;
			}
		}
	}
	for (int j = 0; j < target_name.size(); j++) {
		for (int i = 0; i < n; i++) {
			if (target_name[j] == catalog.get_attribute(table_name).name[i]) {
				mask.push_back(i);
				break;
			}
		}
	}
	//if where condition is less than two
	if (mask.size() <= 1) {
		Table table_output = record.Select(table1, attrSelect, mask, where_vector);
		table_output.show_table();
		int count = table_output.get_tuple().size();
		cout << "Find " << count << " Lines" << endl;
	}
	//if where condition is greater than or equal to two
	else {
		Table table_output;
		temp_mask1.push_back(mask[0]);
		temp_where1.push_back(where_vector[0]);
		Table table3 = record.Select(table1, attrSelect, temp_mask1, temp_where1);
		int times = mask.size();
		for (int i = 1; i < times; i++) {
			record.Select(table3, mask[i], where_vector[i]);
		}
		table3.show_table();
		int count = table3.get_tuple().size();
		cout << "Find " << count << " Lines" << endl;
	}
	return;
}

//Function: insert_into
//Description: insert record into table
void API::insert_into(string table_name, Tuple& tuple)
{
	//check whether table exists or not
	if (!catalog.has_table(table_name))
		throw Exception("Error: Table does not exist!");
	//get attribute
	Attribute attr = catalog.get_attribute(table_name);
	//get block num
	int num = record.get_block_num(table_name + ".table");
	Table table(table_name, attr, num);
	//get index
	table.index = catalog.get_index(table_name);
	//write into record
	record.Insert(table, tuple);
}

//Function: delete_from
//Description: delete record of table
int API::delete_from(string table_name, string attr_name, where wh)
{
	//get attribute
	Attribute attribute = catalog.get_attribute(table_name);
	//get block num
	int num = record.get_block_num(table_name + ".table");
	Table table(table_name, attribute, num);
	//get index
	table.index = catalog.get_index(table_name);
	int ret;
	//get number of attributes
	int n = catalog.get_attribute(table_name).num;
	vector<int> mask;
	vector<where> where;
	//with no where condition
	if (attr_name == "") {
		ret = record.Delete(table, mask, where);
	}
	else {
		//get mask
		for (int i = 0; i < n; i++) {
			if (attr_name == catalog.get_attribute(table_name).name[i]) {
				mask.push_back(i);
				break;
			}
		}
		where.push_back(wh);
		//write into record
		ret = record.Delete(table, mask, where);
	}
	return ret;
}