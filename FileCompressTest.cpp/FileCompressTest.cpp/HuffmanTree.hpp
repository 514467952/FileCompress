#pragma once
#include<iostream>
#include<vector>
#include<queue> //Ҫ�õ����ȼ����б�����������еĽ��
using namespace std;
template<class W>
struct HuffManTreeNode
{
	HuffManTreeNode(const W& weight = W())
	:_pLeft(nullptr)
	, _pRight(nullptr)
	, _pParent(nullptr)
	, _weight(weight)
	{}
	HuffManTreeNode<W>* _pLeft;
	HuffManTreeNode<W>* _pRight;
	HuffManTreeNode<W>* _pParent;//����˫��ָ����
	W _weight; //����Ȩֵ
};

template<class W>
class Less
{
	typedef HuffManTreeNode<W> Node;
public:
	bool operator()( Node*pLeft,  Node* pRight)
	{
		//û���ҵ���ӦCharInfo��ת��
		return pLeft->_weight > pRight->_weight;
	}
};


template<class W>
class HuffManTree
{
	typedef HuffManTreeNode<W> Node;
public:
	HuffManTree()
		:_pRoot(nullptr)
	{}

	HuffManTree(const vector<W>& vWeight, const W& invaild)
	{
		//���н�������
		CreatHuffManTree(vWeight,invaild);
	}
	~HuffManTree()
	{
		_DestroyTree(_pRoot);
	}

	Node* GetRoot()
	{
		return _pRoot;
	}
	void CreatHuffManTree(const vector<W>& vWeight,const W& invaild)
	{
		//1.����ɭ��
		priority_queue<Node*,vector<Node*>,Less<W>> q;
		for (auto e : vWeight)
		{
			if (e == invaild)//�������Ч��Ȩֵ���Ͳ�����
				continue;

			q.push(new Node(e));//�����µĽ�㣬��������
		}
		//2.��ɭ��������û�г���������
		while (q.size() > 1)
		{
			Node* pLeft = q.top();
			q.pop();

			Node* pRight = q.top();
			q.pop();

			Node* pParent = new Node(pLeft->_weight + pRight->_weight);
			pParent->_pLeft = pLeft;
			pParent->_pRight = pRight;

			pLeft->_pParent = pParent; //����˫��
			pRight->_pParent = pParent;
			q.push(pParent);
		}
		//3.ɭ����ֻ��һ�����ˣ����ǹ�������
		_pRoot = q.top();
	}


private:
	//���ٶ�������ֻ���ú�������ķ�ʽ��������
	void _DestroyTree(Node*& pRoot)
	{
		if (pRoot)
		{
			_DestroyTree(pRoot->_pLeft);
			_DestroyTree(pRoot->_pRight);
			delete pRoot;
			//Ҫ��ָ�뱾��Ҫô������ָ�룬Ҫôһ��ָ�������
			pRoot = nullptr;
		}
	}
private:
	Node * _pRoot;
};

//void TestHuffmanTree()
//{
//	vector<int>v{ 3, 1, 7, 5 };
//	HuffManTree<int> t(v);
//}