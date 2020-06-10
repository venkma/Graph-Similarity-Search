#include "common.h"
#include "seq_sim.h"

// For parsing the input graph dataset and query graph
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset, int &dataset_size, Graph &query_graph, int query_index);

// loads the pageranks(quality) for given graph dataset
void loadPageRanks(ifstream &pagerank_file, vector<Graph> &graph_dataset, int &dataset_size, Graph &query_graph, int query_index);

// vertex comparator based on vertex ids
bool vertexComp2(Vertex &v1, Vertex &v2);

// final sequence similarity computation of pruned pairs
void computeSequenceSimilarity(vector<Graph> &graph_dataset, Graph &query_graph, unordered_set<unsigned> &final_cand_graphs, vector<int> &global_score_freq, double simScore_threshold, vector<pair<unsigned, double>> &g_res, unsigned long long &simPairCount);

// graph comparator for sorting graph dataset
bool graphComp(Graph &g1, Graph &g2);

// Displays the memory used by the program(in MB)
double memoryUsage();

// Returns the time from start to end in seconds
unsigned long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end); 

// prints correct usage of program in case of an error
void usage();

int SHINGLESIZE;

// $ ./naive inp-file pagerank-file SHINGLESIZE simScore_threshold dataset-size query_index res-dir

// $ ./filter inp_file pagerank_file SHINGLESIZE choice simScore_threshold [BANDS] [ROWS] dataset-size query_index res-dir
//                                                           			   |____________|
//							                        			            for choice 3

