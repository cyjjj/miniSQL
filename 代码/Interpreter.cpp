#include "Interpreter.h"

using namespace std;

//Function: get_query
//Description: get a query and normalize
void Interpreter::get_query()
{
	query.clear();
	string temp;
	//get a query end with ';'
	do {
		getline(cin, temp);
		temp += ' ';
		query += temp;
	} while (query[query.length() - 2] != ';');
	//add '\0' at end of query
	query[query.length() - 2] = '\0';
	//add blank before and after symbol
	int p = 0;
	while (p < query.length()) {
		if (query[p] == ',' || query[p] == '*' || query[p] == '=' || query[p] == '<' || query[p] == '>' || query[p] == '(' || query[p] == ')') {
			query.insert(p++, " ");
			query.insert(++p, " ");
		}
		p++;
	}
	//add blank at end of query
	query.insert(query.length() - 2, " ");
	//delete redundant blank
	int flag = 0;
	string::iterator iter = query.begin();
	while (iter < query.end()) {
		if (*iter == ' ' && !flag) {
			flag = 1;
		}
		else if (*iter == ' ' && flag) {
			query.erase(iter);
			if (iter != query.begin())
				iter--;
		}
		else {
			flag = 0;
		}
		iter++;
	}
	//delete blank at beginning of query
	if (query[0] == ' ')
		query.erase(query.begin());
}

//Function: interpret
//Description: judge keyword of query
int Interpreter::interpret()
{
	size_t start = 0, end = 0;
	string first, second;
	//get first word
	start = query.find_first_not_of(' ', 0);
	end = query.find_first_of(' ', start);
	first = query.substr(start, end - start);
	//get second word
	start = end + 1;
	end = query.find_first_of(' ', start);
	second = query.substr(start, end - start);
	//choose operation according to first word
	try {
		//create a table in minisql
		if (first == "create" && second == "table") {
			start = clock();
			create_table();
			stop = clock();
			duration = ((double)(stop - start)) / CLK_TCK;
			cout << "1 row affected   time:" << duration << " sec" << endl;
			return 1;
		}
		//delete a table from minisql
		else if (first == "drop" && second == "table") {
			start = clock();
			drop_table();
			stop = clock();
			duration = ((double)(stop - start)) / CLK_TCK;
			cout << "1 row affected   time:" << duration << " sec" << endl;
			return 1;
		}
		//create index on table
		else if (first == "create" && second == "index") {
			start = clock();
			create_index();
			stop = clock();
			duration = ((double)(stop - start)) / CLK_TCK;
			cout << "1 row affected   time:" << duration << " sec" << endl;
			return 1;
		}
		//delete index of table
		else if (first == "drop" && second == "index") {
			start = clock();
			drop_index();
			stop = clock();
			duration = ((double)(stop - start)) / CLK_TCK;
			cout << "1 row affected   time:" << duration << " sec" << endl;
			return 1;
		}
		//search for record
		else if (first == "select") {
			start = clock();
			select_from();
			stop = clock();
			duration = ((double)(stop - start)) / CLK_TCK;
			cout << "1 row affected   time:" << duration << " sec" << endl;
			return 1;
		}
		//insert record into table
		else if (first == "insert") {
			start = clock();
			insert_into();
			stop = clock();
			duration = ((double)(stop - start)) / CLK_TCK;
			cout << "1 row affected   time:" << duration << " sec" << endl;
			return 1;
		}
		//delete record of table
		else if (first == "delete") {
			start = clock();
			delete_from();
			stop = clock();
			duration = ((double)(stop - start)) / CLK_TCK;
			cout << "1 row affected   time:" << duration << " sec" << endl;
			return 1;
		}
		//quit minisql
		else if (first == "quit") {
			return 0;
		}
		//execute sql file
		else if (first == "execfile") {
			start = clock();
			int querynum = execfile();
			stop = clock();
			duration = ((double)(stop - start)) / CLK_TCK;
			cout << querynum << " rows affected   time:" << duration << " sec" << endl;
			return 1;
		}
		//if none of above, throw exception
		else {
			throw Exception("Error: Input format error!");
		}
	}
	//input exception message
	catch (Exception e) {
		cerr << e.msg << endl;
	}
}

