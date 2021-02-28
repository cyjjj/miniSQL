#include "CatalogManager.h"

using namespace std;

//Function: create_table
//Description: input the information of a table into catalog file
//Calls: has_table, num_to_str, get_block_num, getIfIsInBuffer, GiveMeABlock, writeBlock
//Input: table name, attribute, index, primary key number
//Output: void
//Exception: table_already_exist
void CatalogManager::create_table(string input_title, Attribute input_attr, Index input_index, int input_primary) {
	int i, block_num, current = 0;
	string str, len;
	//throw an exception if the table already exists
	if (has_table(input_title))
		throw Exception("Error: Table has already existed!");
	//make sure that the primary key is unique
	input_attr.unique[input_primary] = true;
	//write down the character length of each record
	str = "0000";
	//write down the table name
	str = str + " " + input_title;
	//write down the number of attributes
	str = str + " " + num_to_str(input_attr.num, 2);
	//write down the information of each attribute
	for (i = 0; i < input_attr.num; i++) {
		str = str + " " + num_to_str(input_attr.type[i], 3);
		str = str + " " + input_attr.name[i];
		if (input_attr.unique[i] == true)
			str = str + " " + "1";
		else
			str = str + " " + "0";
	}
	//write down the information of primary key
	str = str + " " + num_to_str(input_primary, 2);
	//use " ;" to mark the begin of index
	str = str + " " + ";";
	//write down the number of index
	str = str + num_to_str(input_index.num, 2);
	//write down the information of index
	for (i = 0; i < input_index.num; i++) {
		str = str + " " + num_to_str(input_index.location[i], 2);
		str = str + " " + input_index.name[i];
	}
	//each block ends with a "$"
	str = str + "\n" + "$";
	//update the character length of record
	len = num_to_str(static_cast<int>(str.length()) - 1, 4);
	str = len + str.substr(4, str.length() - 4);
	//calculate the number of blocks
	block_num = get_block_num(TABLE_MANAGER_PATH) / BlockSize;
	if (!(block_num > 0))
		block_num = 1;
	//go through all the blocks
	while (current < block_num) {
		int buf_id = buffer.getIfIsInBuffer(TABLE_MANAGER_PATH, current);
		char* buf = buffer.Block[buffer.GiveMeABlock(TABLE_MANAGER_PATH, current)].content;
		//find the valid length of the block
		int valid_len = 0;
		while (valid_len < BlockSize && !(buf[valid_len] == '\0' || buf[valid_len] == '$'))
			valid_len++;
		//make sure that the length of that block is less than BlockSize
		if (valid_len + static_cast<int>(str.length()) < BlockSize) {
			//remove the ending flag "$"
			if (buf[valid_len] == '$')
				buf[valid_len] = '\0';
			else if (valid_len >= 1 && buf[valid_len - 1] == '$')
				buf[valid_len - 1] = '\0';
			//connect the two strings
			strcat(buf, str.c_str());
			//mark that the block has been modified
			buffer.writeBlock(buf_id);
			return;
		}
		current++;
	}
	//if the length of block is larger than BlockSize
	//create a new block to store the information
	int buf_id = buffer.getIfIsInBuffer(TABLE_MANAGER_PATH, BlockSize);
	char* buf = buffer.Block[buffer.GiveMeABlock(TABLE_MANAGER_PATH, BlockSize)].content;
	strcat(buf, str.c_str());
	//mark that the block has been modified
	buffer.writeBlock(buf_id);
}

