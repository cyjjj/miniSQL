#include "RecordManager.h"
using namespace std;

RecordManager::~RecordManager()
{
	for (int i = 0; i < MaxBlockNum; i++)
		buf_ptr->FlashBack(i);
}

Table RecordManager::Select(Table & tableIn, vector<int> attrSelect, vector<int> mask, vector<where>& w)
{

	if (mask.size() == 0)
		return Select(tableIn, attrSelect);// Select without 'where' restriction

	string stringRow;
	string filename = tableIn.get_title() + ".table";
	string indexfilename;
	int length = tableIn.get_tuple_size() + 1;// Length of a tuple in the file
	const int recordNum = BlockSize / length;// Number of records stored in a block

	//to find whether there is an index on selected key
	int inPos = -1;//index position
	for (int i = 0; i < w.size();i++) {
		if (w[i].flag == eq){	
			for (int j = 0; j < tableIn.index.num;j++) {
				if (tableIn.index.location[j] == mask[i]){			
					Data* ptrData;
					ptrData = w[i].d;
					string tn = tableIn.get_title();
					string an = tableIn.get_attr().name[tableIn.get_index().location[j]];
					int type = tableIn.get_attr().type[tableIn.get_index().location[j]];
					indexfilename = "INDEX_FILE_" + an + "_" + tn + ".txt";
					
					if (type == -1)//int
						inPos = im.searchIndex(indexfilename, ptrData->data_int, type);
					else if (type == 0)//float
						inPos = im.searchIndex(indexfilename, ptrData->data_float, type);
					else inPos = im.searchIndex(indexfilename, ptrData->data_str, type);
					break;	
				}
			}
			if (inPos != -1) break;
		}
	} 
	if (inPos != -1){ //result found
		int blockOffset = inPos / BlockSize;
		int position = inPos % BlockSize;
		int buffernum;
		buffernum = buf_ptr->GiveMeABlock(filename, blockOffset);

		stringRow = buf_ptr->Block[buffernum].getvalues(position, position + length);
		//if (stringRow.c_str()[0] == 2 || stringRow.c_str()[0] == 0) continue;//empty
		int c_pos = 1;// Current position of the pointer
		Tuple *temp_tuple = new Tuple;
		for (int attr_index = 0; attr_index < tableIn.get_attr().num; attr_index++) {
			if (tableIn.get_attr().type[attr_index] == -1) {//int
				int value;
				memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
				c_pos += sizeof(int);
				Data d;
				d.type = -1;
				d.data_int = value;
				temp_tuple->add_data(d);
			}
			else if (tableIn.get_attr().type[attr_index] == 0) {//float
				float value;
				memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
				c_pos += sizeof(float);
				Data d;
				d.type = 0;
				d.data_float = value;
				temp_tuple->add_data(d);
			}
			else { //string
				char value[MAXSTRINGLEN];
				int strLen = tableIn.get_attr().type[attr_index];
				memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
				c_pos += strLen;
				Data d;
				d.type = strLen;
				d.data_str = value;
				temp_tuple->add_data(d);
			}
		}
			tableIn.add_tuple(temp_tuple);

		return SelectProject(tableIn, attrSelect);
	}

	// Select without index
	for (int blockoffset = 0; blockoffset < tableIn.blockNum; blockoffset++) {
		int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockoffset);
		if (bufferNum == -1) {// Not in the block
			bufferNum = buf_ptr->getEmptyBuffer();
			buf_ptr->readBlock(filename, blockoffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * length;
			stringRow = buf_ptr->Block[bufferNum].getvalues(position, position + length);
			if (stringRow.c_str()[0] == 2 || stringRow.c_str()[0] == 0) continue;// The row is empty
			int c_pos = 1;// Current position of the pointer
			Tuple *temp_tuple = new Tuple;
			for (int attr_index = 0; attr_index < tableIn.get_attr().num; attr_index++) {
				if (tableIn.get_attr().type[attr_index] == -1) {//int
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					Data d;
					d.type = -1;
					d.data_int = value;
					temp_tuple->add_data(d);
				}
				else if (tableIn.get_attr().type[attr_index] == 0) {//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					Data d;
					d.type = 0;
					d.data_float = value;
					temp_tuple->add_data(d);
				}
				else { //string
					char value[MAXSTRINGLEN];
					int strLen = tableIn.get_attr().type[attr_index];
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					Data d;
					d.type = strLen;
					d.data_str = value;
					temp_tuple->add_data(d);
				}
			}

			if (isSatisfied(tableIn, *temp_tuple, mask, w)) { // Check if it satisfies the 'where' restriction
				tableIn.add_tuple(temp_tuple);
			}
			else delete temp_tuple;
		}
	}
	return SelectProject(tableIn, attrSelect);
}