//Function: create_table
//Description: create a table in minisql
void Interpreter::create_table()
{
	size_t start = 0, end = 0;
	string table_name, temp;
	//get the third word (table name)
	start = query.find_first_not_of(' ', 0);
	end = query.find_first_of(' ', start);
	for (int i = 0; i < 2; i++) {
		start = end + 1;
		end = query.find_first_of(' ', start);
	}
	table_name = query.substr(start, end - start);
	//check for "("
	start = end + 1;
	end = query.find_first_of(' ', start);
	temp = query.substr(start, end - start);
	if (temp != "(")
		throw Exception("Error: Input format error!");
	//set attribute and primary key
	Attribute attribute;
	int primary = -1, count = 0;
	while (true) {
		string first, second;
		//get first word (attribute or "primary")
		start = end + 1;
		end = query.find_first_of(' ', start);
		first = query.substr(start, end - start);
		//get second word (data type or "key")
		start = end + 1;
		end = query.find_first_of(' ', start);
		second = query.substr(start, end - start);
		//if first word is "primary" and second word is "key"
		if (first == "primary" && second == "key") {
			//check for "("
			start = end + 1;
			end = query.find_first_of(' ', start);
			temp = query.substr(start, end - start);
			if (temp != "(")
				throw Exception("Error: Input format error!");
			//get attribute name of primary key
			start = end + 1;
			end = query.find_first_of(' ', start);
			string primary_name = query.substr(start, end - start);
			//go through all the attributes
			int i = 0;
			while (i < attribute.num) {
				//find attribute which has same name with primary key
				if (attribute.name[i] == primary_name) {
					primary = i;
					attribute.primary_key = primary;
					attribute.unique[i] = true;
					break;
				}
				i++;
			}
			//check for ")"
			start = end + 1;
			end = query.find_first_of(' ', start);
			temp = query.substr(start, end - start);
			if (temp != ")")
				throw Exception("Error: Input format error!");
			//check for ending
			start = end + 1;
			end = query.find_first_of(' ', start);
			string tail = query.substr(start, end - start);
			//jump out of loop if reach to end
			if (tail == ")") {
				//if there is other char after ")", throw exception
				if (query[end + 1] != '\0')
					throw Exception("Error: Input format error!");
				break;
			}
			//if read ",", read next word
			else if (tail != ",") {
				throw Exception("Error: Input format error!");
			}
		}
		//else the next word is data type
		else {
			attribute.name[count] = first;
			//get data type
			if (second == "int") {
				attribute.type[count] = -1;
			}
			else if (second == "float") {
				attribute.type[count] = 0;
			}
			else if (second == "char") {
				//check for "("
				start = end + 1;
				end = query.find_first_of(' ', start);
				temp = query.substr(start, end - start);
				if (temp != "(")
					throw Exception("Error: Input format error!");
				//get length of string and convert to int
				start = end + 1;
				end = query.find_first_of(' ', start);
				string num = query.substr(start, end - start);
				attribute.type[count] = atoi(num.c_str()) + 1;
				//check for ")"
				start = end + 1;
				end = query.find_first_of(' ', start);
				temp = query.substr(start, end - start);
				if (temp != ")")
					throw Exception("Error: Input format error!");
			}
			//if next word is not int, float or char, throw exception
			else {
				throw Exception("Error: Input format error!");
			}
			//initialize unique
			attribute.unique[count] = false;
			//check for keyword "unique"
			start = end + 1;
			end = query.find_first_of(' ', start);
			string tail = query.substr(start, end - start);
			if (tail == "unique") {
				attribute.unique[count] = true;
				start = end + 1;
				end = query.find_first_of(' ', start);
			}
			//jump out of loop if reach end
			else if (tail == ")") {
				//if there is other char after ")", throw exception
				if (query[end + 1] != '\0')
					throw Exception("Error: Input format error!");
				break;
			}
			//if read ",", read next word
			else if (tail != ",") {
				throw Exception("Error: Input format error!");
			}
		}
		//initialize index
		attribute.index[count] = false;
		//update number of index
		attribute.num = ++count;
	}
	//set index
	Index index;
	index.num = 0;
	//create table using function in API
	API.create_table(table_name, attribute, primary, index);
	cout << "Success!" << endl;
}

