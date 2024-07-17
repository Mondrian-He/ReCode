#ifndef BADDTREE_H_H_H
#define BADDTREE_H_H_H

#include "BAddTreeNode.h"
#include "index_ebuffer.h"

using namespace std;

namespace BAT {
	using namespace BATN;
	template<typename K, typename E>
	class BAddTree
	{
	public:
		BAddTree();
		explicit BAddTree(int order);
		~BAddTree();

		int const order()const;
		int const size()const;
		EBuffer* ebuffer;
	
		virtual BAddTreeLeafNode<K,E>*  search(E const&);
		virtual bool exist(E const&);
		virtual E* find(K const &,MBuf_des* mbdes,char** mbpool);
		virtual bool insert(E const&,MBuf_des* mbdes,char** mbpool);
		virtual bool remove(E const&);

		template<typename Visist>
		void list_traversal(Visist );
	protected:
		
		void solveOverFlow(BAddTreeLeafNode<K, E>*,MBuf_des* mbdes,char** mbpool);
		void solveUnderFlow(BAddTreeLeafNode<K, E>*);
	protected:
		int m_size;
		int m_order;
		BAddTreeNode<K,E>* m_root;
		BAddTreeNode<K,E>* m_hitParentNode;
		BAddTreeLeafNode<K, E>*m_header, *m_trail;
	};

	template<typename K, typename E>
	inline BAddTree<K, E>::BAddTree(int order)
	{
		if (order < 2)order = 2;
		m_order = order;
		ebuffer=new EBuffer;
		this->m_root = new BAddTreeLeafNode<K,E>(); m_size = 0; m_hitParentNode = nullptr;
		m_header = new BAddTreeLeafNode<K,E>();
		m_trail = new BAddTreeLeafNode<K,E>();
		BAddTreeLeafNode<K, E>*_leaf = (BAddTreeLeafNode<K, E>*)(this->m_root);
		m_header->next = _leaf; m_trail->last = _leaf;
		_leaf->last = m_header; _leaf->next = m_trail; 
	}

	template<typename K, typename E>
	inline BAddTree<K, E>::~BAddTree()
	{
		if (!this->m_root)return;
		queue<BAddTreeNode<K,E>*>q;
		q.push(this->m_root);
		while (!q.empty())
		{
			BAddTreeNode<K,E>*p = q.front();
			q.pop();
			int count = p->child.size();
			for (int i = 0; i < count; ++i) {
				if (p->child[i]) {
					q.push(p->child[i]);
				}
			}
			if (p->isLeaf()) {
				BAddTreeLeafNode<K, E>*_t = (BAddTreeLeafNode<K, E>*)p;
				delete _t, _t = nullptr;
			}
			else {
				delete p, p = nullptr;
			}
		}
		delete m_header, m_header = nullptr;
		delete m_trail, m_trail = nullptr;
		this->m_root = nullptr;
		this->m_size = 0;
	}

	template<typename K, typename E>
	inline int const BAddTree<K, E>::order() const
	{
		return m_order;
	}

	template<typename K, typename E>
	inline int const BAddTree<K, E>::size() const
	{
		return m_size;
	}

	template<typename K, typename E>
	inline bool BAddTree<K, E>::exist(E const &e)
	{
		this->m_hitParentNode = nullptr;
		K k = e.key();
		BAddTreeLeafNode<K, E>*v = static_cast<BAddTreeLeafNode<K, E>*>(this->m_root);
		while (!v->isLeaf())
		{
			int _s = v->key.size();
			int i = 0;
			for (; i < _s; ++i) {
				K&tk = v->key[i];
				if (tk > k)break;
			}
			this->m_hitParentNode = static_cast<BAddTreeNode<K, E>*>(v);
			v = static_cast<BAddTreeLeafNode<K, E>*>(v->child[i]);
		}
		if (v&&v->isLeaf()) {
			int _s = v->key.size();
			int i = 0;
			for (; i < _s; ++i) {
				K&tk = v->key[i];
				if (tk == k) {
					return true;
				}
				else if (tk > k) 
					return false;
              }
		}
		return false;
	}

