#include"LZHashTable.h"
#include"Common.h"
#include<string.h>


const USH HASH_BITS = 15;  //一个哈希地址占15个比特位
const USH HASH_SIZE = (1 << HASH_BITS);
const USH HASH_MASK = HASH_SIZE - 1; //哈希的掩码，保证不会越界
LZHashTable::LZHashTable(USH size)
:_prev(new USH[size*2])
, _head(_prev+size)
{
	memset(_prev, 0, size * 2 * sizeof(USH));
}

LZHashTable::~LZHashTable()
{
	delete[]_prev;
	_prev = nullptr;
}

void LZHashTable::Insert(USH& matchHead, UCH ch, USH pos, USH& hashAddr)
{
	HashFunc(hashAddr, ch);

	//找当前三个字符在查找缓冲区中找到的最近的一个，匹配链的头部
	matchHead = _head[hashAddr];

	// pos可能会超过32K，与mask的目的就是不越界
	_prev[pos&HASH_MASK] = _head[hashAddr];
	_head[hashAddr] = pos;
}

//abcdefgh字符串
//hashaddr1: abc
//hashaddr2 : bcd
//hashAddr:前一次计算出的哈希地址
//本次需要计算bcd哈希地址 bc两个字符在前一次已经算过了，所以只需要传进来d
//ch：本次匹配三个字符中的最后一个
//本次哈希地址是在上一次哈希地址的基础上算出来的
void LZHashTable::HashFunc(USH& hashAddr, UCH ch)
{
	hashAddr = (((hashAddr) << H_SHIFT()) ^ (ch)) & HASH_MASK;
}
USH LZHashTable::H_SHIFT()
{
	return (HASH_BITS + MIN_MATCH - 1) / MIN_MATCH;
}

USH LZHashTable::GetNext(USH& matchHead)
{
	return _prev[matchHead&HASH_MASK];
}

void  LZHashTable::Update()
{
	for (USH i = 0; i < WSIZE; ++i)
	{
		//先更新head
		if (_head[i] >= WSIZE) //右窗
			_head[i] -= WSIZE; //下标变到左窗
		else //左窗
			_head[i] = 0; //清零

		//更新prev
		if (_prev[i] >= WSIZE)
			_prev[i] -= WSIZE;
		else
			_prev[i] = 0;
	}
}