//Function: drop_table
//Description: drop table from minisql
void Interpreter::drop_table()
{
	size_t start = 0, end = 0;
	string table_name;
	//get third word
	start = query.find_first_not_of(' ', 0);
	end = query.find_first_of(' ', start);
	for (int i = 0; i < 2; i++) {
		start = end + 1;
		end = query.find_first_of(' ', start);
	}
	table_name = query.substr(start, end - start);
	//if there is other char after ")", throw exception
	if (query[end + 1] != '\0')
		throw Exception("Error: Input format error!");
	//delete table using function in API
	API.drop_table(table_name);
	cout << "Success!" << endl;
}

//Function: create_index
//Description: create index on table
void Interpreter::create_index()
{
	size_t start = 0, end = 0;
	string index_name, table_name, attr_name;
	string forth, sixth, eighth;
	//get third word
	start = query.find_first_not_of(' ', 0);
	end = query.find_first_of(' ', start);
	for (int i = 0; i < 2; i++) {
		start = end + 1;
		end = query.find_first_of(' ', start);
	}
	index_name = query.substr(start, end - start);
	//get forth word
	start = end + 1;
	end = query.find_first_of(' ', start);
	forth = query.substr(start, end - start);
	//get fifth word
	start = end + 1;
	end = query.find_first_of(' ', start);
	table_name = query.substr(start, end - start);
	//get sixth word
	start = end + 1;
	end = query.find_first_of(' ', start);
	sixth = query.substr(start, end - start);
	//get seventh word
	start = end + 1;
	end = query.find_first_of(' ', start);
	attr_name = query.substr(start, end - start);
	//get eighth word
	start = end + 1;
	end = query.find_first_of(' ', start);
	eighth = query.substr(start, end - start);
	//if forth word is not "on", throw exception
	if (forth != "on")
		throw Exception("Error: Input format error!");
	//if table does not exist, throw exception
	else if (!API.catalog.has_table(table_name))
		throw Exception("Error: Table does not exist!");
	//if sixth word is not "(" or eighth word is not ")", throw exception
	if (sixth != "(" || eighth != ")")
		throw Exception("Error: Input format error!");
	//if there is other word after ")", throw exception
	if (query[end + 1] != '\0')
		throw Exception("Error: Input format error!");
	//create index using function in API
	API.create_index(table_name, attr_name, index_name);
	cout << "Success!" << endl;
}

//Function: drop_index
//Description: delete index of table
void Interpreter::drop_index()
{
	size_t start = 0, end = 0;
	string index_name;
	string forth;
	//get third word
	start = query.find_first_not_of(' ', 0);
	end = query.find_first_of(' ', start);
	for (int i = 0; i < 2; i++) {
		start = end + 1;
		end = query.find_first_of(' ', start);
	}
	index_name = query.substr(start, end - start);
	//if there is other char after end, throw exception
	if (query[end + 1] != '\0')
		throw Exception("Error: Input format error!");
	//delete index using function in API
	API.drop_index(index_name);
	cout << "Success!" << endl;
}

