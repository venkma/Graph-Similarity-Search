#include "graph.h"
#include<unordered_set>
#include <unordered_map>

class VEO{
public:
	double ubound;
	unordered_map<unsigned long long int, unsigned long> rank;
	vector<vector<unsigned long>> rankList;
	vector<unsigned long> rankList_qg;
	vector<vector<vector<unsigned>>> bucket;
	vector<vector<unsigned>> bucket_qg;
	VEO(double threshold)
	{
		ubound = double((double)(200.0/threshold) -1.0);
		cout << "ubound: " << ubound << endl;
	}
	bool indexFilter(Graph &g1, Graph &query_graph, int index1, int mode, bool isBucket, int no_of_buckets, long unsigned &indexCount, long unsigned &partitionCount, double threshold);
	void ranking(vector<Graph> &graphDataset, Graph &query_graph);
	void buildPrefix(vector<Graph> &graphDataset, Graph &query_graph, int type, bool isBucket, int b);
	bool mismatchingFilter(Graph &g1, Graph &query_graph, double &c, double threshold);
	void sortGraphDataset(vector<Graph> &graphDataset);
	double computeSimilarity(Graph &g1, Graph &g2, double &commonV);
	void printlist(int i);
	void index(vector<Graph> &graph_dataset, Graph &query_graph, int mode, bool isBucket, int no_of_buckets);
};