	template<typename K, typename E>
	inline E* BAddTree<K, E>::find(K const &k,MBuf_des* mbdes,char** mbpool)
	{

		this->m_hitParentNode = nullptr;
		BAddTreeLeafNode<K, E>*v = static_cast<BAddTreeLeafNode<K, E>*>(this->m_root);

		while (!v->isLeaf())
		{
			int node_id = v->node_id;
			ebuffer->IncTotPageUpdateNum();
			BAddTreeLeafNode<K, E>* node;
			if (ebuffer->node2buffer.find(node_id) != ebuffer->node2buffer.end()) { 
				printf("EBuffer HIT!\t");
				ebuffer->IncHitNum();
				int bfid = ebuffer->node2buffer[node_id];
				
				node = (BAddTreeLeafNode<K, E>*) ebuffer->ebuf_pool[bfid].field;

			}
			else{
				
				int target_bid = ebuffer->size;
				bool do_insert = false;

				if (ebuffer->size >= EBUFFER_NUM_MAX_SIZE) { 
					target_bid = ebuffer->SelectVictim();
					if (ebuffer->ebuf_des[target_bid].dirty == 1) {
						ebuffer->WriteBuffer(target_bid,mbdes,mbpool);
						ebuffer->node2buffer.erase(ebuffer->buffer2node[target_bid]);
					}
				} else {
					target_bid = ebuffer->size;
					do_insert = true;
				}

				ebuffer->ReadPage(node_id,ebuffer->ebuf_pool[target_bid],mbdes,mbpool);

				node = (BAddTreeLeafNode<K, E>*) ebuffer->ebuf_pool[target_bid].field;

				ebuffer->ebuf_des[target_bid].update(node_id, target_bid, 0, false);
				do_insert ? ebuffer->LRUInsert(target_bid) : ebuffer->LRUUpdate(target_bid);
				do_insert ? ebuffer->size += 1 : 0;
				ebuffer->node2buffer[node_id] = target_bid;
				ebuffer->buffer2node[target_bid] = node_id;
			}

			int _s = node->key.size();
			int i = 0;
			for (; i < _s; ++i) {
				K&tk = node->key[i];
				if (tk > k)break;
			}
			this->m_hitParentNode = static_cast<BAddTreeNode<K, E>*>(node);
			v = static_cast<BAddTreeLeafNode<K, E>*>(node->child[i]);
			
		}

		if (v&&v->isLeaf()) {
			int node_id = v->node_id;
			ebuffer->IncTotPageUpdateNum();
			BAddTreeLeafNode<K, E>* node;
			if (ebuffer->node2buffer.find(node_id) != ebuffer->node2buffer.end()) {
				printf("EBuffer HIT!\n");
				ebuffer->IncHitNum();
				int bfid = ebuffer->node2buffer[node_id];
				node = (BAddTreeLeafNode<K, E>*) ebuffer->ebuf_pool[bfid].field;
			}
			else{
				int target_bid = ebuffer->size;
				bool do_insert = false;

				if (ebuffer->size >= EBUFFER_NUM_MAX_SIZE) {
					target_bid = ebuffer->SelectVictim();
					if (ebuffer->ebuf_des[target_bid].dirty == 1) {
						ebuffer->WriteBuffer(target_bid,mbdes,mbpool);
						ebuffer->node2buffer.erase(ebuffer->buffer2node[target_bid]);
					}
				} else {
					target_bid = ebuffer->size;
					do_insert = true;
				}

				ebuffer->ReadPage(node_id,ebuffer->ebuf_pool[target_bid],mbdes,mbpool);
				node = (BAddTreeLeafNode<K, E>*) ebuffer->ebuf_pool[target_bid].field;
				ebuffer->ebuf_des[target_bid].update(node_id, target_bid, 0, false);
				do_insert ? ebuffer->LRUInsert(target_bid) : ebuffer->LRUUpdate(target_bid);
				do_insert ? ebuffer->size += 1 : 0;
				ebuffer->node2buffer[node_id] = target_bid;
				ebuffer->buffer2node[target_bid] = node_id;
			}
			int _s = node->key.size();
			int i = 0;
			for (; i < _s; ++i) {
				K&tk = node->key[i];
				if (tk == k) {
					return node->e[i];
				}
				else if (tk > k) 
					break;
              }
			this->m_hitParentNode = static_cast<BAddTreeNode<K, E>*>(node);
			v = static_cast<BAddTreeLeafNode<K, E>*>(node->child[i]);
		}
		
	}
	
