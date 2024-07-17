#ifndef BADDTREENODE_H_H_H
#define BADDTREENODE_H_H_H

#include <deque>
#include "Ocall_wrappers.h"
using namespace std;

int numcount=0;

namespace BATN {

	template<typename K, typename E>
	class BAddTreeNode {
	public:
		explicit BAddTreeNode();
	   ~BAddTreeNode();
	    bool isLeaf()const;
	public:
        int node_id;

		BAddTreeNode<K, E>* parent;
		std::deque<K>key;
		std::deque<BAddTreeNode<K, E>*>child;
};

	template<typename K, typename E>
	inline BAddTreeNode<K, E>::BAddTreeNode()
	{
        node_id=++numcount;

		parent = nullptr;
		child.insert(child.end(), nullptr);
	}


	template<typename K, typename E>
	inline BAddTreeNode<K, E>::~BAddTreeNode()
	{
	
	}

	template<typename K, typename E>
	inline bool BAddTreeNode<K, E>::isLeaf() const
	{
		return !child[0];
	}

	template<typename K, typename E>
	class BAddTreeLeafNode :public BAddTreeNode<K,E>
	{
	public:
		explicit BAddTreeLeafNode();
		~BAddTreeLeafNode();
	public:
		std::deque <E*>e;
		BAddTreeLeafNode<K, E>*last, *next;
	};

	
	template<typename K, typename E>
	inline BAddTreeLeafNode<K, E>::BAddTreeLeafNode()
	{
		last = nullptr; next = nullptr;
	}
	template<typename K, typename E>
	inline BAddTreeLeafNode<K, E>::~BAddTreeLeafNode()
	{
		int _s = e.size();
		for (int i = 0; i < _s; ++i) {
			delete e[i], e[i] = nullptr;
		}
		e.resize(0);
	}
}
#endif // !BADDTREENODE_H_H_H

