#include "BufferManager.h"

using namespace std;

BufferManager::BufferManager()
{
	maxLRU = 0;
	for (int i = 0; i< MaxBlockNum; i++)
		Block[i].reset();
}

BufferManager::~BufferManager()
{
	for (int i = 0; i < MaxBlockNum; i++)
		FlashBack(i);
}

void BufferManager::FlashBack(int bufferNum)
{
	if (Block[bufferNum].dirty) //was written(dirty) 
	{
		string filename = Block[bufferNum].filename;

		FILE *fp;
		if ((fp = fopen(filename.c_str(), "r+b")) == NULL) {
			cout << "Open file error!!!" << endl;
			return;
		}
		fseek(fp, BlockSize * Block[bufferNum].blockoffset, SEEK_SET);// Set the location of the file pointer stream
		fwrite(Block[bufferNum].content, BlockSize, 1, fp); // Write the block into the file
		Block[bufferNum].reset(); // Clear up the block
		fclose(fp);
	}
	else return;
}

int BufferManager::getbufferNum(string filename, int blockoffset)
{
	int bufferNum = getIfIsInBuffer(filename, blockoffset); // Return the buffernum if the block is in the buffer
	if (bufferNum == -1) { // Not in the buffer
		bufferNum = getEmptyBufferExcept(filename); // Find an empty block without replacing given file
		readBlock(filename, blockoffset, bufferNum); // Read from the file
	}
	return bufferNum;
}

void BufferManager::readBlock(string filename, int blockoffset, int bufferNum)
{
	Block[bufferNum].valid = 1;
	Block[bufferNum].dirty = 0;
	Block[bufferNum].filename = filename;
	Block[bufferNum].blockoffset = blockoffset;
	Block[bufferNum].LRUvalue = ++maxLRU;

	FILE *fp;
	if ((fp = fopen(filename.c_str(), "rb")) == NULL) {
		cout << "Open file error" << endl;
		return;
	}
	fseek(fp, BlockSize * blockoffset, SEEK_SET);// Set the location of the file pointer stream
	fread(Block[bufferNum].content, BlockSize, 1, fp);// Read from file
	fclose(fp);

}

void BufferManager::writeBlock(int bufferNum)
{
	Block[bufferNum].dirty = 1;// Marked as dirty
	Block[bufferNum].LRUvalue = ++maxLRU;// Update the LRUvalue of the block and maxLRU of BufferManger
}

int BufferManager::getEmptyBuffer()
{
	int bufferNum = -1; //for invalid buffer block
	int least_recent_block = 0; //has minimum LRUvalue
	for (int i = 0; i < MaxBlockNum; i++) {
		if (!Block[i].valid) {
			Block[i].reset();
			Block[i].valid = 1;
			return i;
		}
		else if (Block[least_recent_block].LRUvalue > Block[i].LRUvalue) {
			least_recent_block = i;
		}
	}//Get the least_recent_block
	
	FlashBack(least_recent_block);// Write to file
	Block[least_recent_block].valid = 1;
	return least_recent_block;
}

int BufferManager::getEmptyBufferExcept(string filename) {
	int bufferNum = -1; //for invalid buffer block
	int least_recent_block = 0;
	for (int i = 0; i < MaxBlockNum; i++) {
		if (!Block[i].valid) {
			Block[i].reset();
			Block[i].valid = 1;
			return i;
		}
		else if (Block[least_recent_block].LRUvalue > Block[i].LRUvalue && Block[i].filename != filename) {
			least_recent_block = i;
		}
	}// Get the least_recent_block which is not block for this file
	
	FlashBack(least_recent_block);// Write to file
	Block[least_recent_block].valid = 1;
	return least_recent_block;
}

insertPos BufferManager::getInsertPosition(Table& tableinfor) {
	insertPos iPos;
	string filename = tableinfor.get_title() + ".table";
	if (tableinfor.blockNum == 0) //new file and no block exist 
	{ 
		iPos.bufferNum = GiveMeABlock(filename,0);
		iPos.position = 0;
		return iPos;
	}
	int length = tableinfor.get_tuple_size() + 1; // One extra bit for flag of valid or not
	int blockoffset = tableinfor.blockNum - 1;// Insert at the end
	int bufferNum = getIfIsInBuffer(filename, blockoffset);
	if (bufferNum == -1) {
		bufferNum = getEmptyBuffer();// Find an empty block
		readBlock(filename, blockoffset, bufferNum);
	}
	int recordNum = BlockSize / length;
	for (int offset = 0; offset < recordNum; offset++) {
		int position = offset * length;
		if (!Block[bufferNum].content[position]) //find an empty space
		{
			iPos.bufferNum = bufferNum;
			iPos.position = position;
			return iPos;
		}
	}
	// The block is full
	iPos.bufferNum = GiveMeABlock(filename,blockoffset+1);
	iPos.position = 0;
	return iPos;
}

int BufferManager::addBlockInFile(Table& tableinfor) {
	int bufferNum = getEmptyBuffer();
	Block[bufferNum].reset();
	Block[bufferNum].valid = 1;
	Block[bufferNum].dirty = 1;
	Block[bufferNum].filename = tableinfor.get_title() + ".table";
	Block[bufferNum].blockoffset = tableinfor.blockNum++;
	Block[bufferNum].LRUvalue = ++maxLRU;

	return bufferNum;
}

int BufferManager::getIfIsInBuffer(string filename, int blockoffset) {
	for (int bufferNum = 0; bufferNum < MaxBlockNum; bufferNum++)
	{
		if (Block[bufferNum].filename == filename && Block[bufferNum].blockoffset == blockoffset)
			return bufferNum;
	}
	return -1; // Not in buffer
}

void BufferManager::ScanIn(Table tableinfo) {
	string filename = tableinfo.get_title() + ".table";
	fstream  fin(filename.c_str(), ios::in);
	for (int blockoffset = 0; blockoffset < tableinfo.blockNum; blockoffset++) {
		if (getIfIsInBuffer(filename, blockoffset) == -1) //block�����ڴ���
		{	
			int bufferNum = getEmptyBufferExcept(filename);
			readBlock(filename, blockoffset, bufferNum);
		}
	}
	fin.close();
}

void BufferManager::setInvalid(string filename) {
	for (int i = 0; i < MaxBlockNum; i++) {
		if (Block[i].filename == filename) {
			Block[i].valid = 0;
			Block[i].dirty = 0;
		}
	}
}

int BufferManager::GiveMeABlock(string filename, int blockoffset)
{
	int bufferNum = getIfIsInBuffer(filename, blockoffset);
	if (bufferNum == -1) {
		bufferNum = getEmptyBuffer();
		readBlock(filename, blockoffset, bufferNum);
	}
	Block[bufferNum].LRUvalue = ++maxLRU;
	return bufferNum;
}