#pragma once
#ifndef _RECORDMANAGER_H
#define	_RECORDMANAGER_H
#include "Basic.h"
#include "Exception.h"
#include "BufferManager.h"
#include "CatalogManager.h"
#include "IndexManager.h"

#define MAXSTRINGLEN 100 // Maximum length of the string data
#define MIN_Theta 0.0001 // Minimum deviation for float type data equality determination



class RecordManager
{
public:
	RecordManager() {}
	RecordManager(BufferManager *bf) :buf_ptr(bf) {}
	~RecordManager();

	Table Select(Table& tableIn, vector<int>attrSelect, vector<int>mask, vector<where>& w);// Select with 'where' restriction
	void Insert(Table& tableIn, Tuple& singleTuper);// Insert a tuple into the table
	int Delete(Table& tableIn, vector<int>mask, vector<where> w);// Delete tuples sataisfying 'where' restriction
	bool DropTable(Table& tableIn);// Drop the table
	bool CreateTable(Table& tableIn);// Create a table
	void createindex(Table& tableIn, int mask);  //Updated(when create an index on attribute mask, insert all values into index)
	void dropindex(Table& tableIn, int mask); //drop the index on attribute mask

	bool isSatisfied(Table& tableinfor, Tuple& row, vector<int> mask, vector<where> w);// Whether the tuple satisfies the 'where' restriction
	Table Select(Table& tableIn, vector<int>attrSelect);// Select without 'where' restriction
	Table SelectProject(Table& tableIn, vector<int>attrSelect);// Select the chosen projects out
	int FindWithIndex(Table& tableIn, Tuple& row, int mask); // Find the corresponding adress of tuple with index
	char* TupleToChar(Table& tableIn, Tuple& singleTuper);// Change the tuple into char*
	Tuple StringToTuper(Table& tableIn, string stringRow);// Change the string back into tuple
	int get_block_num(string input_title); // Get the block number of the table
	void Select(Table & tableIn, int mask, where& w);

private:  
	BufferManager *buf_ptr = new BufferManager;
	IndexManager im;
};

#endif