//Function: select_from
//Description: select record of table
void Interpreter::select_from()
{
	size_t start = 0, end = 0;
	string table_name, temp;
	vector<string> attr_name;
	vector<string> target_name;
	vector<where> vector_where;
	string relation;
	string temp_attr;
	string value;
	where temp_where;
	Table output;
	int operation = 0;
	int select_all = 0;
	//get second word
	start = query.find_first_not_of(' ', 0);
	end = query.find_first_of(' ', start);
	start = end + 1;
	end = query.find_first_of(' ', start);
	temp = query.substr(start, end - start);
	//check for "*"
	if (temp == "*") {
		select_all = 1;
	}
	//if second word is not "*", get attribute
	else {
		while (true) {
			//push attribute into vector
			attr_name.push_back(temp);
			//if there is other attribute at end
			if (query[end + 1] == ',') {
				//get name attribute
				start = end + 1;
				end = query.find_first_of(' ', start);
				start = end + 1;
				end = query.find_first_of(' ', start);
				temp = query.substr(start, end - start);
			}
			//if it is last attribute
			else {
				break;
			}
		}
	}
	//get next word
	start = end + 1;
	end = query.find_first_of(' ', start);
	temp = query.substr(start, end - start);
	//if next word is not "from", throw exception
	if (temp != "from")
		throw Exception("Error: Input format error!");
	//get name of table
	start = end + 1;
	end = query.find_first_of(' ', start);
	table_name = query.substr(start, end - start);
	//if table does not exist, throw exception
	if (!API.catalog.has_table(table_name))
		throw Exception("Error: Table does not exist!");
	//get attribute name from catalog file
	Attribute catalog_attr = API.catalog.get_attribute(table_name);
	//push all attribute name into vector if it is select *
	int i = 0;
	if (select_all) {
		while (i < catalog_attr.num) {
			attr_name.push_back(catalog_attr.name[i++]);
		}
	}
	//if it is not select *, get all the attributes
	else {
		while (i < attr_name.size()) {
			if (!API.catalog.has_attribute(table_name, attr_name[i++])) {
				throw Exception("Error: Attribute does not exist!");
			}
		}
	}
	//if there is no "where"
	if (query[end + 1] == '\0') {
		API.select_from(table_name, attr_name, vector_where, target_name, 0);
	}
	//if there is "where"
	else {
		//check for "where"
		start = end + 1;
		end = query.find_first_of(' ', start);
		temp = query.substr(start, end - start);
		if (temp != "where")
			throw Exception("Error: Input format error!");
		while (true) {
			//get name of attribute
			start = end + 1;
			end = query.find_first_of(' ', start);
			temp_attr = query.substr(start, end - start);
			//if attribute does not exist, throw exception
			if (!API.catalog.has_attribute(table_name, temp_attr))
				throw Exception("Error: Attribute does not exist!");
			target_name.push_back(temp_attr);
			//get symbol for relationship
			start = end + 1;
			end = query.find_first_of(' ', start);
			if (query[end + 1] == '=' || query[end + 1] == '>')
				end += 2;
			relation = query.substr(start, end - start);
			//set where.flag according to relation
			if (relation == "=")
				temp_where.flag = eq;
			else if (relation == "< =")
				temp_where.flag = leq;
			else if (relation == "<")
				temp_where.flag = l;
			else if (relation == "> =")
				temp_where.flag = geq;
			else if (relation == ">")
				temp_where.flag = g;
			else if (relation == "! =" || relation == "< >")
				temp_where.flag = neq;
			else
				throw Exception("Error: Input format error!");
			//get data to be compared
			start = end + 1;
			end = query.find_first_of(' ', start);
			value = query.substr(start, end - start);
			temp_where.d = new(struct Data);
			//find data type
			i = 0;
			while (i < catalog_attr.num) {
				if (temp_attr == catalog_attr.name[i]) {
					//data type is same as catalog_attr
					temp_where.d->type = catalog_attr.type[i];
					switch (temp_where.d->type) {
						//int type
					case -1:
						try {
							temp_where.d->data_int = string_to_num<int>(value);
						}
						catch (...) {
							throw Exception("Error: Data type conflict!");
						}
						break;
						//float type
					case 0:
						try {
							temp_where.d->data_float = string_to_num<float>(value);
						}
						catch (...) {
							throw Exception("Error: Data type conflict!");
						}
						break;
						//string type
					default:
						try {
							if ((value[0] == '\'' || value[value.length() - 1] == '\'') && (value[0] == '"' || value[value.length() - 1] == '"'))
								throw Exception("Error: Input format error!");
							temp_where.d->data_str = value.substr(1, value.length() - 2);
						}
						catch (Exception e) {
							if (e.msg == "Error: Input format error!")
								throw Exception("Error: Input format error!");
							else
								throw Exception("Error: Data type conflict!");
						}
						break;
					}
					break;
				}
				i++;
			}
			vector_where.push_back(temp_where);
			if (query[end + 1] == '\0') {
				break;
			}
			else {
				start = end + 1;
				end = query.find_first_of(' ', start);
				temp = query.substr(start, end - start);
				//check for "and" or "or"
				//warning: in this edition, our program cannot support "or" operation
				if (temp == "and")
					operation = 1;
				else if (temp == "or")
					operation = 0;
				else
					throw Exception("Error: Input format error!");
			}
		}
		//select recod using function of API
		API.select_from(table_name, attr_name, vector_where, target_name, operation);
	}
}

