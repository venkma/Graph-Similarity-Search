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
void applyFilters(vector<Graph> &graph_dataset, Graph &query_graph, unordered_set<unsigned> &candidate_pairs, int dataset_size, int choice, double simScore_threshold, string res_dir)
{
	// count of pruned graphs after each filter
	long long loose_filter_count = 0, strong_filter_count = 0, candidate_graph_count = 0;

	// Loose Size filter
	simScore_threshold = simScore_threshold/100.0;

	unsigned qg_index = lower_bound(graph_dataset.begin(), graph_dataset.end(), query_graph, GraphComparator()) - graph_dataset.begin();
	
	double num_shingles_qg = max(0,(int)query_graph.vertexCount - SHINGLESIZE + 1);

	for(int g2 = qg_index - 1; g2 >= 0; g2--)
	{
		// to ignore negative values: max(0,val)
		double num_shingles_g2 = max(0,(int)graph_dataset[g2].vertexCount - SHINGLESIZE + 1);

		if(simScore_threshold <= (num_shingles_g2/num_shingles_qg))
		{
			loose_filter_count++;
			if(choice > 1)
			{
				// Common Vertex filter
				int common_vertices = commonVertices(query_graph, graph_dataset[g2]);
				double possible_shingles = max(0,(int)common_vertices - SHINGLESIZE + 1); // to ignore negative values: max(0,val) 
				
				double compute_threshold=0;

				if((num_shingles_g2 + num_shingles_qg - possible_shingles) != 0)
					compute_threshold = (possible_shingles/double(num_shingles_g2 + num_shingles_qg - possible_shingles));

				if(simScore_threshold <= compute_threshold)
				{
					strong_filter_count++;
					// Adding graph to pruned graph data structure
					candidate_pairs.insert(g2);
				}
			}
			else
			{
				// Adding graph to pruned graph data structure
				candidate_pairs.insert(g2);
			}
		}
		else
		{
			// Since Graph g2 is not satisfying Loose Size filter 
			break;
		}
	}

	for(int g2 = qg_index + 1; g2 < graph_dataset.size(); g2++)
	{
		// to ignore negative values: max(0,val)
		double num_shingles_g2 = max(0,(int)graph_dataset[g2].vertexCount - SHINGLESIZE + 1);

		if(simScore_threshold <= (num_shingles_qg/num_shingles_g2))
		{
			loose_filter_count++;
			if(choice > 1)
			{
				// Common Vertex filter
				int common_vertices = commonVertices(query_graph, graph_dataset[g2]);
				double possible_shingles = max(0,(int)common_vertices - SHINGLESIZE + 1); // to ignore negative values: max(0,val) 
				
				double compute_threshold=0;

				if((num_shingles_g2 + num_shingles_qg - possible_shingles) != 0)
					compute_threshold = (possible_shingles/double(num_shingles_g2 + num_shingles_qg - possible_shingles));

				if(simScore_threshold <= compute_threshold)
				{
					strong_filter_count++;
					// Adding graph to pruned graph data structure
					candidate_pairs.insert(g2);
				}
			}
			else
			{
				// Adding graph to pruned graph data structure
				candidate_pairs.insert(g2);
			}
		}
		else
		{
			// Since Graph g2 is not satisfying Loose Size filter 
			break;
		}
	}
	
	// Writing statistics to the stat file
	ofstream stat_file("./"+res_dir+"/stat_file.txt",ios::app);
	stat_file << "Loose Size Filter count: " << loose_filter_count << endl;
	cout << "Loose size filter candidate pair count: " << loose_filter_count << endl;
	if(choice > 1)
	{
		stat_file << "Common Vertex Filter count: " << strong_filter_count << endl;
		cout << "Common Vertex filter candidates pair count: " << strong_filter_count << endl;
	}
	cout << "No. of Candidate Graphs: " << candidate_pairs.size() << endl << endl;
	stat_file.close();
}

// To Preprocess the pruned graph pairs 
void preProcess(vector<Graph> &graph_dataset, Graph &query_graph, unordered_set<unsigned> &candidate_pairs)
{
	query_graph.sortGraph();
	query_graph.walkAlgorithm();
	query_graph.computeShingles();
	query_graph.computeMinHashes();
	for(auto g_iter = candidate_pairs.begin(); g_iter != candidate_pairs.end(); g_iter++)
	{
		graph_dataset[*g_iter].sortGraph();
		graph_dataset[*g_iter].walkAlgorithm();
		graph_dataset[*g_iter].computeShingles();
		graph_dataset[*g_iter].computeMinHashes();	 	
	}
}

// To prune graphs using Banding Technique filter
void bandingTech(vector<Graph> &graph_dataset, Graph &query_graph, unordered_set<unsigned> &candidate_pairs, unordered_set<unsigned> &banding_pairs, int BANDS, int ROWS)
{
	unsigned hashVal;
	for(int b = 0; b < BANDS; b++)
	{
		// Constructing buckets with similar graphs in one bucket using minhashes
		hashVal = boost::hash_range(query_graph.minhashes.begin() + b*ROWS, query_graph.minhashes.begin() + (b+1)*ROWS);
		for(auto cand_graph_ind = candidate_pairs.begin(); cand_graph_ind != candidate_pairs.end(); cand_graph_ind++)
		{
			if(graph_dataset[*cand_graph_ind].minhashes.size()!=0)
			{
				if(hashVal == boost::hash_range(graph_dataset[*cand_graph_ind].minhashes.begin() + b*ROWS, graph_dataset[*cand_graph_ind].minhashes.begin() + (b+1)*ROWS))
					banding_pairs.insert(*cand_graph_ind);
			}
		}
	}
}