//Function: drop_table
//Description: drop a table from catalog file
//Calls: has_table, get_table_place, getIfIsInBuffer, GiveMeABlock, str_to_num, writeBlock
//Input: table name
//Output: void
//Exception: table_not_exist
void CatalogManager::drop_table(string input_title) {
	int i = 0, current = 0;
	//throw exception if the table doesn't exist
	if(!has_table(input_title))
		throw Exception("Error: Table does not exist!");
	//find the block id and the place
	int block_id;
	int start_place = get_table_place(input_title, block_id);
	//find the information of corresponding block
	int buf_id = buffer.getIfIsInBuffer(TABLE_MANAGER_PATH, block_id);
	char* buf = buffer.Block[buffer.GiveMeABlock(TABLE_MANAGER_PATH, block_id)].content;
	string buf_check(buf);
	//find the end place of the block
	int end_place = start_place + str_to_num(buf_check.substr(start_place, 4));
	//delete the block
	for (; buf[i] != '$'; i++) {
		if (!(i >= start_place && i < end_place))
			buf[current++] = buf[i];
	}
	buf[current++] = '$';
	buf[current] = '\0';
	//mark that the block has been modified
	buffer.writeBlock(buf_id);
}

//Function: has_table
//Description: search for table with same name
//Calls: get_block_num, GiveMeABlock, get_table_name, str_to_num
//Input: table name
//Output: bool
bool CatalogManager::has_table(string input_title) {
	int current = 0;
	//calculate the number of blocks
	int block_num = get_block_num(TABLE_MANAGER_PATH) / BlockSize;
	block_num = (block_num > 0) ? block_num : 1;
	//go through the block
	while (current < block_num) {
		int start = 0, end = 0;
		char* buf = buffer.Block[buffer.GiveMeABlock(TABLE_MANAGER_PATH, current)].content;
		string buf_check(buf);
		//check the next block if start with '$'
		if (buf_check[0] == '$') {
			current++;
			break;
		}
		for (; buf_check[start] != '$'; ) {
			//check if the table name is same as input name
			if (get_table_name(buf, start, end) == input_title) {
				return true;
			}
			//if the file is empty, find the next table
			else {
				start += str_to_num(buf_check.substr(start, 4));
				if (start == 0) {
					current++;
					break;
				}
			}
		}
		current++;
	}
	return false;
}

//Function: has_attribute
//Description: search for attribute with same name in a given table
//Calls: has_table, get_attribute
//Input: table name, attribute name
//Output: bool
//Exception: table_not_exist
bool CatalogManager::has_attribute(string input_title, string input_attr) {
	int i = 0;
	//throw exception if the table doesn't exist
	if (!has_table(input_title))
		throw Exception("Error: Table does not exist!");
	//find all the attributes of the table
	Attribute attr = get_attribute(input_title);
	//go through all the attribute names
	while (i < attr.num) {
		if (input_attr == attr.name[i])
			return true;
		i++;
	}
	return false;
}

//Function: get_attribute
//Description: return all the attributes of a table
//Calls: has_table, get_table_place, GiveMeABlock, get_table_name, str_to_num, get_index
//Input: table name
//Output: class Attribute
//Exception: table_not_exist
Attribute CatalogManager::get_attribute(string input_title) {
	int i = 0;
	Attribute attr;
	//throw exception if the table doesn't exist
	if (!has_table(input_title))
		throw Exception("Error: Table does not exist!");
	//find the block id and the place
	int block_id;
	int start_place = get_table_place(input_title, block_id);
	//find the information of corresponding block
	char* buf = buffer.Block[buffer.GiveMeABlock(TABLE_MANAGER_PATH, block_id)].content;
	string buf_check(buf);
	//find the end place of the block
	int end_place = 0;
	string title = get_table_name(buf_check, start_place, end_place);
	start_place = end_place + 1;
	//get the number of attributes
	string num = buf_check.substr(start_place, 2);
	attr.num = str_to_num(num);
	start_place += 3;
	while (i < attr.num) {
		//data type is int
		if (buf_check[start_place] == '-') {
			attr.type[i] = -1;
			start_place += 5;
		}
		//data type is float
		else if (!str_to_num(buf_check.substr(start_place, 3))) {
			attr.type[i] = 0;
			start_place += 4;
		}
		//data type is char
		else {
			attr.type[i] = str_to_num(buf_check.substr(start_place, 3));
			start_place += 4;
		}
		//get attribute names
		for (; buf_check[start_place] != ' '; start_place++)
			attr.name[i] += buf_check[start_place];
		start_place++;
		//if the attribute is unique or not
		if (buf_check[start_place] == '1')
			attr.unique[i] = true;
		else
			attr.unique[i] = false;
		start_place += 2;
		i++;
	}
	//get the information of primary key
	if (buf_check[start_place] == '-')
		attr.primary_key = -1;
	else
		attr.primary_key = str_to_num(buf_check.substr(start_place, 2));
	//set the information of index
	Index index = get_index(input_title);
	for (i = 0; i < 32; i++)
		attr.index[i] = false;
	for (i = 0; i < index.num; i++)
		attr.index[index.location[i]] = true;
	return attr;
}

