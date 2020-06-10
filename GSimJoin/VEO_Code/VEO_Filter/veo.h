#include "graph.h"
#include<unordered_set>
#include <map>

class VEO{
public:
	double ubound;
	map< pair<unsigned, unsigned>, unsigned long > rank;
	vector<vector<unsigned long>> rankList;
	vector<vector<vector<unsigned>>> bucket;
	VEO(double threshold)
	{
		ubound = double((double)(200.0/threshold) -1.0);
		cout << "ubound: " << ubound << endl;
	}
	bool indexFilter(Graph &g2, Graph &g1, int index2, int index1, int mode, bool isBucket, int no_of_buckets, long unsigned &indexCount, long unsigned &partitionCount, double threshold);
	void ranking(vector<Graph> &graphDataset);
	void buildPrefix(vector<Graph> &graphDataset, int type, bool isBucket, int b);
	bool mismatchingFilter(Graph &g1, Graph &g2, double &c, double threshold);
	void sortGraphDataset(vector<Graph> &graphDataset);
	double computeSimilarity(Graph &g1, Graph &g2, double &commonV);
	void sortVertexEdge(vector<Graph> &graphDataset);
	void printlist(int i);
	void index(vector<Graph> &graphDataset, int type, bool isBucket, int b);
};
