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
	for(int g1_vtx=0, g2_vtx=0; g1_vtx < g1.vertices.size() && g2_vtx < g2.vertices.size(); )
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

struct GraphComparator
{
	// Compare 2 Player objects using name
	bool operator ()(const Graph &g1,const Graph &g2)
	{
		if(g1.vertexCount == g2.vertexCount)
			return g1.gid < g2.gid;
		return g1.vertexCount < g2.vertexCount;
	}
};

// To apply Loose size and Strong size filter to input graph dataset
void applyFilters(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, vector<bool> &candidate_graphs_R, vector<bool> &candidate_graphs_S, unordered_map<unsigned, unordered_set<unsigned> > &candidate_pairs, int dataset_size_R, int dataset_size_S, int choice, double simScore_threshold, long long &loose_filter_count, long long &strong_filter_count, long long &candidate_graph_count_R, long long &candidate_graph_count_S)
{
	simScore_threshold = simScore_threshold/100.0;
	
	for(int gR = 0; gR < graph_dataset_R.size(); gR++)
	{
		// correct position for graph gR in graph dataset S
		unsigned gR_index = lower_bound(graph_dataset_S.begin(), graph_dataset_S.end(), graph_dataset_R[gR], GraphComparator()) - graph_dataset_S.begin();

		// no. of shingles in graph gR
		double num_shingles_gR = max(0,(int)graph_dataset_R[gR].vertexCount - SHINGLESIZE + 1); // to ignore negative values: max(0,val)

		for(int gS = gR_index; gS >= 0; gS--)
		{
			// no. of shingles in graph gS
			double num_shingles_gS = max(0,(int)graph_dataset_S[gS].vertexCount - SHINGLESIZE + 1);

			// Loose size Filter
			if(simScore_threshold <= (num_shingles_gS/num_shingles_gR))
			{
				loose_filter_count++;
				if(choice > 1)
				{
					// Common Vertex filter
					int common_vertices = commonVertices(graph_dataset_R[gR], graph_dataset_S[gS]);
					double possible_shingles = max(0,(int)common_vertices - SHINGLESIZE + 1); // to ignore negative values: max(0,val) 
					
					double compute_threshold = 0;

					if((num_shingles_gS + num_shingles_gR - possible_shingles) != 0)
						compute_threshold = (possible_shingles/double(num_shingles_gS + num_shingles_gR - possible_shingles));

					if(simScore_threshold <= compute_threshold)
					{
						strong_filter_count++;
						// Adding graph to pruned graph data structure
						if(!candidate_graphs_S[gS]){
							candidate_graph_count_S++;
							candidate_graphs_S[gS] = true;
						}
						if(!candidate_graphs_R[gR]){
							candidate_graph_count_R++;
							candidate_graphs_R[gR] = true;
						}
						candidate_pairs[gR].insert(gS);
					}
				}
				else
				{
					// Adding graph to pruned graph data structure
					if(!candidate_graphs_S[gS]){
						candidate_graph_count_S++;
						candidate_graphs_S[gS] = true;
					}
					if(!candidate_graphs_R[gR]){
						candidate_graph_count_R++;
						candidate_graphs_R[gR] = true;
					}
					candidate_pairs[gR].insert(gS);
				}
			}
			else
			{
				// Since Graph gS is not satisfying Loose Size filter 
				break;
			}
		}

		for(int gS = gR_index+1; gS < graph_dataset_S.size(); gS++)
		{
			// to ignore negative values: max(0,val)
			double num_shingles_gS = max(0,(int)graph_dataset_S[gS].vertexCount - SHINGLESIZE + 1);

			// Loose size Filter
			if(simScore_threshold <= (num_shingles_gR/num_shingles_gS))
			{
				loose_filter_count++;
				if(choice > 1)
				{
					// Common Vertex filter
					int common_vertices = commonVertices(graph_dataset_R[gR], graph_dataset_S[gS]);
					double possible_shingles = max(0,(int)common_vertices - SHINGLESIZE + 1); // to ignore negative values: max(0,val) 
					
					double compute_threshold = 0;

					if((num_shingles_gS + num_shingles_gR - possible_shingles) != 0)
						compute_threshold = (possible_shingles/double(num_shingles_gS + num_shingles_gR - possible_shingles));

					if(simScore_threshold <= compute_threshold)
					{
						strong_filter_count++;
						// Adding graph to pruned graph data structure
						if(!candidate_graphs_S[gS]){
							candidate_graph_count_S++;
							candidate_graphs_S[gS] = true;
						}
						if(!candidate_graphs_R[gR]){
							candidate_graph_count_R++;
							candidate_graphs_R[gR] = true;
						}
						candidate_pairs[gR].insert(gS);
					}
				}
				else
				{
					// Adding graph to pruned graph data structure
					if(!candidate_graphs_S[gS]){
						candidate_graph_count_S++;
						candidate_graphs_S[gS] = true;
					}
					if(!candidate_graphs_R[gR]){
						candidate_graph_count_R++;
						candidate_graphs_R[gR] = true;
					}
					candidate_pairs[gR].insert(gS);
				}
			}
			else
			{
				// Since Graph gS is not satisfying Loose Size filter 
				break;
			}
		}
	}	
}

