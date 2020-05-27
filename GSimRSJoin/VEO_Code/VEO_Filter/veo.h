#include "graph.h"
#include<unordered_set>
#include <unordered_map>

class VEO{
public:
	double ubound;
	unordered_map<unsigned long long int, unsigned long> rank;
	vector<vector<unsigned long>> rankList_R;
	vector<vector<unsigned long>> rankList_S;
	vector<vector<vector<unsigned>>> bucket_R;
	vector<vector<vector<unsigned>>> bucket_S;
	VEO(double threshold)
	{
		ubound = double((double)(200.0/threshold) -1.0);
		cout << "ubound: " << ubound << endl;
	}
	bool indexFilter(Graph &g1, Graph &g2, int index1, int index2, int mode, bool isBucket, int no_of_buckets, unsigned long long &indexCount, unsigned long long &partitionCount, double threshold);
	void ranking(vector<Graph> &graphDataset_R, vector<Graph> &graphDataset_S);
	void buildPrefix(vector<Graph> &graphDataset_R, vector<Graph> &graphDataset_S, int type, bool isBucket, int b);
	bool mismatchingFilter(Graph &g1, Graph &g2, double &c, double threshold);
	void sortGraphDataset(vector<Graph> &graphDataset);
	double computeSimilarity(Graph &g1, Graph &g2, double &commonV);
	void printlist(int i);
	void index(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, int mode, bool isBucket, int no_of_buckets);
};
