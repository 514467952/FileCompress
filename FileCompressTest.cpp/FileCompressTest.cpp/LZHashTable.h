#pragma once
#include "Common.h"
class LZHashTable
{
public:
	LZHashTable(USH size);
	~LZHashTable();

	//ƥ������ͷ�����ַ����ַ���window�е�λ�ã���ϣ��ַ
	void Insert(USH& matchHead,UCH ch,USH pos,USH& hashAddr);
	void HashFunc(USH& hashAddr, UCH ch);
	USH H_SHIFT();
	USH GetNext(USH& matchHead); //��ȡǰһ���Ĺ�ϣ��ַ
	void Update();
private:
	USH* _prev; 
	USH* _head;
};