void RecordManager::Insert(Table & tableIn, Tuple & singleTuper)
{
	// Check Redundancy using index
	for (int i = 0; i < tableIn.get_attr().num; i++) {
		if (tableIn.get_attr().unique[i] == 1) {// Unique attribution
			int addr = FindWithIndex(tableIn, singleTuper, i);
			if (addr >= 0) { // Already in the table
				throw Exception("Unique Value Redundancy occurs, thus insertion failed");
				return;
			}
		}
	}
	
	for (int i = 0; i < tableIn.get_attr().num; i++) { // Search all 
		if (tableIn.get_attr().unique[i]) {
			vector<where> w;
			vector<int> mask;
			where *uni_w = new where;
			uni_w->d = new Data;
			uni_w->flag = eq;
			(uni_w->d)->type = singleTuper[i].type;
			switch (singleTuper[i].type) {
			case -1:
				(uni_w->d)->data_int = singleTuper[i].data_int;
				break;
			case 0:
				(uni_w->d)->data_float = singleTuper[i].data_float;
				break;
			default:
				(uni_w->d)->data_str = singleTuper[i].data_str;
				break;
			}
			w.push_back(*uni_w);
			mask.push_back(i);

			Table temp_table = Select(tableIn, mask, mask, w);

			if (temp_table.get_tuple().size() != 0) {
				throw Exception("Unique Value Redundancy occurs, thus insertion failed");
			}

			delete uni_w->d;
			delete uni_w;
		}
	}
	
	char *charTuper;
	charTuper = TupleToChar(tableIn, singleTuper);// Converts a tuple into a string for data storage 
	insertPos iPos = buf_ptr->getInsertPosition(tableIn);// Get the position for insertion
	buf_ptr->Block[iPos.bufferNum].content[iPos.position] = 1; // flag as nonempty
	memcpy(&(buf_ptr->Block[iPos.bufferNum].content[iPos.position + 1]), charTuper, tableIn.get_tuple_size());

	int length = tableIn.get_tuple_size() + 1; // Length of a tuple in the file

	// Insert into the index file
	int blockCapacity = BlockSize / length;
	for (int i = 0; i < tableIn.get_index().num; i++) {
		int tuperAddr = buf_ptr->Block[iPos.bufferNum].blockoffset*BlockSize + iPos.position; //��Ԫ�����ļ��еĵ�ַ
		int type = singleTuper[tableIn.get_index().location[i]].type;
		string tn = tableIn.get_title();
		string an = tableIn.get_attr().name[tableIn.get_index().location[i]];   
		if (type == -1)//int
			im.insertIndex("INDEX_FILE_" + an + "_" + tn + ".txt", singleTuper[tableIn.get_index().location[i]].data_int, tuperAddr, singleTuper[tableIn.get_index().location[i]].type);
		else if (type == 0)//float
			im.insertIndex("INDEX_FILE_" + an + "_" + tn + ".txt", singleTuper[tableIn.get_index().location[i]].data_float, tuperAddr, singleTuper[tableIn.get_index().location[i]].type);
		else im.insertIndex("INDEX_FILE_" + an + "_" + tn + ".txt", singleTuper[tableIn.get_index().location[i]].data_str, tuperAddr, singleTuper[tableIn.get_index().location[i]].type);
	}
	buf_ptr->writeBlock(iPos.bufferNum);
	delete[] charTuper;
}

