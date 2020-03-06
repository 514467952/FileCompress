#include"LZHashTable.h"
#include"Common.h"
#include<string.h>


const USH HASH_BITS = 15;  //һ����ϣ��ַռ15������λ
const USH HASH_SIZE = (1 << HASH_BITS);
const USH HASH_MASK = HASH_SIZE - 1; //��ϣ�����룬��֤����Խ��
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

	//�ҵ�ǰ�����ַ��ڲ��һ��������ҵ��������һ����ƥ������ͷ��
	matchHead = _head[hashAddr];

	// pos���ܻᳬ��32K����mask��Ŀ�ľ��ǲ�Խ��
	_prev[pos&HASH_MASK] = _head[hashAddr];
	_head[hashAddr] = pos;
}

//abcdefgh�ַ���
//hashaddr1: abc
//hashaddr2 : bcd
//hashAddr:ǰһ�μ�����Ĺ�ϣ��ַ
//������Ҫ����bcd��ϣ��ַ bc�����ַ���ǰһ���Ѿ�����ˣ�����ֻ��Ҫ������d
//ch������ƥ�������ַ��е����һ��
//���ι�ϣ��ַ������һ�ι�ϣ��ַ�Ļ������������
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
		//�ȸ���head
		if (_head[i] >= WSIZE) //�Ҵ�
			_head[i] -= WSIZE; //�±�䵽��
		else //��
			_head[i] = 0; //����

		//����prev
		if (_prev[i] >= WSIZE)
			_prev[i] -= WSIZE;
		else
			_prev[i] = 0;
	}
}