#include<iostream>
#include<vector>
#include<numeric>
#include<algorithm>
#include<math.h>
using namespace std;
 
#ifndef KDTREE_H
#define KDTREE_H
 
 
class KDTree
{
public:
	struct KDNode
	{
		bool m_isLeaf;
		vector<double> m_point;
		int m_split;
		KDNode* m_parentNode;
		KDNode* m_leftNode;
		KDNode* m_rightNode;
	};
    vector<vector<double> > m_points;
private:
	using st = vector<double>::size_type;
 
	KDNode* m_root;
	int m_k;
	int m_pointNum;
	
public:
	KDTree(){}
	KDTree(int k, vector<vector<double> > allpoints) :m_k(k)
	{
		m_root = new KDNode();
		m_root->m_isLeaf = false;
		m_root->m_leftNode = nullptr;
		m_root->m_rightNode = nullptr;
 
		m_pointNum = allpoints.size();
		m_points = allpoints;
		KDTreeBuild(allpoints, m_root);
	}
 
	void Insert(vector<double> newpoint);
 
	vector<vector<double> >SearchByRegion(vector<double> from, vector<double> to)const;
 
	vector<double> SearchNearestNeighbor(vector<double> goalpoint);
private:
	void KDTreeBuild(vector<vector<double>>points, KDNode* root);
	void SearchNearestByTree(vector<double> goalpoint, double&curdis, const KDNode*treeroot, vector<double>&nearestpoint);
	void SearchRecu(vector<double>from, vector<double>to, const KDNode*temp, vector<vector<double>>&nodes)const;
	double CalDistance(vector<double> point1, vector<double> point2);
};
 
#endif // !1

void KDTree::KDTreeBuild(vector<vector<double>>points, KDNode* root)
{
	int indexpart = 0, max = 0;
	vector<double>temp;
	for (st i = 0; i < m_k; i++)
	{
		temp.clear();
        for(auto var : points)
        {
            temp.push_back(var[i]);
        }
 
		double ave = accumulate(temp.begin(), temp.end(), 0.0) / m_pointNum;
		double accum = 0.0;
        for(auto var : temp)
        {
            accum += (var - ave)*(var - ave);
        }
 
		if (accum > max)
		{
			max = accum;
			indexpart = i;
		}
	}
	temp.clear();
    for(auto var : points)
    {
        temp.push_back(var[indexpart]);
    }

	sort(temp.begin(), temp.end());
	double median = temp[(temp.size()) >> 1];
	vector<vector<double>>leftpoints, rightpoints;
	for (auto var : points)
	{
		
		if (var[indexpart] < median)
			leftpoints.push_back(var);
 
		if (var[indexpart] == median)
		{
			root->m_split = indexpart + 1;
			root->m_point = var;
		}
		if (var[indexpart] > median)
			rightpoints.push_back(var);
	}
	if (leftpoints.size() == 0 && rightpoints.size() == 0)root->m_isLeaf = true;
	if (leftpoints.size() != 0)
	{
		root->m_leftNode = new KDNode();
		root->m_leftNode->m_parentNode = root;
		KDTreeBuild(leftpoints, root->m_leftNode);
	}
	if (rightpoints.size() != 0)
	{
		root->m_rightNode = new KDNode();
		root->m_rightNode->m_parentNode = root;
		KDTreeBuild(rightpoints, root->m_rightNode);
	}

	
}

