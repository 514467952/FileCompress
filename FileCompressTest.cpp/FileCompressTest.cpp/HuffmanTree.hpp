#pragma once
#include<iostream>
#include<vector>
#include<queue> //要用到优先级队列保存哈夫曼树中的结点
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
	HuffManTreeNode<W>* _pParent;//加上双亲指针域
	W _weight; //结点的权值
};

template<class W>
class Less
{
	typedef HuffManTreeNode<W> Node;
public:
	bool operator()( Node*pLeft,  Node* pRight)
	{
		//没有找到对应CharInfo的转换
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
		//进行建树操作
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
		//1.构建森林
		priority_queue<Node*,vector<Node*>,Less<W>> q;
		for (auto e : vWeight)
		{
			if (e == invaild)//如果是无效的权值，就不放了
				continue;

			q.push(new Node(e));//创建新的结点，树就有了
		}
		//2.看森林里面有没有超过两个树
		while (q.size() > 1)
		{
			Node* pLeft = q.top();
			q.pop();

			Node* pRight = q.top();
			q.pop();

			Node* pParent = new Node(pLeft->_weight + pRight->_weight);
			pParent->_pLeft = pLeft;
			pParent->_pRight = pRight;

			pLeft->_pParent = pParent; //更新双亲
			pRight->_pParent = pParent;
			q.push(pParent);
		}
		//3.森林里只有一棵树了，就是哈夫曼树
		_pRoot = q.top();
	}


private:
	//销毁二叉树，只能用后序遍历的方式进行销毁
	void _DestroyTree(Node*& pRoot)
	{
		if (pRoot)
		{
			_DestroyTree(pRoot->_pLeft);
			_DestroyTree(pRoot->_pRight);
			delete pRoot;
			//要改指针本身，要么传二级指针，要么一级指针的引用
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