int main(int argc, char const *argv[])
{
	// Verifying args
	if(argc < 9)
		usage();

	int choice = stoi(argv[4]); // Choice of filter

	if(choice==1 || choice==2)
		if(argc!=9)
			usage();
	else if(choice == 3)
		if(argc!=11)
			usage();
	
	SHINGLESIZE = stoi(argv[3]); // size of each shingle
	double simScore_threshold = stod(argv[5]); // threshold to write only those graph pairs to all_graph_file.txt
	int dataset_size = stoi(argv[argc-3]); // dataset-size
	int query_index = stoi(argv[argc-2]); // dataset-size
	const string res_dir = argv[argc-1]; // directory in which all stat files would be stored
	mkdir(res_dir.c_str(),0777);

	vector<Graph> graph_dataset; // Input graph dataset
	Graph query_graph; 
	unsigned long long simPairCount = 0; // count of allsimilar pairs
	long long loose_filter_count = 0, strong_filter_count = 0, candidate_graph_count = 0, banding_pair_count = 0; // count of pruned graphs after each filter
	vector<pair<unsigned, double>> g_res; // stores all the similar graphs 

	unordered_set<unsigned> candidate_pairs; // candidate pairs after filters 1 & 2
	unordered_set<unsigned> banding_pairs; // candidate pairs after banding technique

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr<<"Unable to open the given input file"<<endl;
		exit(0);
	}
	// Parsing Graph Dataset
	parseGraphDataset(dataset_file, graph_dataset, dataset_size, query_graph, query_index); 
	cout<< "parsing is done..." <<endl; 
	
	cout << "GSimSearch: Sequence Similarity(filters)" << endl;
	cout << "Dataset size: " << dataset_size << endl;
	cout << "Shingle Size: " << SHINGLESIZE << endl;
	cout << "Choice of filter: " << choice << endl;
	cout << "Threshold: " << simScore_threshold << endl;
	cout << "Input file: " << argv[1] <<endl;
	cout << "Results Directory: " << res_dir <<endl;

	// Writing statistics to the stat file
	ofstream stat_file("./"+res_dir+"/stat_file.txt");
	stat_file << "GSimSearch: Sequence Similarity(filters)" << endl;
	stat_file << "Dataset size: " << dataset_size << endl;
	stat_file << "Shingle size: " << SHINGLESIZE << endl;
	stat_file << "Similarity Score Threshold: " << simScore_threshold << endl;
	stat_file << "Filter choice: " << choice << endl;

	int BANDS,ROWS;
	if(choice == 3)
	{
		BANDS = stoi(argv[6]);
		ROWS = stoi(argv[7]);
		cout << "Bands: " << BANDS <<endl;
		cout << "Rows: " << ROWS <<endl;
		stat_file << "No. of Bands: " << BANDS << endl;
		stat_file << "No. of Rows: " << ROWS << endl;
	}

	stat_file.close();

	// Parsing Pageranks
	ifstream pagerank_file(argv[2]);
	if(!pagerank_file.is_open())
	{
		cerr << "Unable to open the pagerank file." << endl;
		exit(0);
	}
	// Pageranks must be stored in same order of graphs as the input graph dataset file
	loadPageRanks(pagerank_file, graph_dataset, dataset_size, query_graph, query_index);
	cout<<"pagerank loading is done..."<<endl;

	// sort each graph's vertex-set based on vertex ids 
	for(int g_ind = 0; g_ind < graph_dataset.size(); g_ind++)
	{
		sort(graph_dataset[g_ind].vertices.begin(), graph_dataset[g_ind].vertices.end(), vertexComp2);
		for(int vtx_ind = 0; vtx_ind < graph_dataset[g_ind].vertexCount; vtx_ind++)
			graph_dataset[g_ind].vid_to_ind[graph_dataset[g_ind].vertices[vtx_ind].vid] = vtx_ind;
	}
	// sort query graph's vertex-set based on vertex ids 
	sort(query_graph.vertices.begin(), query_graph.vertices.end(), vertexComp2);
	for(int vtx_ind = 0; vtx_ind < query_graph.vertexCount; vtx_ind++)
		query_graph.vid_to_ind[query_graph.vertices[vtx_ind].vid] = vtx_ind;

	// timestamping start time
	chrono::high_resolution_clock::time_point cl0 = chrono::high_resolution_clock::now();
	
	// Applies Loose size and Strong size filter to input graph dataset
	applyFilters(graph_dataset, query_graph, candidate_pairs, dataset_size, choice, simScore_threshold, loose_filter_count, strong_filter_count, candidate_graph_count);
	cout << "filters are done" <<endl;
	
	// Preprocesses the pruned graph pairs 
	preProcess(graph_dataset, query_graph, candidate_pairs);
	cout<<"preprocessing part is done..."<<endl;

	// Freq of simScore with range of 1% 0-1, 1-2, 1-3, ... 99-100% 
	vector<int> global_score_freq(102, 0);	

	if(choice == 3)
	{
		/*Banding Technique*/
		// Applies filter using Banding Technique
		bandingTech(graph_dataset, query_graph, candidate_pairs, banding_pairs, BANDS, ROWS, banding_pair_count);
		cout<<"banding technique is done..." <<endl;

		computeSequenceSimilarity(graph_dataset, query_graph, banding_pairs, global_score_freq, simScore_threshold, g_res, simPairCount);
	}	
	else
	{
		computeSequenceSimilarity(graph_dataset, query_graph, candidate_pairs, global_score_freq, simScore_threshold, g_res, simPairCount);
	}
	// timestamping end time
	chrono::high_resolution_clock::time_point cl1 = chrono::high_resolution_clock::now();	
	cout << "Similarity Computation is done" << endl;

	// Writing statistics to the stat file
	stat_file.open("./"+res_dir+"/stat_file.txt",ios::app);

	stat_file << "Loose Size Filter count: " << loose_filter_count << endl;
	cout << "Loose size Filter count: " << loose_filter_count << endl;
	if(choice > 1)
	{
		stat_file << "Common Vertex Filter count: " << strong_filter_count << endl;
		cout << "Common Vertex Filter count: " << strong_filter_count << endl;
	}

	stat_file << "No. of Candidate Graphs: " << candidate_graph_count << endl << endl;
	cout << "No. of Candidate Graphs: " << candidate_graph_count << endl << endl;

	if(choice == 3)
	{
		stat_file << "Banding Technique filter candidates pair count: " << banding_pair_count << endl;
		cout << "Banding Technique filter candidates pair count: " << banding_pair_count << endl;
	}
	stat_file << "Similar Graphs: " << simPairCount << endl;
	cout << "Similar Graphs: " << simPairCount << endl;

	stat_file << "Memory used: " << memoryUsage() << " MB" << endl; // for displaying total memory usage (in MB)
	cout << "Memory used: " << memoryUsage() << " MB" << endl; // for displaying total memory usage (in MB)

	stat_file << "Total Time Taken: "<< (clocksTosec(cl0,cl1))  << " milliseconds" << endl;
	cout << "Total Time Taken: "<< (clocksTosec(cl0,cl1)) << " milliseconds" << endl;

	stat_file.close();

	ofstream freq_file("./"+res_dir+"/freq_distr_file.txt");
	// for simScore==0
	freq_file << "0 " << global_score_freq[0] << endl; 
	for(int i=1; i<101; i++)
		freq_file << i << " " << global_score_freq[i] << endl;
	// for simScore==100
	freq_file << "101 " << global_score_freq[101] << endl; 
	freq_file.close();

	ofstream all_graph_file("./"+res_dir+"/all_graph_file.txt");
	for(auto g_iter = g_res.begin(); g_iter != g_res.end(); g_iter++)
		all_graph_file << query_graph.gid << " " << g_iter->first << " " << g_iter->second << endl;
	all_graph_file.close();

	return 0;
}