	template<typename K, typename E>
	inline BAddTreeLeafNode<K, E>* BAddTree<K, E>::search(E const &e)
	{
		this->m_hitParentNode = nullptr;
		K k = e.key();
		BAddTreeLeafNode<K, E>*v = static_cast<BAddTreeLeafNode<K, E>*>(this->m_root);
		while (!v->isLeaf())
		{
			int _s = v->key.size();
			int i = 0;
			for (; i < _s; ++i) {
				K&tk = v->key[i];
				if (tk > k)break;
			}
			this->m_hitParentNode = static_cast<BAddTreeNode<K, E>*>(v);
			v = static_cast<BAddTreeLeafNode<K, E>*>(v->child[i]);
		}
		if (v&&v->isLeaf()) {
			int _s = v->key.size();
			int i = 0;
			for (; i < _s; ++i) {
				K&tk = v->key[i];
				if (tk == k) {
					return v;
				}
				else if (tk > k) 
					break;
              }
			  
			this->m_hitParentNode = static_cast<BAddTreeNode<K, E>*>(v);
			v = static_cast<BAddTreeLeafNode<K, E>*>(v->child[i]);
		}
		
		return v;
	}

	template<typename K, typename E>
	inline bool BAddTree<K, E>::insert(E const &e,MBuf_des* mbdes,char** mbpool)
	{
		printf("tree.insert\n");

		K k = e.key();
		BAddTreeLeafNode<K, E>*v = this->search(e);

		if (v) {
			return false;
		}
		else {

			v = static_cast<BAddTreeLeafNode<K, E>*>(this->m_hitParentNode);
			int _s = v->key.size();
			
			int i = 0;
			for (; i < _s; ++i) {
				K&tk = v->key[i];

				if (tk > k)break;
			}
			v->key.insert(v->key.begin()+i,k);
			++this->m_size;		
			v->child.insert(v->child.begin() + i + 1, nullptr);
			
			E*_e = new E();
			*_e =std::move(e);
			v->e.insert(v->e.begin() + i, _e);

				if(size() == 1){

					int target_bid = ebuffer->size;		
					int node_id = v->node_id;
					memcpy(ebuffer->ebuf_pool[target_bid].field,v,EBUFFER_SIZE);

					ebuffer->ebuf_des[target_bid].update(node_id, target_bid, 0, false);
					ebuffer->SetDirty(target_bid);
					ebuffer->LRUInsert(target_bid);
					ebuffer->size += 1;

					ebuffer->node2buffer[node_id] = target_bid;
					ebuffer->buffer2node[target_bid] = node_id;
				}
				else{

					int node_id = v->node_id;
					BAddTreeLeafNode<K, E>* node;
					int bid = ebuffer->size;

					if (ebuffer->node2buffer.find(node_id) != ebuffer->node2buffer.end()) {
						bid = ebuffer->node2buffer[node_id];
						memcpy(ebuffer->ebuf_pool[bid].field,v,EBUFFER_SIZE);
						ebuffer->SetDirty(bid);
						ebuffer->LRUUpdate(bid);
					}
					else{
						
						int target_bid = ebuffer->size;
						bool do_insert = false;

						if (ebuffer->size >= EBUFFER_NUM_MAX_SIZE) {
							target_bid = ebuffer->SelectVictim();
							if (ebuffer->ebuf_des[target_bid].dirty == 1) { 
								ebuffer->WriteBuffer(target_bid,mbdes,mbpool);
								ebuffer->node2buffer.erase(ebuffer->buffer2node[target_bid]);
							}
						} else {
							target_bid = ebuffer->size;
							do_insert = true;
						}

						memcpy(ebuffer->ebuf_pool[target_bid].field,v,EBUFFER_SIZE);
						ebuffer->ebuf_des[target_bid].update(node_id, target_bid, 0, false);
						do_insert ? ebuffer->LRUInsert(target_bid) : ebuffer->LRUUpdate(target_bid);
						do_insert ? ebuffer->size += 1 : 0;
						ebuffer->node2buffer[node_id] = target_bid;
						ebuffer->buffer2node[target_bid] = node_id;
					}

				}
				solveOverFlow(v,mbdes,mbpool);

			return true;
		}
	}

	template<typename K, typename E>
	inline bool BAddTree<K, E>::remove(E const &e)
	{
		BAddTreeLeafNode<K, E>*v = this->search(e);
		if (!v) {
			return false;
		}
		int rank = 0; while (v->key[rank] != e.key())++rank;
		v->key.erase(v->key.begin() + rank);
		E*_e = v->e[rank];
		v->e.erase(v->e.begin() + rank);
		--this->m_size;
		delete _e;
		v->child.erase(v->child.begin() + rank + 1);
		solveUnderFlow(v);
		return true;
	}