void KDTree::Insert(vector<double>newpoint)
{
	if (newpoint.size() != m_k)
	{
		exit(1);
	}
	KDNode*temp = m_root;
	if (temp == nullptr)
	{
		temp = new KDNode();
		temp->m_isLeaf = true;
		temp->m_split = 1;
		temp->m_point = newpoint;
		return;
	}
	if (temp->m_isLeaf)
	{
		temp->m_isLeaf = false;
		int max = 0, partindex = 0;
		for (st i = 0; i < m_k; i++)
		{
			double delta = abs(newpoint[i] - temp->m_point[i]);
			if (delta > max)
			{
				max = delta;
				temp->m_split = i + 1;
			}
		}
	}
	while (true)
	{
		int partindex = temp->m_split - 1;
		KDNode*nextnode;
		if (newpoint[partindex] > temp->m_point[partindex])
		{
			if (temp->m_rightNode == nullptr)
			{
				temp->m_rightNode = new KDNode();
				temp->m_rightNode->m_parentNode = temp;
				temp->m_rightNode->m_isLeaf = true;
				temp->m_rightNode->m_split = 1;
				temp->m_rightNode->m_point = newpoint;
				break;
			}
			else nextnode = temp->m_rightNode;
		}
		else
		{
			if (temp->m_leftNode == nullptr)
			{
				temp->m_leftNode = new KDNode();
				temp->m_leftNode->m_parentNode = temp;
				temp->m_leftNode->m_isLeaf = true;
				temp->m_leftNode->m_split = 1;
				temp->m_leftNode->m_point = newpoint;
				break;
			}
			else nextnode = temp->m_leftNode;
		}
 
		if (nextnode->m_isLeaf)
		{
			nextnode->m_isLeaf = false;
			int max = 0, partindex = 0;
			for (st i = 0; i < m_k; i++)
			{
				double delta = abs(newpoint[i] - nextnode->m_point[i]);
				if (delta > max)
				{
					max = delta;
					nextnode->m_split = i + 1;
				}
			}
		}
		temp = nextnode;
	}
}
vector<vector<double>>KDTree::SearchByRegion(vector<double>from, vector<double>to)const
{
	vector<vector<double>>result;
	if (from.size() != m_k || to.size() != m_k)
	{
		exit(1);
	}
	for (st i = 0; i < m_k; i++)
	{
		if (from[i] > to[i])
		{
			exit(1);
		}
	}
	SearchRecu(from, to, m_root, result);
	return result;
}
 
void KDTree::SearchRecu(vector<double>from, vector<double>to, const KDNode*temp, vector<vector<double>>&nodes)const
{
	if (temp == nullptr)return;
 
	int partindex = temp->m_split - 1;
	int value = temp->m_point[partindex];
	if (from[partindex] <= value && to[partindex] >= value)
	{
		bool inregion = true;
		for (st i = 0; i < m_k; i++)
		{
			if (from[i] > temp->m_point[i] || to[i] < temp->m_point[i])
				inregion = false;
		}
		if (inregion)nodes.push_back(temp->m_point);
		SearchRecu(from, to, temp->m_leftNode, nodes);
		SearchRecu(from, to, temp->m_rightNode, nodes);
	}
	else if (value > to[partindex])
		SearchRecu(from, to, temp->m_leftNode, nodes);
 
	else if (value < from[partindex])
		SearchRecu(from, to, temp->m_rightNode, nodes);
}

double KDTree::CalDistance(vector<double> point1, vector<double> point2)
{
	if (point1.size() != point2.size())
	{
		exit(1);
	}
	double distance = 0.0;
	for (st i = 0; i < point1.size(); i++)
		distance += pow((point1[i] - point2[i]), 2);
 
	return sqrt(distance);
}
 
vector<double> KDTree::SearchNearestNeighbor(vector<double> goalpoint)
{
	vector<double>nearestpoint;
	KDNode*temp = m_root;
	while (!temp->m_isLeaf)
	{
		int partindex = temp->m_split - 1;
		
		if (temp->m_leftNode != nullptr && goalpoint[partindex] < temp->m_point[partindex])
		{
			temp = temp->m_leftNode;
		}
		else if (temp->m_rightNode != nullptr)
		{
			temp = temp->m_rightNode;
		}
	}
	nearestpoint = temp->m_point;
	
	double curdis = CalDistance(goalpoint, nearestpoint);
	bool isleft = false;
	while (temp != m_root)
	{
		isleft = (temp == temp->m_parentNode->m_leftNode);
 
		temp = temp->m_parentNode;
		if (CalDistance(goalpoint, temp->m_point) < curdis)
		{
			nearestpoint = temp->m_point;
			curdis = CalDistance(goalpoint, nearestpoint);
		}
		int partindex = temp->m_split - 1;
		if (curdis > abs(temp->m_point[partindex] - goalpoint[partindex]))
		{
			if (isleft)
			{
				SearchNearestByTree(goalpoint, curdis, temp->m_rightNode, nearestpoint);
			}
			else SearchNearestByTree(goalpoint, curdis, temp->m_leftNode, nearestpoint);
		}
	}
	return nearestpoint;
}
 
void KDTree::SearchNearestByTree(vector<double> goalpoint, double&curdis, const KDNode*treeroot, vector<double>&nearestpoint)
{
	if (treeroot == nullptr)return;
	double newdis = CalDistance(goalpoint, treeroot->m_point);
	if (newdis < curdis)
	{
		curdis = newdis;
		nearestpoint = treeroot->m_point;
	}
	SearchNearestByTree(goalpoint, curdis, treeroot->m_leftNode, nearestpoint);
	SearchNearestByTree(goalpoint, curdis, treeroot->m_rightNode, nearestpoint);
}