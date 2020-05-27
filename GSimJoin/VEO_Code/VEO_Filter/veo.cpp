#include "veo.h"
#include <cmath>
#include <chrono>

bool freqComp(pair<unsigned, unsigned> v1, pair<unsigned, unsigned> v2)
{ 
    return (v1.second > v2.second); 
}

// Edge comparator
bool edgeComp(pair<unsigned, unsigned> &a, pair<unsigned, unsigned> &b)
{
	if(a.first == b.first)
		return a.second < b.second;
	return a.first < b.first;
}

// Intersection of vertex lists of 2 graphs
double intersection_vertices(vector<unsigned> &s1, vector<unsigned> &s2)
{
	unsigned s1_iter = 0;
	unsigned s2_iter = 0;
	double common = 0;

	while(s1_iter < s1.size() && s2_iter < s2.size())
	{
		if(s1[s1_iter] < s2[s2_iter])
			s1_iter++;
		else if(s1[s1_iter] > s2[s2_iter])
			s2_iter++;
		else
		{
			
			s1_iter++;
			s2_iter++;
			common++;		
		}
	}
	return common;
}

// Intersection of edge lists of 2 graphs
double intersection_edges(vector<pair<unsigned, unsigned>> &s1, vector<pair<unsigned, unsigned>> &s2)
{
	unsigned s1_iter = 0;
	unsigned s2_iter = 0;
	double common = 0;

	for(auto s1_iter = s1.begin(), s2_iter = s2.begin(); s1_iter != s1.end() && s2_iter != s2.end(); )
	{
		if(edgeComp(*s1_iter,*s2_iter))
			s1_iter++;
		else if(edgeComp(*s2_iter,*s1_iter))
			s2_iter++;
		else
		{
			
			s1_iter++;
			s2_iter++;
			common++;		
		}
	}
	return common;
}

// VEO Similarity computation
double VEO:: computeSimilarity(Graph &g1, Graph &g2, double &commonV)
{
	if(commonV == 0)
		commonV = intersection_vertices(g1.vertices, g2.vertices);
	commonV+=intersection_edges(g1.edges, g2.edges);
	
	double simScore = (double)(200.0*(commonV)/(double)(g1.vertexCount+g2.vertexCount+g1.edgeCount+g2.edgeCount));
	return simScore;
}

// Sorts vertex and edge set of graph dataset
void VEO:: sortVertexEdge(vector<Graph> &graph_dataset)
{
	for(int i = 0; i < graph_dataset.size(); i++)
	{
		sort(graph_dataset[i].vertices.begin(), graph_dataset[i].vertices.end());
		sort(graph_dataset[i].edges.begin(), graph_dataset[i].edges.end());
	}
}

void VEO:: printlist(int k)
{
	for(int i = 0; i < rankList[k].size(); i++)
		cout << i  << ": "<< rankList[k][i] << endl;
}

//globaly ranks vertices and edges together
void VEO:: ranking(vector<Graph> &graph_dataset) 
{
	// traversing the graph-dataset
	for(int g_ind = 0; g_ind < graph_dataset.size(); g_ind++)
	{
		// traversing the vertex-set
		for(int vtx_ind = 0; vtx_ind < graph_dataset[g_ind].vertices.size(); vtx_ind++)
		{
			if(rank.find(graph_dataset[g_ind].vertices[vtx_ind]) != rank.end())
				rank[graph_dataset[g_ind].vertices[vtx_ind]]++;
			else
				rank[graph_dataset[g_ind].vertices[vtx_ind]] = 0;
		}
		// traversing the edge-set
		for(int edge_ind = 0; edge_ind < graph_dataset[g_ind].edges.size(); edge_ind++)
		{
			string edge_str = to_string(graph_dataset[g_ind].edges[edge_ind].first) + to_string(graph_dataset[g_ind].edges[edge_ind].second);
			if(rank.find(stoll(edge_str)) != rank.end())
				rank[stoll(edge_str)]++;
			else
				rank[stoll(edge_str)] = 0;
		}
	}

	vector<pair<unsigned long long int, unsigned>> freqList;
	copy(rank.begin(), rank.end(), back_inserter(freqList));
	sort(freqList.begin(), freqList.end(), freqComp);

	// ranking each vertex and each edge
	unsigned long r = 1;
	for(auto entry = freqList.begin(); entry!=freqList.end(); entry++)
	{
		rank[entry->first] = r;
		r++;
	}
}