	template<typename K, typename E>
	template<typename Visist>
	inline void BAddTree<K, E>::list_traversal(Visist visit)
	{
		BAddTreeLeafNode<K, E>*p = m_header;
		while ((p=p->next) != m_trail) {
			visit(p->e);
		}
	}

	template<typename K, typename E>
	inline BAddTree<K, E>::BAddTree()
	{
		m_size = -1; m_order = -1; m_root = nullptr; m_hitParentNode = nullptr; m_header = nullptr; m_trail = nullptr;
	}

	template<typename K, typename E>
	inline void BAddTree<K, E>::solveOverFlow(BAddTreeLeafNode<K, E>*v,MBuf_des* mbdes,char** mbpool)
	{
		while (v->child.size()>m_order)
		{
			int mid =m_order / 2;
			BAddTreeLeafNode<K, E>*_r;

			if (v->isLeaf()) {
				_r= new BAddTreeLeafNode<K, E>();

				{
					_r->e.insert(_r->e.begin(), v->e.begin() + mid, v->e.end());
					v->e.erase(v->e.begin() + mid, v->e.end());
				}
				{
					_r->key.insert(_r->key.begin(), v->key.begin() + mid, v->key.end());
					v->key.erase(v->key.begin() + mid+1, v->key.end());
				}
				{
					_r->child.insert(_r->child.begin(), v->child.begin() + mid, v->child.end() - 1);
					v->child.erase(v->child.begin() + mid + 1, v->child.end() - 1);
					_r->child[_r->child.size() - 1] = v->child[v->child.size() - 1];
					v->child.erase(v->child.end()-1);
				}
				v->next->last = _r; _r->next = v->next;
				_r->last = v; v->next = _r;
				
			}
			else {
				_r=static_cast<BAddTreeLeafNode<K,E>*>(new BAddTreeNode<K, E>());

				{
					_r->key.insert(_r->key.begin(), v->key.begin() + mid+1, v->key.end());
					v->key.erase(v->key.begin() + mid+1, v->key.end());
				}
				{
					_r->child.insert(_r->child.begin(), v->child.begin() + mid + 1, v->child.end() - 1);
					v->child.erase(v->child.begin() + mid + 1, v->child.end() - 1);
					_r->child[_r->child.size() - 1] = v->child[v->child.size() - 1];
					v->child.erase(v->child.end()-1);
				}

			}
			{

				{
					int _s = _r->child.size();
					
					if (_r->child[0])
						for (int i = 0; i < _s; ++i)
							_r->child[i]->parent = static_cast<BAddTreeNode<K, E>*>(_r);
				}
				BAddTreeNode<K, E>*p = v->parent;

				if (!p) {
					this->m_root = p = new BAddTreeNode<K, E>(); p->child[0] = v; v->parent = p;

					K& k = v->key[v->key.size() - 1];
					int i = 0; while (p->child[i] != v){++i;}
					
					p->key.insert(p->key.begin()+i, k);
					p->child.insert(p->child.begin()+ i + 1, _r);
					_r->parent = p;
					v->key.erase(v->key.end() - 1);

					int target_bpid = ebuffer->size;
					bool pdo_insert = false; 
					if (ebuffer->size >= EBUFFER_NUM_MAX_SIZE) {
						target_bpid = ebuffer->SelectVictim(); 
						if(ebuffer->ebuf_des[target_bpid].dirty){ 
							ebuffer->WriteBuffer(target_bpid,mbdes,mbpool);
							ebuffer->node2buffer.erase(ebuffer->buffer2node[target_bpid]);
							
						} else {
							ebuffer->node2buffer.erase(ebuffer->buffer2node[target_bpid]);
							
						}
					} else { 
						target_bpid = ebuffer->size;
						pdo_insert = true;
					}
					int node_pid=p->node_id;

					memcpy(ebuffer->ebuf_pool[target_bpid].field,p,EBUFFER_SIZE);

					ebuffer->ebuf_des[target_bpid].update(node_pid, target_bpid, 0, false);
					ebuffer->SetDirty(target_bpid);
					pdo_insert ? ebuffer->LRUInsert(target_bpid) : ebuffer->LRUUpdate(target_bpid);
					ebuffer->size += 1;
					ebuffer->node2buffer[node_pid] = target_bpid;
					ebuffer->buffer2node[target_bpid] = node_pid;

				}
				else{

					K& k = v->key[v->key.size() - 1];
					int i = 0; while (p->child[i] != v){++i;}

					p->key.insert(p->key.begin()+i, k);
					p->child.insert(p->child.begin()+ i + 1, _r);
					_r->parent = p;
					v->key.erase(v->key.end() - 1);

					int node_id=p->node_id;
					int target_bid = ebuffer->node2buffer[node_id];
					memcpy(ebuffer->ebuf_pool[target_bid].field,p,EBUFFER_SIZE);
					ebuffer->ebuf_des[target_bid].update(node_id, target_bid, 0, false);
					ebuffer->SetDirty(target_bid);
					ebuffer->LRUUpdate(target_bid);


				}
					int old_vid = v->node_id;
					int old_bvid = ebuffer->node2buffer[old_vid];
					memcpy(ebuffer->ebuf_pool[old_bvid].field,v,EBUFFER_SIZE);
					ebuffer->ebuf_des[old_bvid].update(old_vid, old_bvid, 0, false);
					ebuffer->SetDirty(old_bvid);
					ebuffer->LRUUpdate(old_bvid);

					int target_brid = ebuffer->size;
					
					bool rdo_insert = false;
					if (ebuffer->size >= EBUFFER_NUM_MAX_SIZE) {
						target_brid = ebuffer->SelectVictim(); 

						if(ebuffer->ebuf_des[target_brid].dirty){
							ebuffer->WriteBuffer(target_brid,mbdes,mbpool);
							ebuffer->node2buffer.erase(ebuffer->buffer2node[target_brid]);
							
						} else { 
							ebuffer->node2buffer.erase(ebuffer->buffer2node[target_brid]);
						}
					} else { 
						target_brid = ebuffer->size;
						rdo_insert = true;
					}

					int node_rid=_r->node_id;
					memcpy(ebuffer->ebuf_pool[target_brid].field,_r,EBUFFER_SIZE);
					ebuffer->ebuf_des[target_brid].update(node_rid, target_brid, 0, false);
					ebuffer->SetDirty(target_brid);
					rdo_insert ? ebuffer->LRUInsert(target_brid) : ebuffer->LRUUpdate(target_brid);
					ebuffer->size += 1;
					ebuffer->node2buffer[node_rid] = target_brid;
					ebuffer->buffer2node[target_brid] = node_rid;

				v = static_cast<BAddTreeLeafNode<K, E>*>(p);

			}
		}
	}