int RecordManager::Delete(Table & tableIn, vector<int> mask, vector<where> w)
{
	//select
	string filename = tableIn.get_title() + ".table";
	string stringRow;
	int count = 0;
	int length = tableIn.get_tuple_size() + 1;
	const int recordNum = BlockSize / length;
	for (int blockoffset = 0; blockoffset < tableIn.blockNum; blockoffset++) {
		int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockoffset);
		if (bufferNum == -1) {// Not in the block 
			bufferNum = buf_ptr->getEmptyBuffer();
			buf_ptr->readBlock(filename, blockoffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * length;
			stringRow = buf_ptr->Block[bufferNum].getvalues(position, position + length);
			if (stringRow.c_str()[0] == 2 || stringRow.c_str()[0] == 0) continue;// The row is empty
			int c_pos = 1;// Current position
			Tuple *temp_tuple = new Tuple;
			for (int attr_index = 0; attr_index < tableIn.get_attr().num; attr_index++) {//���ļ�������һ��tuple
				if (tableIn.get_attr().type[attr_index] == -1) {//int
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					Data d;
					d.type = -1;
					d.data_int = value;
					temp_tuple->add_data(d);
				}
				else if (tableIn.get_attr().type[attr_index] == 0) {//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					Data d;
					d.type = 0;
					d.data_float = value;
					temp_tuple->add_data(d);
				}
				else { //string
					char value[MAXSTRINGLEN];
					int strLen = tableIn.get_attr().type[attr_index];
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					Data d;
					d.type = strLen;
					d.data_str = value;
					temp_tuple->add_data(d);
				}
			}
    
			if (isSatisfied(tableIn, *temp_tuple, mask, w)) { // Check if it satisfies the 'where' restriction
				buf_ptr->Block[bufferNum].content[position] = 2; //flag as deleted(same as empty falg)
				buf_ptr->writeBlock(bufferNum);
				count++;

				// Delete index
				for (int i = 0; i < tableIn.get_index().num; i++) {  
					string tn = tableIn.get_title();
					string an = tableIn.get_attr().name[tableIn.get_index().location[i]];
					int type = tableIn.get_attr().type[tableIn.get_index().location[i]];
					if (type == -1)//int
						im.deleteIndex("INDEX_FILE_" + an + "_" + tn + ".txt", (*temp_tuple)[tableIn.get_index().location[i]].data_int, (*temp_tuple)[tableIn.get_index().location[i]].type);
					else if (type == 0)//float
						im.deleteIndex("INDEX_FILE_" + an + "_" + tn + ".txt", (*temp_tuple)[tableIn.get_index().location[i]].data_float, (*temp_tuple)[tableIn.get_index().location[i]].type);
					else im.deleteIndex("INDEX_FILE_" + an + "_" + tn + ".txt", (*temp_tuple)[tableIn.get_index().location[i]].data_str, (*temp_tuple)[tableIn.get_index().location[i]].type);
				}
			}
		}
	}
	return count;
}

bool RecordManager::DropTable(Table & tableIn)
{
	string filename = tableIn.get_title() + ".table";
	if (remove(filename.c_str()) != 0) {
		throw Exception("Can't delete the file!\n");
	}
	else {
		buf_ptr->setInvalid(filename);
		for (int i = 0; i < tableIn.get_index().num; i++) {  //drop index
			string tn = tableIn.get_title();
			string an = tableIn.get_attr().name[tableIn.get_index().location[i]]; 
			im.dropIndex("INDEX_FILE_" + an + "_" + tn + ".txt", tableIn.get_attr().type[tableIn.get_index().location[i]]);
		}
	}
	return true;
}

bool RecordManager::CreateTable(Table & tableIn)
{
	string filename = tableIn.get_title() + ".table";
	fstream fout(filename.c_str(), ios::out);
	fout.close();
	tableIn.blockNum = 1;
	for (int i = 0; i < tableIn.get_index().num; i++) {  //create index
		string tn = tableIn.get_title();
		string an = tableIn.get_attr().name[tableIn.get_index().location[i]];
		im.createIndex("INDEX_FILE_" + an + "_" + tn + ".txt", tableIn.get_attr().type[tableIn.get_index().location[i]]);
	}
	return true;
}