// parses the input graph dataset and query graph
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset, int &dataset_size, Graph &query_graph, int query_index)
{
	int size;
	dataset_file >> size; // input-file dataset size
	if(dataset_size==-1)
		dataset_size = size; // use the whole dataset

	if(query_index < dataset_size)
	{
		dataset_size--;
		graph_dataset.resize(dataset_size);
		int g_ind = 0;
		// read all graphs to dataset except query_index and read query_index to query_graph
		for(int i = 0; i <= dataset_size; i++)
		{
			if(i != query_index)
			{
				graph_dataset[g_ind].readGraph(dataset_file);
				g_ind++;
			}
			else
			{
				query_graph.readGraph(dataset_file);
			}
		}
	}
	else if(query_index < size){
		graph_dataset.resize(dataset_size);
		// first read all graphs then read query_graph at query_index
		int g_ind=0;
		for(; g_ind < dataset_size; g_ind++)
		{
			graph_dataset[g_ind].readGraph(dataset_file);
		}
		while(g_ind <= query_index)
		{
			query_graph.readGraph(dataset_file);
			g_ind++;
		}
	}
}

// loads the pageranks(quality) for given graph dataset and query graph
void loadPageRanks(ifstream &pagerank_file, vector<Graph> &graph_dataset, int &dataset_size, Graph &query_graph, int query_index)
{
	unsigned gid, vCount, eCount, vtx_id;
	double vtx_quality;
	char tag;
	int size;
	pagerank_file >> size;
	if(query_index < dataset_size)
	{
		int graph_ind = 0;
		// read all graph's pagerank to dataset's pagerank except query_index and read query_index's pagerank to query_graph's pagerank
		for(int i = 0; i < dataset_size; i++)
		{
			if(i != query_index)
			{
				// first line should be of the format "g vertexCount(unsigned int) edgeCount(unsigned int) gid(unsigned int)"
				pagerank_file >> tag >> vCount >> eCount >> gid;
				for(int vtx_ind=0; vtx_ind<vCount; vtx_ind++)
				{
					// each line for each vertex should be in the format like: "v vid(unsigned int) vertex quality(double)"
					pagerank_file >> tag >> vtx_id >> vtx_quality;
					graph_dataset[graph_ind].vertices[vtx_ind].quality = vtx_quality;
				}
				graph_ind++;
			}
			else
			{
				// first line should be of the format "g vertexCount(unsigned int) edgeCount(unsigned int) gid(unsigned int)"
				pagerank_file >> tag >> vCount >> eCount >> gid;
				for(int vtx_ind=0; vtx_ind<vCount; vtx_ind++)
				{
					// each line for each vertex should be in the format like: "v vid(unsigned int) vertex quality(double)"
					pagerank_file >> tag >> vtx_id >> vtx_quality;
					query_graph.vertices[vtx_ind].quality = vtx_quality;
				}
			}
		}
	}
	else
	{
		int graph_ind = 0;
		// first read all graph's pagerank then read query_graph's pagerank at query_index's pagerank
		for(; graph_ind < dataset_size; graph_ind++)
		{
			// first line should be of the format "g vertexCount(unsigned int) edgeCount(unsigned int) gid(unsigned int)"
			pagerank_file >> tag >> vCount >> eCount >> gid;
			for(int vtx_ind=0; vtx_ind<vCount; vtx_ind++)
			{
				// each line for each vertex should be in the format like: "v vid(unsigned int) vertex quality(double)"
				pagerank_file >> tag >> vtx_id >> vtx_quality;
				graph_dataset[graph_ind].vertices[vtx_ind].quality = vtx_quality;
			}
		}
		while(graph_ind < query_index)
		{
			// first line should be of the format "g vertexCount(unsigned int) edgeCount(unsigned int) gid(unsigned int)"
			pagerank_file >> tag >> vCount >> eCount >> gid;
			for(int vtx_ind=0; vtx_ind<vCount; vtx_ind++)
			{
				// each line for each vertex should be in the format like: "v vid(unsigned int) vertex quality(double)"
				pagerank_file >> tag >> vtx_id >> vtx_quality;
			}
			graph_ind++;
		}
		// first line should be of the format "g vertexCount(unsigned int) edgeCount(unsigned int) gid(unsigned int)"
		pagerank_file >> tag >> vCount >> eCount >> gid;
		for(int vtx_ind=0; vtx_ind<vCount; vtx_ind++)
		{
			// each line for each vertex should be in the format like: "v vid(unsigned int) vertex quality(double)"
			pagerank_file >> tag >> vtx_id >> vtx_quality;
			query_graph.vertices[vtx_ind].quality = vtx_quality;
		}
	}
}

