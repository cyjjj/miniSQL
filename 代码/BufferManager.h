#pragma once
#ifndef _BUFFERMANAGER_H
#define _BUFFERMANAGER_H
#include <fstream>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <cstring>
#include "Basic.h"
using namespace std;

#define BlockSize 4096
#define MaxBlockNum 128

struct insertPos {
	int bufferNum;//the id of block in buffer
	int position; //position in block
};

struct BufferBlock {
	string filename;
	bool dirty;
	bool valid;
	int blockoffset; //block offset in file, indicate position in file
	int LRUvalue;
	char content[BlockSize];
	
	BufferBlock() { reset(); }
	void reset()
	{
		filename = "";
		dirty = valid = blockoffset = LRUvalue = 0;
		memset(content, 0, BlockSize);
	}
	string getvalues(int start, int end) {
		string tmpt = "";
		if (start >= 0 && start <= end && end <= BlockSize)
			for (int i = start; i < end; i++)
				tmpt += content[i];
		return tmpt;
	}
};
class BufferManager
{
public:
	BufferManager();
	~BufferManager();

	int GiveMeABlock(string filename, int blockoffset); // Given filename and blockoffset, get the bufferNum(already in buffer or get empty block and readBlock)
	insertPos getInsertPosition(Table& tableinfor);// Get the valid insert position
	void writeBlock(int bufferNum);// Write the block, update the LRUvalue, marked as dirty
    void readBlock(string filename, int blockoffset, int bufferNum);// Read the block from file, update the LRUvalue

	void FlashBack(int bufferNum);// Write the block to file
	int getbufferNum(string filename, int blockoffset);// Get the bufferNum
	int getEmptyBuffer();// Find an empty block
	int getEmptyBufferExcept(string filename);// Find an empty block without replacing given file
	int addBlockInFile(Table& tableinfor);// Add a new block to the file and return the buffernum
	int getIfIsInBuffer(string filename, int blockoffset);// Return the buffernum if the block is in the buffer
	void ScanIn(Table tableinfo);// Read in the whole table into buffer
	void setInvalid(string filename);// mark all blocks of the file as invalid(when drop table and drop index)

	BufferBlock Block[MaxBlockNum];

private:
	int maxLRU; // Maximum of LRUvalues, always keep updating
	
};
#endif