void VEO:: buildPrefix(vector<Graph> &graph_dataset, int mode, bool isBucket, int no_of_buckets)
{
	rankList.resize(graph_dataset.size());
	if(isBucket)
		bucket.resize(graph_dataset.size());
	
	for(int g_ind = 0; g_ind < graph_dataset.size(); g_ind++)
	{
		unsigned prefixLength = 0;
	
		double graph_size = graph_dataset[g_ind].vertexCount + graph_dataset[g_ind].edgeCount; // size of graph g_ind

		if(mode == 3)
		{
			double invUbound = (double)1.0/ubound; // inverse of ubound
			prefixLength = 1 +(unsigned)(ceil((graph_size*(double)(1.0-invUbound)))); // Prefix Length
		}
		else
		{
			prefixLength = graph_size;
		}
		
		// Constructing the rank-list 

		// Putting vertex and edge ranks together
		vector<unsigned> graph_ranks;
		for(int vtx_ind = 0; vtx_ind < graph_dataset[g_ind].vertices.size(); vtx_ind++)
			graph_ranks.push_back(rank[graph_dataset[g_ind].vertices[vtx_ind]]);
		
		for(int edge_ind = 0; edge_ind < graph_dataset[g_ind].edges.size(); edge_ind++)
		{
			string edge_str = to_string(graph_dataset[g_ind].edges[edge_ind].first) + to_string(graph_dataset[g_ind].edges[edge_ind].second);
			graph_ranks.push_back(rank[stoll(edge_str)]);
		}

		// sort the ranks of the graph in descending order  
		sort(graph_ranks.begin(), graph_ranks.end(), greater <>());

		// crop graph's rank-list upto prefix-length
		for(int pref = 0; pref < prefixLength; pref++)
			rankList[g_ind].push_back(graph_ranks[pref]);

		if(isBucket)
		{
			bucket[g_ind].resize(no_of_buckets);
			for(int buck_ind = 0; buck_ind < no_of_buckets; buck_ind++)
				bucket[g_ind][buck_ind].resize(graph_size, 0);

			vector<unsigned> sumBucket(no_of_buckets, 0);

			// traversing graph's rank-list in ascending order 
			for(int grank_ind = graph_size-1; grank_ind >= 0; grank_ind--)
			{
				sumBucket[graph_ranks[grank_ind]%no_of_buckets]++;
				for(int buck_ind = 0; buck_ind < no_of_buckets; buck_ind++)
				{
					bucket[g_ind][buck_ind][grank_ind] = sumBucket[buck_ind];
				}
			}
		}
	}
}

// index each input graphs in dataset
void VEO:: index(vector<Graph> &graph_dataset, int mode, bool isBucket, int no_of_buckets)
{	
	ranking(graph_dataset); // ranking vertices and edges together
	buildPrefix(graph_dataset, mode, isBucket, no_of_buckets);
}

// Applies index filter on Graph g1 and g2
bool VEO:: indexFilter(Graph &g1, Graph &g2, int index1, int index2, int mode, bool isBucket, int no_of_buckets, long unsigned &indexCount, long unsigned &partitionCount, double threshold)
{
	unsigned size1 = g1.vertexCount + g1.edgeCount; // Size of Graph g1
	unsigned size2 = g2.vertexCount + g2.edgeCount; // Size of Graph g2

	unsigned prefix1;
	unsigned prefix2;

	if(mode == 3)
	{
		prefix1 = rankList[index1].size(); // prefix-length of graph g1
		prefix2 = rankList[index2].size(); // prefix-length of graph g2
	}
	if(mode == 4)
	{
		unsigned common = (unsigned)floor(((double)(threshold/200.0))*(double)(size1+size2));
		prefix1 = size1 - common + 1; // prefix-length of graph g1
		prefix2 = size2 - common + 1; // prefix-length of graph g2
	}

	unsigned start1 = 0;
	unsigned start2 = 0;
	bool out = true;
	long double commonTotal=0;
	
	while(start1 < prefix1 && start2 < prefix2)
	{
		if(rankList[index1][start1] == rankList[index2][start2] && isBucket)
		{
			out = false;
			start1++;
			start2++;
			commonTotal++;		
		}
		else if(rankList[index1][start1] == rankList[index2][start2] && !isBucket)
		{
			out = false;
			break;
		}
		else if(rankList[index1][start1] > rankList[index2][start2])
			start1++;
		else
			start2++;
	}
	if(out)
		return out;
	indexCount++;
	if(isBucket)
	{
		for(int i = 0; i < no_of_buckets; i++)
			commonTotal += (long double)min(bucket[index1][i][start1], bucket[index2][i][start2]);

		long double sizeSum = (long double)(g1.vertexCount + g1.edgeCount + g2.vertexCount + g2.edgeCount);
		long double veoEstimate =(long double)200.0*((long double)commonTotal/(long double)(sizeSum));
		if(veoEstimate < (long double)threshold)
			out = true;
		else
		{
			out = false;
			partitionCount++;
		}
	}
	return out;
}

// MisMatching Filter
bool VEO:: mismatchingFilter(Graph &g1, Graph &g2, double &common, double threshold)
{
	int index1 = 0;
	int index2 = 0;
	unsigned degreeSum12 = 0;
	unsigned degreeSum21 = 0;
	while(index1 < g1.vertexCount && index2 < g2.vertexCount)
	{
		if(g1.vertices[index1] == g2.vertices[index2])
		{
			common++;
			index1++;
			index2++;
		}
		else if(g1.vertices[index1] > g2.vertices[index2])
		{
			degreeSum21 += g2.degrees[g2.vid_to_ind[g2.vertices[index2]]];
			index2++;
		}
		else
		{
			degreeSum12 += g1.degrees[g1.vid_to_ind[g1.vertices[index1]]];
			index1++;
		}
	}

	double ES = (double)min((g1.edgeCount - degreeSum12), (g2.edgeCount - degreeSum21) );
	ES = max(0.0,ES);
	double simEstimate = (double)(200.0*(double)((common + ES)/(double)(g1.edgeCount + g1.vertexCount + g2.edgeCount + g2.vertexCount)));
	return (simEstimate <= threshold);
}
