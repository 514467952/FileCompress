#pragma once
#include "Common.h"
class LZHashTable
{
public:
	LZHashTable(USH size);
	~LZHashTable();

	//匹配链的头部，字符，字符在window中的位置，哈希地址
	void Insert(USH& matchHead,UCH ch,USH pos,USH& hashAddr);
	void HashFunc(USH& hashAddr, UCH ch);
	USH H_SHIFT();
	USH GetNext(USH& matchHead); //获取前一个的哈希地址
	void Update();
private:
	USH* _prev; 
	USH* _head;
};