//Function: insert_into
//Description: insert data into table
void Interpreter::insert_into()
{
	string table_name;
	string temp;
	Attribute attribute;
	Tuple tuple_in;
	int start = 0, end = 0;
	//check second word for "into"
	start = query.find_first_not_of(' ', 0);
	end = query.find_first_of(' ', start);
	start = end + 1;
	end = query.find_first_of(' ', start);
	temp = query.substr(start, end - start);
	if (temp != "into")
		throw Exception("Error: Input format error!");
	//get table name
	start = end + 1;
	end = query.find_first_of(' ', start);
	table_name = query.substr(start, end - start);
	//check whether table has existed or not
	if (!API.catalog.has_table(table_name))
		throw Exception("Error: Table does not exist!");
	//check forth word for "values"
	start = end + 1;
	end = query.find_first_of(' ', start);
	temp = query.substr(start, end - start);
	if (temp != "values")
		throw Exception("Error: Input format error!");
	//check next word for "("
	start = end + 1;
	end = query.find_first_of(' ', start);
	temp = query.substr(start, end - start);
	if (temp != "(")
		throw Exception("Error: Input format error!");
	//get attribute name of table
	attribute = API.catalog.get_attribute(table_name);
	//calculate number of inserted data
	int count = 0;
	//go through the attribute
	while (query[end + 1] != '\0' && query[end + 1] != ')') {
		if (count > attribute.num)
			throw Exception("Error: Input format error!");
		start = end + 1;
		end = query.find_first_of(' ', start);
		string value = query.substr(start, end - start);
		Data in_data;
		in_data.type = attribute.type[count];
		switch (attribute.type[count]) {
		case -1:
			try {
				in_data.data_int = string_to_num<int>(value);
			}
			catch (...) {
				throw Exception("Error: Data type conflict!");
			}
			break;
		case 0:
			try {
				in_data.data_float = string_to_num<float>(value);
			}
			catch (...) {
				throw Exception("Error: Data type conflict!");
			}
			break;
		default:
			try {
				if ((value[0] != '\'' || value[value.length() - 1] != '\'') && (value[0] != '"' || value[value.length() - 1] != '"'))
					throw Exception("Error: Input format error!");
				if (value.length() - 1 > attribute.type[count])
					throw Exception("Error: Input format error!");
				in_data.data_str = value.substr(1, value.length() - 2);
			}
			catch (Exception e) {
				if (e.msg == "Error: Input format error!")
					throw Exception("Error: Input format error!");
				else
					throw Exception("Error: Data type conflict!");
			}
			break;
		}
		tuple_in.add_data(in_data);
		count++;
		if (query[end + 1] == ',') {
			start += 2;
			end += 2;
		}
		else if (query[end + 1] == ')') {
			break;
		}
		else {
			throw Exception("Error: Input format error!");
		}
	}
	if (query[end + 3] != '\0')
		throw Exception("Error: Input format error!");
	if (count != attribute.num)
		throw Exception("Error: Input format error!");
	API.insert_into(table_name, tuple_in);
	cout << "Success!" << endl;
}

