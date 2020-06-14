#include "veo.h"

using namespace std;

// For parsing the input graph dataset
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset, Graph &query_graph, int &dataset_size, int &query_index);

// Sorts vertex and edge set of graph dataset
void sortGraphDataset(vector<Graph> &graph_dataset, Graph &query_graph);

// Returns the time from start to end in seconds
unsigned long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end);

// Displays the memory used by the program(in MB)
double memoryUsage();

// prints correct usage of program in case of an error
void usage(); 

// $ ./naive inp-file simScore_threshold dataset-size query_index res-file

void printingAndWritingFinalStatistics(int dataset_size,int query_index,double simScore_threshold,vector<pair<unsigned, double>>& g_res,int totalTimeTaken,const string res_dir,vector<int>& global_score_freq,Graph query_graph)
{
	// Creating Result Files for graph 
	ofstream stat_file, all_graph_file;
	all_graph_file.open("./"+res_dir+"/all_graph_file.txt");
	
	// Writing the result-set for each graph to the file for each graph
	for(auto g_iter = g_res.begin(); g_iter != g_res.end(); g_iter++)
	{
		all_graph_file << query_graph.gid << " " << g_iter->first << " " << g_iter->second << endl;
	}
	all_graph_file.close();

	cout << "GSimSearch: VEO Similarity(naive)" << endl;
	cout << "Dataset size: " << dataset_size << endl;
	cout << "Query Index: " << query_index << endl;
	cout << "Query Graph id: " << query_graph.gid << endl;
	cout << "Similarity Score Threshold: " << simScore_threshold << endl;
	cout << "Similarity pairs: " << g_res.size() << endl;
	cout << "Memory used: " << memoryUsage()  << " MB" << endl;
	cout << "Total Time Taken: "<< totalTimeTaken << " milliseconds" << endl;
	
	// Writing statistics to the final stat file
	stat_file.open("./"+res_dir+"/stat_file.txt");	
	stat_file << "GSimSearch: VEO Similarity(naive)" << endl;
	stat_file << "Dataset size: " << dataset_size << endl;
	stat_file << "Query Index: " << query_index << endl;
	stat_file << "Query Graph id: " << query_graph.gid << endl;
	stat_file << "Similarity Score Threshold: " << simScore_threshold << endl;
	stat_file << "Similarity pairs: " << g_res.size() << endl;
	stat_file << "Memory used: " << memoryUsage() << " MB" << endl;
	stat_file << "Total Time Taken: "<< totalTimeTaken  << " milliseconds" << endl;
	stat_file.close();
	
	ofstream freq_file("./"+res_dir+"/freq_distr_file.txt");
	// for simScore==0
	freq_file << "0 " << global_score_freq[0] << endl; 
	for(int i=1; i<101; i++)
		freq_file << i << " " << global_score_freq[i] << endl;
	// for simScore==100
	freq_file << "101 " << global_score_freq[101] << endl; 
	freq_file.close();
		
}

int main(int argc, char const *argv[])
{
	if(argc!=6)
		usage();

	double simScore_threshold = stod(argv[2]);  // threshold to write only those graph pairs to all_graph_file.txt
	int dataset_size = stoi(argv[3]); // size of input dataset
	int query_index = stoi(argv[4]); // index of query graph in dataset
	const string res_dir = argv[5]; // directory in which all stat files would be stored
	mkdir(res_dir.c_str(),0777);

	vector<Graph> graph_dataset; // input graph dataset
	Graph query_graph;

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr << "Unable to open dataset file" << endl;
		exit(0);
	}
	// parsing input dataset 
	parseGraphDataset(dataset_file, graph_dataset, query_graph, dataset_size, query_index);
	cout << argv[1] << ": Graph Dataset parsed." << endl;

	sortGraphDataset(graph_dataset, query_graph); // to sort vertex and edge set
	cout << "All graphs in dataset sorted." << endl;

	// Result-set for each graph as vector of other graph's gid and their similarity score as double
	vector<pair<unsigned, double>> g_res;
	// Freq of simScore with range of 1% 0-1, 1-2, 1-3, ... 99-100% 
	vector<int> global_score_freq(102, 0);
	long long int global_time = 0;

	double simScore;

	// For clock-time calculation
	chrono::high_resolution_clock::time_point cl0 = chrono::high_resolution_clock::now();

	for(int g1 = 0; g1 < graph_dataset.size(); g1++)
	{
		// Similarity Calculation...
		double common = 0;
		double simScore = computeSimilarity(graph_dataset[g1], query_graph, common);
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
		// Storing only those graph pairs which have similarity above (simScore_threshold)%
		if(simScore >= simScore_threshold)
			g_res.push_back(make_pair(graph_dataset[g1].gid, simScore));
	}
		
	// For clock-time calculation
	chrono::high_resolution_clock::time_point cl1 = chrono::high_resolution_clock::now();
    	int totalTimeTaken = (clocksTosec(cl0,cl1));
	printingAndWritingFinalStatistics(dataset_size,query_index,simScore_threshold,g_res,totalTimeTaken,res_dir,global_score_freq,query_graph);
	

	return 0;
}

// parses the input graph dataset and query graph
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset, Graph &query_graph, int &dataset_size, int &query_index)
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

// Sorts vertex and edge set of graph dataset
void sortGraphDataset(vector<Graph> &graph_dataset, Graph &query_graph)
{
	sort(query_graph.vertices.begin(), query_graph.vertices.end()); // sort vertex-set 
	sort(query_graph.edges.begin(), query_graph.edges.end()); // sort edge-set
	for(int i = 0; i < graph_dataset.size(); i++)
	{
		sort(graph_dataset[i].vertices.begin(), graph_dataset[i].vertices.end()); // sort vertex-set 
		sort(graph_dataset[i].edges.begin(), graph_dataset[i].edges.end()); // sort edge-set
	}
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

void usage()
{
	cerr << "usage: ./naive inp-file simScore_threshold dataset-size query_index res-file" << endl;
	exit(0);
}