// To Preprocess the pruned graph pairs 
void preProcess(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, vector<bool> &candidate_graphs_R, vector<bool> &candidate_graphs_S)
{
	for(int g_iter=0; g_iter < graph_dataset_R.size(); g_iter++)
	{
		if(candidate_graphs_R[g_iter])
		{

			graph_dataset_R[g_iter].sortGraph();
			graph_dataset_R[g_iter].walkAlgorithm();
			graph_dataset_R[g_iter].computeShingles();
			graph_dataset_R[g_iter].computeMinHashes();
	 	}
	}
	for(int g_iter=0; g_iter < graph_dataset_S.size(); g_iter++)
	{
		if(candidate_graphs_S[g_iter])
		{
			graph_dataset_S[g_iter].sortGraph();
			graph_dataset_S[g_iter].walkAlgorithm();
			graph_dataset_S[g_iter].computeShingles();
			graph_dataset_S[g_iter].computeMinHashes();
	 	}
	}
}

// To prune graphs using Banding Technique filter
void bandingTech(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, unordered_map<unsigned, unordered_set<unsigned> > &candidate_pairs, unordered_map<unsigned, unordered_set<unsigned> > &banding_pairs, int BANDS, int ROWS, long long &banding_pair_count)
{
	unsigned hashVal_R, hashVal_S;
	for(int b = 0; b < BANDS; b++)
	{
		for(auto g_iter_R = candidate_pairs.begin(); g_iter_R != candidate_pairs.end(); g_iter_R++)
		{
			// Constructing buckets with similar graphs in one bucket using minhashes
			hashVal_R = boost::hash_range(graph_dataset_R[g_iter_R->first].minhashes.begin() + b*ROWS, graph_dataset_R[g_iter_R->first].minhashes.begin() + (b+1)*ROWS);
			for(auto g_iter_S = candidate_pairs[g_iter_R->first].begin(); g_iter_S != candidate_pairs[g_iter_R->first].end(); g_iter_S++)
			{
				if(graph_dataset_S[*g_iter_S].minhashes.size()!=0)
				{
					hashVal_S = boost::hash_range(graph_dataset_S[*g_iter_S].minhashes.begin() + b*ROWS, graph_dataset_S[*g_iter_S].minhashes.begin() + (b+1)*ROWS);
					if(banding_pairs[g_iter_R->first].find(*g_iter_S) == banding_pairs[g_iter_R->first].end() && hashVal_R == hashVal_S)
					{
						banding_pairs[g_iter_R->first].insert(*g_iter_S);
						banding_pair_count++;
					}
				}
			}
		}
	}
}