//Function: delete_from
//Description: delete record of table
void Interpreter::delete_from()
{
	where where;
	where.d = new Data;
	int start = 0, end = 0;
	string table_name, attr_name;
	string relation, value, temp;
	//check for second word
	start = query.find_first_not_of(' ', 0);
	end = query.find_first_of(' ', start);
	start = end + 1;
	end = query.find_first_of(' ', start);
	temp = query.substr(start, end - start);
	if (temp != "from")
		throw Exception("Error: Input format error!");
	//get table name
	start = end + 1;
	end = query.find_first_of(' ', start);
	table_name = query.substr(start, end - start);
	//check whether the table has existed or not
	if (!API.catalog.has_table(table_name))
		throw Exception("Error: Table does not exist!");
	//if all record is to be deleted
	if (query[end + 1] == '\0') {
		attr_name = "";
		API.delete_from(table_name, attr_name, where);
		cout << "Success!" << endl;
		return;
	}
	//if there is "where"
	start = end + 1;
	end = query.find_first_of(' ', start);
	temp = query.substr(start, end - start);
	if (temp != "where")
		throw Exception("Error: Input format error!");
	//get attribute name
	start = end + 1;
	end = query.find_first_of(' ', start);
	attr_name = query.substr(start, end - start);
	//check whether attribute exists or not
	if (!API.catalog.has_attribute(table_name, attr_name))
		throw Exception("Error: Attribute does not exist!");
	//get symbol for relationship
	start = end + 1;
	end = query.find_first_of(' ', start);
	if (query[end + 1] == '=' || query[end + 1] == '>')
		end += 2;
	relation = query.substr(start, end - start);
	//get where.flag according to relation
	if (relation == "=")
		where.flag = eq;
	else if (relation == "< =")
		where.flag = leq;
	else if (relation == "<")
		where.flag = l;
	else if (relation == "> =")
		where.flag = geq;
	else if (relation == ">")
		where.flag = g;
	else if (relation == "! =" || relation == "< >")
		where.flag = neq;
	else
		throw Exception("Error: Input format error!");
	//get data to be compared
	start = end + 1;
	end = query.find_first_of(' ', start);
	value = query.substr(start, end - start);
	Attribute attribute = API.catalog.get_attribute(table_name);
	int i = 0;
	//find attribute which has same name as attr_name
	while (i < attribute.num) {
		if (attr_name == attribute.name[i]) {
			where.d->type = attribute.type[i];
			switch (attribute.type[i]) {
			case -1:
				try {
					where.d->data_int = string_to_num<int>(value);
				}
				catch (...) {
					throw Exception("Error: Data type conflict!");
				}
				break;
			case 0:
				try {
					where.d->data_float = string_to_num<float>(value);
				}
				catch (...) {
					throw Exception("Error: Data type conflict!");
				}
				break;
			default:
				try {
					if ((value[0] != '\'' || value[value.length() - 1] != '\'') && (value[0] != '"' || value[value.length() - 1] == '"'))
						throw Exception("Error: Input format error!");
					where.d->data_str = value.substr(1, value.length() - 2);
				}
				catch (Exception e) {
					if (e.msg == "Error: Input format error!")
						throw Exception("Error: Input format error!");
				}
				break;
			}
			break;
		}
		i++;
	}
	API.delete_from(table_name, attr_name, where);
	cout << "Success!" << endl;
}

//Function: execfile
//Description: execute sql file
int Interpreter::execfile()
{
	int querynum = 0;
	int start = 0, end = 0;
	string file_path;
	//get file path
	start = query.find_first_not_of(' ', 0);
	end = query.find_first_of(' ', start);
	start = end + 1;
	end = query.find_first_of(' ', start);
	file_path = query.substr(start, end - start);
	//if there is other char after file name
	if (query[end + 1] != '\0')
		throw Exception("Error: Input format error!");
	string::iterator it;
	fstream f_s(file_path);
	stringstream str_s;
	str_s << f_s.rdbuf();
	string temp = str_s.str();
	//execute again
	start = end = 0;
	do {
		querynum++;
		while (temp[end] != '\n')
			end++;
		query = temp.substr(start, end - start);
		end++;
		start = end;
		//add blank before and after symbol
		int p = 0;
		while (p < query.length()) {
			if (query[p] == ',' || query[p] == '*' || query[p] == '=' || query[p] == '<' || query[p] == '>' || query[p] == '(' || query[p] == ')') {
				query.insert(p++, " ");
				query.insert(++p, " ");
			}
			p++;
		}
		//delete redundant blank
		int flag = 0;
		string::iterator iter = query.begin();
		while (iter < query.end()) {
			if (*iter == ' ' && !flag) {
				flag = 1;
			}
			else if (*iter == ' ' && flag) {
				query.erase(iter);
				if (iter != query.begin())
					iter--;
			}
			else {
				flag = 0;
			}
			iter++;
		}
		//add '\0' at end
		query[query.length() - 1] = '\0';
		//delete blank at beginning
		if (query[0] == ' ')
			query.erase(query.begin());
		interpret();
	} while (temp[end] != '\0' && temp[end] != '\n');
	return querynum;
}

//Function: get_bit
//Description: get length of data with int type
int Interpreter::get_bit(int i)
{
	int ret = 0;
	if (!i) {
		ret = 1;
	}
	else {
		if (i < 0) {
			ret++;
			i *= -1;
		}
		while (i) {
			i /= 10;
			ret++;
		}
	}
	return ret;
}

//Function: get_bit
//Description: get length of data with float type
int Interpreter::get_bit(float f)
{
	int ret = 0;
	if (static_cast<int>(f) == 0) {
		ret = 4;
	}
	else {
		if (f < 0) {
			ret++;
			f *= -1;
		}
		int i = f;
		while (i) {
			ret++;
			i /= 10;
		}
		ret += 3;
	}
	return ret;
}