#include "seq_sim.h"
#include "rollingHash.h"
#include "minhash.h"

// computes Sequence Similarity of 2 input graphs
double computeSimilarity(Graph &g1, Graph &g2)
{
	double commonShingles = 0;

	for(int i=0,j=0; i < g1.shingles.size() && j < g2.shingles.size(); )
	{
		if(g1.shingles[i] < g2.shingles[j])
			i++;
		else if(g1.shingles[i] > g2.shingles[j])
			j++;
		else
		{
			i++;
			j++;
			commonShingles++;
		}

	}
	double simScore = 100.0*(commonShingles / (g1.shingles.size()+g2.shingles.size()-commonShingles));
	return simScore;
}

// To get the count of common vertices between 2 graphs
int commonVertices(Graph &g1, Graph &g2)
{
	int common_vts = 0;
	for(int g1_vtx=0,g2_vtx=0; g1_vtx < g1.vertices.size() && g2_vtx < g2.vertices.size(); )
	{
		if(g1.vertices[g1_vtx].vid < g2.vertices[g2_vtx].vid)
		{
			g1_vtx++;
		}
		else if(g1.vertices[g1_vtx].vid > g2.vertices[g2_vtx].vid)
		{
			g2_vtx++;
		}
		else
		{
			g1_vtx++; 
			g2_vtx++;
			common_vts++;
		}
	}
	return common_vts;
}

// To apply Loose size and Strong size filter to input graph dataset
void applyFilters(vector<Graph> &graph_dataset, vector<bool> &candidate_graphs, unordered_map<unsigned, unordered_set<unsigned> > &candidate_pairs, int dataset_size, int choice, double simScore_threshold, long long &loose_filter_count, long long &strong_filter_count, long long &candidate_graph_count)
{
	// Loose Size filter
	simScore_threshold = simScore_threshold/100.0;

	for(int g1=1; g1 < graph_dataset.size(); g1++)
	{
		for(int g2 = g1-1; g2 >= 0; g2--)
		{
			// to ignore negative values: max(0,val)
			double num_shingles_g1 = max(0,(int)graph_dataset[g1].vertexCount - SHINGLESIZE + 1);
			double num_shingles_g2 = max(0,(int)graph_dataset[g2].vertexCount - SHINGLESIZE + 1);
			
			if(simScore_threshold <= (num_shingles_g2/num_shingles_g1))
			{
				loose_filter_count++;
				if(choice > 1)
				{
					// Common Vertex filter
					int common_vertices = commonVertices(graph_dataset[g1], graph_dataset[g2]);
					double possible_shingles = max(0,(int)common_vertices - SHINGLESIZE + 1); // to ignore negative values: max(0,val) 

					double compute_threshold=0;
					if((num_shingles_g2 + num_shingles_g1 - possible_shingles) != 0)
						compute_threshold = (possible_shingles/double(num_shingles_g2 + num_shingles_g1 - possible_shingles));

					if(simScore_threshold <= compute_threshold)
					{
						strong_filter_count++;
						// Adding graph to pruned graph data structure
						if(!candidate_graphs[g2]){
							candidate_graph_count++;
							candidate_graphs[g2] = true;
						}
						if(!candidate_graphs[g1]){
							candidate_graph_count++;
							candidate_graphs[g1] = true;
						}
						candidate_pairs[g1].insert(g2);
					}
				}
				else
				{
					// Adding graph to pruned graph data structure
					if(!candidate_graphs[g2]){
						candidate_graph_count++;
						candidate_graphs[g2] = true;
					}
					if(!candidate_graphs[g1]){
						candidate_graph_count++;
						candidate_graphs[g1] = true;
					}
					candidate_pairs[g1].insert(g2);
				}
			}
			else
			{
				// Since Graph g2 is not satisfying Loose Size filter 
				break;
			}
		}
	}
}

// To Preprocess the pruned graph pairs 
void preProcess(vector<Graph> &graph_dataset, vector<bool> &candidate_graphs)
{
	for(int g_iter=0; g_iter < graph_dataset.size(); g_iter++)
	{
		if(candidate_graphs[g_iter])
		{
			graph_dataset[g_iter].sortGraph();
			graph_dataset[g_iter].walkAlgorithm();
			graph_dataset[g_iter].computeShingles();
			graph_dataset[g_iter].computeMinHashes();
	 	}
	}
}

// To prune graphs using Banding Technique filter
void bandingTech(vector<Graph> &graph_dataset, vector<bool> &candidate_graphs, unordered_map<unsigned, unordered_set<unsigned> > &banding_candidate_pairs, int BANDS, int ROWS)
{
	unsigned hashVal;
	for(int b = 0; b < BANDS; b++)
	{
		// Constructing buckets with similar graphs in one bucket using minhashes
		unordered_map<unsigned, vector<unsigned> > buckets;
		for(int g_index = 0; g_index < graph_dataset.size(); g_index++)
		{
			if(graph_dataset[g_index].minhashes.size()!=0 && candidate_graphs[g_index])
			{
				hashVal = boost::hash_range(graph_dataset[g_index].minhashes.begin() + b*ROWS, graph_dataset[g_index].minhashes.begin() + (b+1)*ROWS);
				buckets[hashVal].push_back(g_index);
			}
		}
		// Extracting candidate pairs from buckets
		for(auto bucket_itr = buckets.begin(); bucket_itr!=buckets.end(); bucket_itr++)
		{
			// All graphs in one bucket are similar and can form a candidate pair among each other
			for(int i = 0; i < bucket_itr->second.size(); i++)
			{
				for(int j = i+1; j < bucket_itr->second.size(); j++)
				{
					// Adding each pair from bucket to banding candidate pairs data structure
					banding_candidate_pairs[bucket_itr->second[i]].insert(bucket_itr->second[j]);
				}
			}
		}
	}
}