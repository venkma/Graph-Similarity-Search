#include "common.h"
#include "seq_sim.h"

// For parsing the input graph dataset
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset, int &dataset_size, Graph &query_graph, int query_index);

// loads the pageranks(quality) for given graph dataset
void loadPageRanks(ifstream &pagerank_file, vector<Graph> &graph_dataset, int &dataset_size, Graph &query_graph, int query_index);

// Displays the memory used by the program(in MB)
double memoryUsage();

// Returns the time from start to end in seconds
unsigned long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end); 

// prints correct usage of program in case of an error
void usage(); 

int SHINGLESIZE;

// $ ./naive inp-file pagerank-file SHINGLESIZE simScore_threshold dataset-size query_index res-dir

int main(int argc, char const *argv[])
{
	if(argc!=8)
		usage();
	
	SHINGLESIZE = stoi(argv[3]); // size of each shingle
	double simScore_threshold = stod(argv[4]); // threshold to write only those graph pairs to all_graph_file.txt
	int dataset_size = stoi(argv[5]); // dataset-size
	int query_index = stoi(argv[6]); // index of query graph in dataset
	const string res_dir = argv[7]; // directory in which all stat files would be stored
	mkdir(res_dir.c_str(),0777);

	vector<Graph> graph_dataset; // Input graph dataset
	Graph query_graph;

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr << "Unable to open the dataset file." << endl;
		exit(0);
	}
	parseGraphDataset(dataset_file, graph_dataset, dataset_size, query_graph, query_index); // dataset parsed	
	cout << "Parsing graph-dataset successfull." << endl; 
	
	ifstream pagerank_file(argv[2]);	
	if(!pagerank_file.is_open())
	{
		cerr << "Unable to open the pagerank file." << endl;
		exit(0);
	}
	// pageranks must be stored in same order of graphs as the input graph dataset file
	loadPageRanks(pagerank_file, graph_dataset, dataset_size, query_graph, query_index); 
	cout<<"PageRank Loaded successfull."<<endl;
 
	// timestamping start time
	chrono::high_resolution_clock::time_point cl0 = chrono::high_resolution_clock::now();

	// Preprocessing the graph dataset
	query_graph.sortGraph(); // sort vertex-set wrt quality and edge-list of each vertex
	query_graph.walkAlgorithm(); // generate random walk of graph
	query_graph.computeShingles(); // compute shingles
	for(int graph_ind = 0; graph_ind < graph_dataset.size(); graph_ind++)
	{
		graph_dataset[graph_ind].sortGraph(); // sort vertex-set wrt quality and edge-list of each vertex
		graph_dataset[graph_ind].walkAlgorithm(); // generate random walk of graph
		graph_dataset[graph_ind].computeShingles(); // compute shingles
	}

	double simScore;

	// Result-set for each graph as vector of other graph's gid and their similarity score as double
	vector<pair<unsigned, double>> g_res;
	vector<int> global_score_freq(102,0);

	cout << query_index << endl;
	query_graph.displayGraph();
	// Computing Sequence Similarity for each of all nC2 pairs
	for(int g1 = 0; g1 < graph_dataset.size(); g1++)
	{
		// Similarity Calculation...
		simScore = computeSimilarity(graph_dataset[g1], query_graph);
		
		// Incrementing count... 
		if(simScore == 0.0)
		{
			// Disjoint graphs
			global_score_freq[0]++;
		}
		else if(simScore == 100.0)
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

		// Storing only those graph pairs which have similarity above (simScore_threshold)%
		if(simScore >= simScore_threshold)
			g_res.push_back(make_pair(graph_dataset[g1].gid, simScore));
	}

	// For clock-time calculation
	chrono::high_resolution_clock::time_point cl1 = chrono::high_resolution_clock::now();

	// Creating Result Files for graph 
	ofstream stat_file, all_graph_file;
	all_graph_file.open("./"+res_dir+"/all_graph_file.txt");

	// Writing the result-set for each graph to the file for each graph
	for(auto g_iter = g_res.begin(); g_iter != g_res.end(); g_iter++)
	{
		all_graph_file<< query_graph.gid << " " << g_iter->first << " " << g_iter->second << endl;
	}
	all_graph_file.close();
	
	cout << "GSimSearch: Sequence Similarity(naive)" << endl;
	cout << "Dataset size: " << dataset_size << endl;
	cout << "Query Index: " << query_index << endl;
	cout << "Query Graph id: " << query_graph.gid << endl;
	cout << "Shingle size: " << SHINGLESIZE << endl;
	cout << "Similarity Score Threshold: " << simScore_threshold << endl;
	cout << "Similarity pairs: " << g_res.size() << endl;
	cout << "Memory used: " << memoryUsage()  << " MB" << endl;
	cout << "Total Time Taken: "<< (clocksTosec(cl0,cl1)) << " milliseconds" << endl;

	// Writing statistics to the final stat file
	stat_file.open("./"+res_dir+"/stat_file.txt");
	
	stat_file << "GSimSearch: Sequence Similarity(naive)" << endl;
	stat_file << "Dataset size: " << dataset_size << endl;
	stat_file << "Query Index: " << query_index << endl;
	stat_file << "Query Graph id: " << query_graph.gid << endl;
	stat_file << "Shingle size: " << SHINGLESIZE << endl;
	stat_file << "Similarity Score Threshold: " << simScore_threshold << endl;
	stat_file << "Similarity pairs: " << g_res.size() << endl;
	stat_file << "Memory used: " << memoryUsage() << " MB" << endl;
	stat_file << "Total Time Taken: "<< (clocksTosec(cl0,cl1))  << " milliseconds" << endl;
	stat_file.close();
	
	ofstream freq_file("./"+res_dir+"/freq_distr_file.txt");
	// for simScore==0
	freq_file << "0 " << global_score_freq[0] << endl; 
	for(int i=1; i<101; i++)
		freq_file << i << " " << global_score_freq[i] << endl;
	// for simScore==100
	freq_file << "101 " << global_score_freq[101] << endl; 
	freq_file.close();

	return 0;
}

// prints correct usage of program in case of an error
void usage()
{
	cerr << "usage: ./naive path/dataset_filename path/pagerank_filename SHINGLESIZE simScore_threshold dataset_size query_index res_dir_folder" <<endl;
	exit(0);
}

// Returns the time from start to end in seconds
unsigned long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end)
{
	return (unsigned long long int)(1e-6*chrono::duration_cast<chrono::nanoseconds>(end - start).count());
}

// Displays the memory used by the program(in MB)
double memoryUsage()
{
	struct rusage r_usage;
	getrusage(RUSAGE_SELF, &r_usage);
	return r_usage.ru_maxrss/1024.0;
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
		for(int i = 0; i < dataset_size+1; i++)
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