//Function: get_attribute
//Description: return all the attributes of a table
//Calls: has_table, get_table_place, GiveMeABlock, get_table_name, str_to_num, get_index
//Input: table name
//Output: class Attribute
//Exception: table_not_exist
Index CatalogManager::get_index(string input_title) {
	Index index;
	int block_id = 0, j = 0;
	int start = get_table_place(input_title, block_id);
	char* buf = buffer.Block[buffer.GiveMeABlock(TABLE_MANAGER_PATH, block_id)].content;
	string buf_check(buf);
	for (; buf_check[start] != ';'; )
		start++;
	start++;
	index.num = str_to_num(buf_check.substr(start, 2));
	while (j < index.num) {
		start += 3;
		index.location[j] = str_to_num(buf_check.substr(start, 2));
		start += 3;
		for (; buf_check[start] != ' ' && buf_check[start] != '$' && buf_check[start] != '\n'; )
			index.name[j] += buf_check[start++];
		start -= 2;
		j++;
	}
	return index;
}

//Function: create_index
//Description: create an index in the catalog file
//Calls: has_table, has_attribute, get_index, get_attribute, drop_table, create_table
//Input: table name, attribute name, index name
//Output: void
//Exception: table_not_exist, attribute_not_exist, index_already_full, index_already_exist
void CatalogManager::create_index(string input_title, string input_attr, string input_index) {
	int i = 0;
	//throw exception if the table doesn't exist
	if (!has_table(input_title))
		throw Exception("Error: Table does not exist!");
	//throw exception if the attribute doesn't exist
	if (!has_attribute(input_title, input_attr))
		throw Exception("Error: Attribute does not exist!");
	//get the information about index
	Index index = get_index(input_title);
	//make sure that the number of index is smaller than 10
	if (index.num >= 10)
		throw Exception("Error: Index is full!");
	//get attribute
	Attribute attr = get_attribute(input_title);
	//go through all the index
	while (i < index.num) {
		//if the index name already exists
		if (index.name[i] == input_index || attr.name[index.location[i]] == input_index)
			throw Exception("Error: Index has already existed!");
		i++;
	}
	//add a new index
	index.name[index.num] = input_index;
	i = 0;
	while (i < attr.num) {
		if (input_attr == attr.name[i]) {
			index.location[index.num] = i;
			break;
		}
		i++;
	}
	//update the total number
	index.num++;
	//update the old table
	drop_table(input_title);
	create_table(input_title, attr, index, attr.primary_key);
}

//Function: drop_index
//Description: drop an index from catalog file
//Calls: has_table, get_attribute, drop_table, create_table
//Input: table name, index name
//Output: void
//Exception: table_not_exist, index_not_exist
void CatalogManager::drop_index(string input_title, string input_index) {
	int i = 0, flag = -1;
	//throw exception if the table doesn't exist
	if (!has_table(input_title))
		throw Exception("Error: Table does not exist!");
	//get the index of table
	Index index = get_index(input_title);
	//get the attribute of table
	Attribute attr = get_attribute(input_title);
	//go through all the index
	while (i < index.num) {
		if (index.name[i] == input_index)
			flag = i;
		i++;
	}
	//if the index cannot be found
	if (flag == -1)
		throw Exception("Error: Index does not exist!");
	//delete the index
	index.name[flag] = index.name[index.num - 1];
	index.location[flag] = index.location[index.num - 1];
	//update the total number;
	index.num--;
	//update the old table
	drop_table(input_title);
	create_table(input_title, attr, index, attr.primary_key);
}