bool vertexComp2(Vertex &v1, Vertex &v2)
{
	return v1.vid < v2.vid;
}

// final sequence similarity computation of filtered graphs with query graphs
void computeSequenceSimilarity(vector<Graph> &graph_dataset, Graph &query_graph, unordered_set<unsigned> &final_cand_pairs, vector<int> &global_score_freq, double simScore_threshold, vector<pair<unsigned, double>> &g_res, unsigned long long &simPairCount)
{
	for(auto f_graph_ind = final_cand_pairs.begin(); f_graph_ind != final_cand_pairs.end(); f_graph_ind++)
	{
		double simScore = computeSimilarity(graph_dataset[*f_graph_ind], query_graph);
		
		if(simScore >= simScore_threshold)
		{
			// Incrementing count... 
			if(simScore==0.0)
			{
				// Disjoint graphs
				global_score_freq[0]++;
			}
			else if(simScore==100.0)
			{
				// Identical graphs
				global_score_freq[101]++;
			}
			else
			{
				// example: 54.5% will be mapped to index 60
				// example: 0.5% will be mapped to index 1
				// example: 99.5% will be mapped to index 100
				global_score_freq[(int)ceil(simScore)]++;
			}
			simPairCount++;
			g_res.push_back(make_pair(graph_dataset[*f_graph_ind].gid, simScore));
		}
	}	
}

// Displays the memory used by the program(in MB)
double memoryUsage()
{
	struct rusage r_usage;
	getrusage(RUSAGE_SELF, &r_usage);
	return r_usage.ru_maxrss/1024.0;
}

// Returns the time from start to end in seconds
unsigned long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end)
{
	return (unsigned long long int)(1e-6*chrono::duration_cast<chrono::nanoseconds>(end - start).count());
}

// prints correct usage of program in case of an error
void usage()
{
	cerr << "How to execute:" << endl;
	cerr <<	"1: Loose size filter" << endl;
	cerr <<	"2: Strong size filter" << endl;
	cerr <<	"3: Banding Technique" << endl;
	cerr <<	"Only 1: ./filter inp_file pagerank_file SHINGLESIZE 1 simScore_threshold dataset-size res-dir" << endl;
	cerr <<	"Only 1+2: ./filter inp_file pagerank_file SHINGLESIZE 2 simScore_threshold dataset-size res-dir" << endl;
	cerr <<	"all 1+2+3: ./filter inp_file pagerank_file SHINGLESIZE 3 simScore_threshold [BANDS] [ROWS] dataset-size res-dir" << endl;
	exit(0);
}
