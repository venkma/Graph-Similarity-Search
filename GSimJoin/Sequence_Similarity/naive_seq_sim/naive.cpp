#include "common.h"
#include "seq_sim.h"

// For parsing the input graph dataset
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset, int &dataset_size); 

// loads the pageranks(quality) for given graph dataset
void loadPageRanks(ifstream &pagerank_file, vector<Graph> &graph_dataset, int &dataset_size);

// graph comparator for sorting graph dataset
bool graphComp(Graph &g1, Graph &g2);

// Displays the memory used by the program(in MB)
double memoryUsage();

// Returns the time from start to end in seconds
unsigned long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end); 

// prints correct usage of program in case of an error
void usage(); 

int SHINGLESIZE;

// $ ./naive inp-file pagerank-file SHINGLESIZE simScore_threshold dataset-size res-dir

int main(int argc, char const *argv[])
{
	if(argc!=7)
		usage();
	
	SHINGLESIZE = stoi(argv[3]); // size of each shingle
	double simScore_threshold = stod(argv[4]); // threshold to write only those graph pairs to all_graph_file.txt
	int dataset_size = stoi(argv[5]); // dataset-size
	const string res_dir = argv[6]; // directory in which all stat files would be stored
	mkdir(res_dir.c_str(),0777);
	mkdir((res_dir+"/graph_details/").c_str(),0777);
	vector<Graph> graph_dataset; // Input graph dataset

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr << "Unable to open the dataset file." << endl;
		exit(0);
	}
	parseGraphDataset(dataset_file, graph_dataset, dataset_size); // dataset parsed	
	cout << "Parsing graph-dataset successfull." << endl; 

	ifstream pagerank_file(argv[2]);	
	if(!pagerank_file.is_open())
	{
		cerr << "Unable to open the pagerank file." << endl;
		exit(0);
	}
	// pageranks must be stored in same order of graphs as the input graph dataset file
	loadPageRanks(pagerank_file, graph_dataset, dataset_size); 
	cout<<"PageRank Loaded successfull."<<endl;

 	// timestamping start time
	chrono::high_resolution_clock::time_point cl0 = chrono::high_resolution_clock::now();

	// Preprocessing the graph dataset
	for(int graph_ind=0; graph_ind < graph_dataset.size(); graph_ind++)
	{
		graph_dataset[graph_ind].sortGraph(); // sort vertex-set wrt quality and edge-list of each vertex
		graph_dataset[graph_ind].walkAlgorithm(); // generate random walk of graph
		graph_dataset[graph_ind].computeShingles(); // compute shingles
	}

	double simScore; // similarity score
	unsigned long long int simPairCount = 0; // no. of graph pairs having similarity score > threshold

	ofstream all_graph_file("./"+res_dir+"/all_graph_file.txt");
	all_graph_file.close();

	// Result-set for each graph as vector of other graph's gid and their similarity score as double
	vector<pair<unsigned, double>> g_res; // stores graph pair with the score of a specific graph
	vector<unsigned long long int> g_time(graph_dataset.size()); // stores time required for each graph in the dataset
	unsigned long long int global_time = 0; // total time taken for similarity computation 
	// Freq of simScore with range of 1% 0-1, 1-2, 1-3, ... 99-100% 
	vector<int> score_freq(102,0); // stores sim-score frequency distribution of a particular graph
	vector<long long int> global_score_freq(102, 0); // stores sim-score frequency distribution of the dataset

	// For clock-time calculation
	chrono::high_resolution_clock::time_point clTemp0, clTemp1; 

	// Computing Sequence Similarity for each of all nC2 pairs
	for(int g1 = 1; g1<graph_dataset.size(); g1++)
	{
		clTemp0 = chrono::high_resolution_clock::now();

		for(int g2 = g1-1; g2>=0; g2--)
		{
			// Similarity Calculation...
			simScore = computeSimilarity(graph_dataset[g1],graph_dataset[g2]);

			// Incrementing count... 
			if(simScore==0.0)
			{
				// Disjoint graphs
				score_freq[0]++;
				global_score_freq[0]++;
			}
			else if(simScore==100.0)
			{
				// Identical graphs
				score_freq[101]++;
				global_score_freq[101]++;
			}
			else
			{
				// example: 54.5% will be mapped to index 60
				// example: 0.5% will be mapped to index 1
				// example: 99.5% will be mapped to index 100
				score_freq[(int)ceil(simScore)]++;
				global_score_freq[(int)ceil(simScore)]++;
			}

			// Storing only those graph pairs which have similarity above (simScore_threshold)%
			if(simScore>=simScore_threshold)
			{
				simPairCount++;
				g_res.push_back(make_pair(graph_dataset[g2].gid, simScore));
			}
		}

		clTemp1 = chrono::high_resolution_clock::now();

		g_time[g1] = clocksTosec(clTemp0,clTemp1); // Similarity Computation time for graph g1
		global_time += g_time[g1]; // Adding this to global similarity time

		// Creating Result Files for graph g1
		ofstream gfile;
		gfile.open("./"+res_dir+"/graph_details/g_"+to_string(g1)+"_"+to_string(graph_dataset[g1].gid)+"_sim.txt");
		all_graph_file.open("./"+res_dir+"/all_graph_file.txt",ios::app);

		// Writing the result-set for each graph to the file for each graph
		gfile << g_res.size() << endl;
		for(auto g_iter = g_res.begin(); g_iter != g_res.end(); g_iter++)
		{
			gfile << graph_dataset[g1].gid << " " << g_iter->first << " " << g_iter->second << endl;
			all_graph_file<< graph_dataset[g1].gid << " " << g_iter->first << " " << g_iter->second << endl;
		}
		g_res.clear();
		all_graph_file.close();

		// Writing g1's simScore-freq
		// for simScore==0
		gfile << "0 " << score_freq[0] << endl; 
		score_freq[0] = 0;
		for(int i=1; i<101; i++)
		{
			gfile << i << " " << score_freq[i] << endl;
			score_freq[i] = 0;
		}
		// for simScore==100
		gfile << "101 " << score_freq[101] << endl; 
		score_freq[101] = 0;
		gfile << g_time[g1] << " milliseconds" << endl;
		gfile << global_time << " milliseconds" << endl;
		gfile.close();

	}
 	// timestamping end time
	chrono::high_resolution_clock::time_point cl1=chrono::high_resolution_clock::now();	

	cout << "GSimJoin: Sequence Similarity(naive)" << endl;
	cout << "Dataset size: " << dataset_size << endl;
	cout << "Shingle size: " << SHINGLESIZE << endl;
	cout << "Similarity Score Threshold: " << simScore_threshold << endl;
	cout << "Similar Graph Pairs: " << simPairCount << endl;
	cout << "Memory used: " << memoryUsage()  << " MB" << endl;
	cout << "Similarity Time: "<< global_time << " milliseconds" << endl;
	cout << "Total Time Taken: "<< (clocksTosec(cl0,cl1)) << " milliseconds" << endl;

	// Writing statistics to the final stat file
	ofstream stat_file("./"+res_dir+"/stat_file.txt");
	stat_file << "GSimJoin: Sequence Similarity(naive)" << endl;
	stat_file << "Dataset size: " << dataset_size << endl;
	stat_file << "Shingle size: " << SHINGLESIZE << endl;
	stat_file << "Similarity Score Threshold: " << simScore_threshold << endl;
	stat_file << "Similar Graph Pairs: " << simPairCount << endl;
	stat_file << "Memory used: " << memoryUsage() << " MB" << endl;
	stat_file << "Similarity Time: "<< global_time << " milliseconds" << endl;
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
	cerr << "usage: ./naive path/dataset_filename path/pagerank_filename SHINGLESIZE simScore_threshold dataset_size res_dir_folder" <<endl;
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

// parses the input graph dataset
void parseGraphDataset(ifstream &inp, vector<Graph> &graph_dataset, int &dataset_size)
{
	int size;
	inp >> size; // input-file dataset size
	if(dataset_size==-1) 
		dataset_size=size; // use the whole dataset 
	graph_dataset.resize(dataset_size);

	for(auto g_ind=0; g_ind < dataset_size; g_ind++)
	{
		graph_dataset[g_ind].readGraph(inp);

	}
}

// loads the pageranks(quality) for given graph dataset
void loadPageRanks(ifstream &pagerank_file, vector<Graph> &graph_dataset, int &dataset_size)
{
	unsigned gid, vCount, eCount, vtx_id;
	double vtx_quality;
	char tag;
	int size;
	pagerank_file >> size;
	for(int graph_ind=0; graph_ind < dataset_size; graph_ind++)
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
}

// graph comparator for sorting graph dataset
bool graphComp(Graph &g1, Graph &g2)
{
	int sumCount_g1 = g1.vertexCount+g1.edgeCount; 
	int sumCount_g2 = g2.vertexCount+g2.edgeCount;
	if(sumCount_g1==sumCount_g2)
		return g1.gid<g2.gid;
	return sumCount_g1<sumCount_g2;
}