//Function: index_to_attr
//Description: find attribute name according to index name
//Calls: has_table, get_index, get_attribute
//Input: table name, index name
//Output: attribute name
//Exception: table_not_exist, index_not_exist
string CatalogManager::index_to_attr(string input_title, string input_index) {
	int i = 0, flag = -1;
	string attr_name;
	//throw exception if the table doesn't exist
	if (!has_table(input_title))
		throw Exception("Error: Table does not exist!");
	//get the index of table
	Index index = get_index(input_title);
	while (i < index.num) {
		if (index.name[i] == input_index) {
			flag = i;
			break;
		}
		i++;
	}
	//if the index cannot be found
	if (flag == -1)
		throw Exception("Error: Index does not exist!");
	//get attribute and find its name
	Attribute attr = get_attribute(input_title);
	attr_name = attr.name[index.location[flag]];
	return attr_name;
}

//Function: index_to_title
//Description: convert index to table name
//Input: index name
//Output: table name
string CatalogManager::index_to_title(string input_index) {
	string table_name, index_name;
	int block_num = get_block_num(TABLE_MANAGER_PATH);
	block_num = (block_num >= 1) ? block_num : 1;
	for (int block_id = 0; block_id < block_num; block_id++) {
		char* buf = buffer.Block[buffer.GiveMeABlock(TABLE_MANAGER_PATH, block_id)].content;
		string buf_check(buf);
		string str = "";
		int start = 0, rear = 0;
		while (buf_check[start] != '$') {
			if (buf_check[0] == '$') {
				block_id++;
				break;
			}
			table_name = get_table_name(buf, start, rear);
			while (buf_check[start] != ';')
				start++;
			start++;
			rear = buf_check.find_first_of(' ', start);
			string num_str = buf_check.substr(start, rear - start);
			int num = str_to_num(num_str);
			for (int i = 0; i < num; i++) {
				start = rear + 4;
				if (i == num - 1)
					rear = buf_check.find_first_of('\n', start);
				else
					rear = buf_check.find_first_of(' ', start);
				index_name = buf_check.substr(start, rear - start);
				if (input_index == index_name) {
					return table_name;
				}
				else if (i == num - 1) {
					start = rear + 1;
					break;
				}
			}
		}
	}
	table_name = "Error: Index does not exist!";
	return table_name;
}