bool RecordManager::isSatisfied(Table & tableinfor, Tuple & row, vector<int> mask, vector<where> w)
{
	bool res = true;
	for (int i = 0; i < mask.size(); i++) {
		if (w[i].d == NULL) { // Not exsit 'where' restriction
			continue;
		}
		else if (row[mask[i]].type == -1) { //int
			switch (w[i].flag) {
			case eq: //equal
				if (!(row[mask[i]].data_int == ((Data*)w[i].d)->data_int))
					return false;
				break;
			case leq: //less or equal
				if (!(row[mask[i]].data_int <= ((Data*)w[i].d)->data_int))
					return false;
				break;
			case l: //less
				if (!(row[mask[i]].data_int < ((Data*)w[i].d)->data_int))
					return false;
				break;
			case geq: //greater or equal
				if (!(row[mask[i]].data_int >= ((Data*)w[i].d)->data_int))
					return false;
				break;
			case g: //greater
				if (!(row[mask[i]].data_int >((Data*)w[i].d)->data_int))
					return false;
				break;
			case neq: //not equal
				if (!(row[mask[i]].data_int != ((Data*)w[i].d)->data_int))
					return false;
				break;
			default: break;
			}
		}
		else if (row[mask[i]].type == 0) { //float
			switch (w[i].flag) {
			case eq: //equal
				if (!(abs(row[mask[i]].data_float - ((Data*)w[i].d)->data_float) <= MIN_Theta))
					return false;
				break;
			case leq: //less or equal
				if (!(row[mask[i]].data_float <= ((Data*)w[i].d)->data_float))
					return false;
				break;
			case l: //less
				if (!(row[mask[i]].data_float < ((Data*)w[i].d)->data_float))
					return false;
				break;
			case geq: //greater or equal
				if (!(row[mask[i]].data_float >= ((Data*)w[i].d)->data_float))
					return false;
				break;
			case g: //greater
				if (!(row[mask[i]].data_float >((Data*)w[i].d)->data_float))
					return false;
				break;
			case neq: //not equal
				if (!(abs(row[mask[i]].data_float - ((Data*)w[i].d)->data_float) > MIN_Theta))
					return false;
				break;
			default: break;
			}
		}
		else if (row[mask[i]].type > 0) { //string
			switch (w[i].flag) {
			case eq: //equal
				if (!(row[mask[i]].data_str == ((Data*)w[i].d)->data_str))
					return false;
				break;
			case leq: //less or equal
				if (!(row[mask[i]].data_str <= ((Data*)w[i].d)->data_str))
					return false;
				break;
			case l: //less
				if (!(row[mask[i]].data_str < ((Data*)w[i].d)->data_str))
					return false;
				break;
			case geq: //greater or equal
				if (!(row[mask[i]].data_str >= ((Data*)w[i].d)->data_str))
					return false;
				break;
			case g: //greater
				if (!(row[mask[i]].data_str >((Data*)w[i].d)->data_str))
					return false;
				break;
			case neq: //not equal
				if (!(row[mask[i]].data_str != ((Data*)w[i].d)->data_str))
					return false;
				break;
			default: break;
			}
		}
		else {
			cout << "Error in RecordManager in function isSatisified!" << endl;
			system("pause");
		}
	}

	return res;
}

Table RecordManager::Select(Table & tableIn, vector<int> attrSelect)
{
	string stringRow;
	string filename = tableIn.get_title() + ".table";
	string indexfilename;
	int length = tableIn.get_tuple_size() + 1;// Length of a tuple in file
	const int recordNum = BlockSize / length;// Number of records in a block

	for (int blockoffset = 0; blockoffset < tableIn.blockNum; blockoffset++) {
		int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockoffset);
		if (bufferNum == -1) {// Not in the block
			bufferNum = buf_ptr->getEmptyBuffer();
			buf_ptr->readBlock(filename, blockoffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * length;
			stringRow = buf_ptr->Block[bufferNum].getvalues(position, position + length);
			if (stringRow.c_str()[0] == 2 || stringRow.c_str()[0] == 0) continue;// The row is empty
			int c_pos = 1;// Current position
			Tuple *temp_tuple = new Tuple;
			for (int attr_index = 0; attr_index < tableIn.get_attr().num; attr_index++) {
				if (tableIn.get_attr().type[attr_index] == -1) {//int
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					Data d;
					d.type = -1;
					d.data_int = value;
					temp_tuple->add_data(d);
				}
				else if (tableIn.get_attr().type[attr_index] == 0) {//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					Data d;
					d.type = 0;
					d.data_float = value;
					temp_tuple->add_data(d);
				}
				else { //string
					char value[MAXSTRINGLEN];
					int strLen = tableIn.get_attr().type[attr_index];
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					Data d;
					d.type = strLen;
					d.data_str = value;
					temp_tuple->add_data(d);
				}
			}
			tableIn.add_tuple(temp_tuple);
		}
	}
	return SelectProject(tableIn, attrSelect);
}