	template<typename K, typename E>
	inline void BAddTree<K, E>::solveUnderFlow(BAddTreeLeafNode<K, E>*v)
	{
		size_t sep = (m_order+1) / 2;
		while (v->child.size()<sep)
		{
			BAddTreeNode<K,E>*p = v->parent;
			if (!p) {
				if (v->key.empty()&&v->child[0]) {
					this->m_root = v->child[0];
					this->m_root->parent = nullptr;
					if (v->isLeaf()) {
						v->next->last = v->last; v->last->next = v->next;
						delete v, v = nullptr;
					}
					else {
						BAddTreeNode<K, E>* _n = static_cast<BAddTreeNode<K, E>*>(v);
						delete _n, _n = nullptr;
					}
				}
				return;
			}

			int rank = 0;while (p->child[rank] != v)++rank;
			
			if (v->isLeaf()) {
				if (rank > 0) {
					BAddTreeLeafNode<K, E>*_left = static_cast<BAddTreeLeafNode<K, E>*>(p->child[rank - 1]);
					if (_left->child.size() > sep) {
						v->child.insert(v->child.begin(), _left->child[_left->child.size() - 1]);
						_left->child.erase(_left->child.end() - 1);
						{
							auto it = --_left->key.end();
							p->key[rank - 1] = *(it);
							v->key.insert(v->key.begin(), *it);
							_left->key.erase(it);
						}
						{
							auto it = --_left->e.end();
							v->e.insert(v->e.begin(), *it);
							_left->e.erase(it);
						}
						return;
					}
				}
				if (rank < p->child.size() - 1) {
					BAddTreeLeafNode<K, E>*_right = static_cast<BAddTreeLeafNode<K, E>*>(p->child[rank+1]);
					if (_right->child.size() > sep) {
						v->child.insert(v->child.end(), _right->child[0]);
						_right->child.erase(_right->child.begin());
						{
							auto it =_right->key.begin()+1;
							p->key[rank ] = *(it--);
							v->key.insert(v->key.end(), *it);
							_right->key.erase(it);
						}
						{
							auto it = _right->e.begin();
							v->e.insert(v->e.end(), *it);
							_right->e.erase(it);
						}
						return;
					}
				}
				if (rank > 0) {
					BAddTreeLeafNode<K, E>*_left = static_cast<BAddTreeLeafNode<K, E>*>(p->child[rank - 1]);
					v->child.erase(v->child.begin());
					_left->child.insert(_left->child.end(), v->child.begin(), v->child.end());
					_left->key.insert(_left->key.end(), v->key.begin(), v->key.end());

					p->child.erase(p->child.begin()+rank);

					_left->e.insert(_left->e.end(), v->e.begin(), v->e.end());
					v->e.resize(0);

					v->last->next = v->next; v->next->last = v->last;
					delete v, v = nullptr;
					v = static_cast<BAddTreeLeafNode<K, E>*>(p);
					continue;
				}
				else {
					BAddTreeLeafNode<K, E>*_right = static_cast<BAddTreeLeafNode<K, E>*>(p->child[rank +1]);
					_right->child.erase(_right->child.begin());
					_right->child.insert(_right->child.begin(), v->child.begin(), v->child.end());
					_right->key.insert(_right->key.begin(), v->key.begin(), v->key.end());
					p->child.erase(p->child.begin() + rank);
					p->key.erase(p->key.begin() + rank );

					_right->e.insert(_right->e.begin(), v->e.begin(), v->e.end());
					v->e.resize(0);
					v->last->next = v->next; v->next->last = v->last;
					delete v, v = nullptr;
					v = static_cast<BAddTreeLeafNode<K, E>*>(p);
					continue;
				}
			}
			else {
				if (rank > 0) {
					BAddTreeNode<K,E>*_left = p->child[rank - 1];
					if (_left->child.size() > sep) {
						v->child.insert(v->child.begin(), _left->child[_left->child.size() - 1]);
						if (v->child[0])v->child[0]->parent = v;
						_left->child.erase(_left->child.end() - 1);
							auto it = --_left->key.end();
                            v->key.insert(v->key.begin(), p->key[rank - 1]);
							p->key[rank - 1] = *(it);
							_left->key.erase(it);
						return;
					}
				}
				if (rank < p->child.size() - 1) {
					BAddTreeNode<K,E>*_right = p->child[rank + 1];
					if (_right->child.size() > sep) {
						v->child.insert(v->child.end(), _right->child[0]);
						if (v->child[v->child.size()-1])v->child[v->child.size() - 1]->parent = v;
						_right->child.erase(_right->child.begin());
							auto it = _right->key.begin();
							v->key.insert(v->key.end(), p->key[rank]);
							p->key[rank] = *(it);
							_right->key.erase(it);
						return;
					}
				}
				if (rank > 0) {
					BAddTreeNode<K,E>*_left = p->child[rank - 1];
					_left->child.insert(_left->child.end(), v->child.begin(), v->child.end());
					{
						auto it = _left->child.end() - v->child.size();
						auto ite = _left->child.end();
						while (it!=ite)
						{
							if(*it)(*it)->parent = _left;
							++it;
						}
					}
					_left->key.insert(_left->key.end(),p->key[rank-1]);
					_left->key.insert(_left->key.end(), v->key.begin(), v->key.end());
					p->child.erase(p->child.begin() + rank);
					p->key.erase(p->key.begin() + rank - 1);
					{
						BAddTreeNode<K, E>*_t = static_cast<BAddTreeNode<K, E>*>(v);
						delete _t, _t = nullptr;
					}
					v = static_cast<BAddTreeLeafNode<K, E>*>(p);
					continue;
				}
				else {
					BAddTreeNode<K,E>*_right = p->child[rank + 1];
					_right->child.insert(_right->child.begin(), v->child.begin(), v->child.end());
					{
						auto it = _right->child.begin();
						auto ite = _right->child.begin()+v->child.size();
						while (it != ite)
						{
							if(*it)(*it)->parent = _right;
							++it;
						}
					}
					_right->key.insert(_right->key.begin(), p->key[rank]);
					_right->key.insert(_right->key.begin(), v->key.begin(), v->key.end());
					p->child.erase(p->child.begin() + rank);
					p->key.erase(p->key.begin() + rank);
					{
						BAddTreeNode<K, E>*_t = static_cast<BAddTreeNode<K, E>*>(v);
						delete _t, _t = nullptr;
					}
					v = static_cast<BAddTreeLeafNode<K, E>*>(p);
					continue;
				}
			}
		}
	}

}

#endif // !BADDTREE_H_H_H