//Function: show_table
//Description: print the information of table
//Calls: has_table, get_attribute, get_index, num_to_str
//Input: table name
//Output: void
//Exception: table_not_exist
void CatalogManager::show_table(string input_title) {
	//throw exception if the table doesn't exist
	if (!has_table(input_title))
		throw Exception("Error: Table does not exist!");
	//print the table name
	cout << "Table name: " << input_title << endl;
	//get attribute and index
	Attribute attr = get_attribute(input_title);
	Index index = get_index(input_title);
	//find the longest index name
	int i = 0, index_longest = 0;
	while (i < attr.num) {
		if (static_cast<int>(attr.name[i].length()) > index_longest)
			index_longest = static_cast<int>(attr.name[i].length());
		i++;
	}
	//print the attributes
	string type;
	cout << "Attribute:" << endl << "Num|Name" << setw(index_longest + 2) << "|Type" << type << setw(6) << "|Unique|Primary Key" << endl;
	i = 0;
	while (i < index_longest + 35) {
		cout << "-";
		i++;
	}
	cout << endl;
	i = 0;
	while (i < attr.num) {
		switch (attr.type[i]) {
		case -1:
			type = "int";
			break;
		case 0:
			type = "float";
			break;
		default:
			type = "char(" + num_to_str(attr.type[i] - 1, 3) + ")";
			break;
		}
		cout << i << setw(3 - i / 10) << "|" << attr.name[i] << setw(index_longest - static_cast<int>(attr.name[i].length()) + 2) << "|" << type << setw(10 - static_cast<int>(type.length())) << "|";
		if (attr.unique[i])
			cout << "unique|";
		else
			cout << setw(7) << "|";
		if (attr.primary_key == i)
			cout << "primary key";
		cout << endl;
		i++;
	}
	i = 0;
	while (i < index_longest + 35)
		cout << "-";
	cout << endl;
	//print the index
	cout << "Index: " << endl << "Num|Location|Name" << endl;
	i = 0, index_longest = 0;
	while (i < index.num) {
		if (static_cast<int>(index.name[i].length()) > index_longest)
			index_longest = static_cast<int>(index.name[i].length());
		i++;
	}
	i = 0;
	while (i < (18 > (index_longest + 14) ? 18 : (index_longest))) {
		cout << "-";
		i++;
	}
	cout << endl;
	i = 0;
	while (i < index.num) {
		cout << i << setw(3 - i / 10) << "|" << index.location[i] << setw(8 - index.location[i] / 10) << "|" << index.name[i] << endl;
		i++;
	}
	i = 0;
	while (i < (18 > (index_longest + 14) ? 18 : (index_longest + 14))) {
		cout << "-";
		i++;
	}
	cout << endl << endl;
}

//Function: get_block_num
//Description: get the size of file
//Calls: GiveMeABlock
//Input: table name
//Output: size of table
int CatalogManager::get_block_num(string input_title) {
	int ret = 0;
	char* p = buffer.Block[buffer.GiveMeABlock(input_title, ret)].content;
	for (; p[0] != '\0'; )
		p = buffer.Block[buffer.GiveMeABlock(input_title, ++ret)].content;
	return ret;
}

//Function: num_to_str
//Description: convert number to string
string CatalogManager::num_to_str(int num, int bit) {
	int i = 0;
	string str = "";
	if (num < 0) {
		num *= -1;
		str += "-";
	}
	int divider = pow(10, bit - 1);
	while (i < bit) {
		str += (num / divider % 10 + '0');
		divider /= 10;
		i++;
	}
	return str;
}

//Function: str_to_num
//Description: convert string to number
int CatalogManager::str_to_num(string str) {
	return atoi(str.c_str());
}

//Function: get_table_name
//Description: get table name from buffer
string CatalogManager::get_table_name(string buf, int start, int& rear) {
	string str = "";
	rear = 0;
	if (buf == "")
		return buf;
	for (; buf[start + rear + 5] != ' '; )
		rear++;
	str = buf.substr(start + 5, rear);
	rear += start + 5;
	return str;
}

//Function: get_table_place
//Description: get start place of table
int CatalogManager::get_table_place(string input_title, int& block_id) {
	int block_num = get_block_num(TABLE_MANAGER_PATH);
	block_num = (block_num >= 1) ? block_num : 1;
	block_id = 0;
	while (block_id < block_num) {
		char* buf = buffer.Block[buffer.GiveMeABlock(TABLE_MANAGER_PATH, block_id)].content;
		string buf_check(buf);
		string str = "";
		int start = 0, rear = 0;
		for (; buf_check[start] != '$'; ) {
			if (buf_check[0] == '$') {
				block_id++;
				break;
			}
			if (get_table_name(buf, start, rear) == input_title) {
				return start;
			}
			else {
				start += str_to_num(buf_check.substr(start, 4));
				if (start == 0) {
					block_id++;
					break;
				}
			}
		}
		block_id++;
	}
	return -1;
}