Table RecordManager::SelectProject(Table & tableIn, vector<int> attrSelect)
{
	Attribute attrOut;
	Tuple *ptr_tuple = NULL;
	attrOut.num = attrSelect.size();
	for (int i = 0; i < attrSelect.size(); i++) {
		attrOut.type[i] = tableIn.get_attr().type[attrSelect[i]];
		attrOut.name[i] = tableIn.get_attr().name[attrSelect[i]];
		attrOut.unique[i] = tableIn.get_attr().unique[attrSelect[i]];
	}
	Table tableOut(tableIn.get_title(), attrOut, tableIn.blockNum);
	int k;
	for (int i = 0; i < tableIn.get_tuple().size(); i++) {// For every tuple in the table
		ptr_tuple = new Tuple;

		for (int j = 0; j < attrSelect.size(); j++) {// For every selected attribute
			k = attrSelect[j];
			Data resadd;
			resadd.type = (((tableIn.get_tuple())[i]).get_data())[k].type;
			if (resadd.type == -1) {//int
				resadd.data_int = (((tableIn.get_tuple())[i]).get_data())[k].data_int;
			}
			else if (resadd.type == 0) {//float
				resadd.data_float = (((tableIn.get_tuple())[i]).get_data())[k].data_float;
			}
			else if (resadd.type > 0) {//string
				resadd.data_str = (((tableIn.get_tuple())[i]).get_data())[k].data_str;
			}
			ptr_tuple->add_data(resadd);
		}
		tableOut.add_tuple(ptr_tuple);
	}
	return tableOut;
}


int RecordManager::FindWithIndex(Table & tableIn, Tuple & row, int mask)
{
	for (int i = 0; i < tableIn.get_index().num; i++) {
		if (tableIn.get_index().location[i] == mask) { // Find index
			Data ptrData;
			ptrData = (row.get_data())[mask];
			int pos;
			string tn = tableIn.get_title();
			string an = tableIn.get_attr().name[mask];
			if (ptrData.type == -1)//int
				pos = im.searchIndex("INDEX_FILE_" + an + "_" + tn + ".txt" , ptrData.data_int, ptrData.type);
			else if (ptrData.type == 0)//float
				pos = im.searchIndex("INDEX_FILE_" + an + "_" + tn + ".txt", ptrData.data_float, ptrData.type);
			else pos = im.searchIndex("INDEX_FILE_" + an + "_" + tn + ".txt", ptrData.data_str, ptrData.type);
			return pos;
		}
	}
	return -1;
}

char * RecordManager::TupleToChar(Table & tableIn, Tuple & singleTuper)
{
	char* ptrRes;
	int pos = 0;// Current position
	ptrRes = new char[(tableIn.get_tuple_size() + 1) * sizeof(char)];
	for (int i = 0; i < tableIn.get_attr().num; i++) {
		if (tableIn.get_attr().type[i] == -1) { //int
			int value = singleTuper[i].data_int;
			memcpy(ptrRes + pos, &value, sizeof(int));
			pos += sizeof(int);
		}
		else if (tableIn.get_attr().type[i] == 0) { //float
			float value = singleTuper[i].data_float;
			memcpy(ptrRes + pos, &value, sizeof(float));
			pos += sizeof(float);
		}
		else { //string
			string value = singleTuper[i].data_str;
			int strLen = tableIn.get_attr().type[i];
			memcpy(ptrRes + pos, value.c_str(), strLen);
			pos += strLen;
		}
	}
	ptrRes[tableIn.get_tuple_size()] = '\0';// End with '\0'
	return ptrRes;
}

Tuple RecordManager::StringToTuper(Table & tableIn, string stringRow)
{
	Tuple temp_tuple;
	if (!stringRow.c_str()[0]) return temp_tuple;// The row is empty
	int c_pos = 1;// Current position
	for (int attr_index = 0; attr_index < tableIn.get_attr().num; attr_index++) {
		if (tableIn.get_attr().type[attr_index] == -1) {//int
			int value;
			memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
			c_pos += sizeof(int);
			Data d;
			d.type = -1;
			d.data_int = value;
			temp_tuple.add_data(d);
		}
		else if (tableIn.get_attr().type[attr_index] == 0) {//float
			float value;
			memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
			c_pos += sizeof(float);
			Data d;
			d.type = 0;
			d.data_float = value;
			temp_tuple.add_data(d);
		}
		else { //string
			char value[MAXSTRINGLEN];
			int strLen = tableIn.get_attr().type[attr_index];
			memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
			c_pos += strLen;
			Data d;
			d.type = strLen;
			d.data_str = value;
			temp_tuple.add_data(d);
		}
	}
	return temp_tuple;
}

void RecordManager::createindex(Table& tableIn, int mask)  //create an index and insert all existed values into the index
{
    string filename = tableIn.get_title() + ".table";
	string stringRow;
	int count = 0;
	int length = tableIn.get_tuple_size() + 1;
	string an = tableIn.get_attr().name[mask];
	string tn = tableIn.get_title();
	im.createIndex("INDEX_FILE_" + an + "_" + tn + ".txt", tableIn.get_attr().type[mask]); // create index
	const int recordNum = BlockSize / length;
	// Select
	for (int blockoffset = 0; blockoffset < tableIn.blockNum; blockoffset++) {
		int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockoffset);
		if (bufferNum == -1) {// Not in the block
			bufferNum = buf_ptr->getEmptyBuffer();
			buf_ptr->readBlock(filename, blockoffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * length;
			stringRow = buf_ptr->Block[bufferNum].getvalues(position, position + length);
			if (stringRow.c_str()[0] == 2 || stringRow.c_str()[0] == 0) continue;// The row is empty
			int c_pos = 1;// Current position
			Tuple *temp_tuple = new Tuple;
			for (int attr_index = 0; attr_index < tableIn.get_attr().num; attr_index++) {
				if (tableIn.get_attr().type[attr_index] == -1) {//int
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					Data d;
					d.type = -1;
					d.data_int = value;
					temp_tuple->add_data(d);
				}
				else if (tableIn.get_attr().type[attr_index] == 0) {//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					Data d;
					d.type = 0;
					d.data_float = value;
					temp_tuple->add_data(d);
				}
				else { //string
					char value[MAXSTRINGLEN];
					int strLen = tableIn.get_attr().type[attr_index];
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					Data d;
					d.type = strLen;
					d.data_str = value;
					temp_tuple->add_data(d);
				}
			}
			// Insert into index file
			int blockCapacity = BlockSize / length;
			int tuperAddr = buf_ptr->Block[bufferNum].blockoffset*BlockSize + position;
            int type = (*temp_tuple)[mask].type;
				if (type == -1)//int
					im.insertIndex("INDEX_FILE_" + an + "_" + tn + ".txt", (*temp_tuple)[mask].data_int, tuperAddr, type);
				else if (type == 0)//float
					im.insertIndex("INDEX_FILE_" + an + "_" + tn + ".txt", (*temp_tuple)[mask].data_float, tuperAddr, type);
				else im.insertIndex("INDEX_FILE_" + an + "_" + tn + ".txt", (*temp_tuple)[mask].data_str, tuperAddr, type);
		}
	}
}	

void RecordManager::dropindex(Table& tableIn, int mask)
{
	string tn = tableIn.get_title();
	string an = tableIn.get_attr().name[mask];
	int type = tableIn.get_attr().type[mask];
	im.dropIndex("INDEX_FILE_" + an + "_" + tn + ".txt", type);
	return;
}

int RecordManager::get_block_num(string input_title) {
	int ret = 0;
	char* p = buf_ptr->Block[buf_ptr->GiveMeABlock(input_title, ret)].content;
	for (; p[0] != '\0'; )
		p = buf_ptr->Block[buf_ptr->GiveMeABlock(input_title, ++ret)].content;
	return ret;
}

void RecordManager::Select(Table & tableIn, int mask, where& w)
{
	int i = 0;
	vector<int> mask1;
	vector<where> w1;
	mask1.push_back(mask);
	w1.push_back(w);
	vector<Tuple>::iterator ite;
	ite = tableIn.tuple.begin();
	while (ite != tableIn.tuple.end())
	{
		if (!isSatisfied(tableIn, *ite, mask1, w1)) tableIn.tuple.erase(ite);
		else ite++;
	}
	return;
}
