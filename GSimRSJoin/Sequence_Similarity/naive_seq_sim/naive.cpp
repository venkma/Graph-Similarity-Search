#include "common.h"
#include "seq_sim.h"

// parses the input graph datasets R and S
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, int &dataset_size_R, int &dataset_size_S);

// loads the pageranks(quality) for given graph datasets R and S
void loadPageRanks(ifstream &pagerank_file, vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, int &dataset_size_R, int &dataset_size_S);

// graph comparator for sorting graph dataset
bool graphComp(Graph &g1, Graph &g2);

// Displays the memory used by the program(in MB)
double memoryUsage();

// Returns the time from start to end in seconds
unsigned long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end); 

// prints correct usage of program in case of an error
void usage(); 

int SHINGLESIZE;

// $ ./naive inp-file pagerank-file SHINGLESIZE simScore_threshold dataset-size-R dataset-size-S res-dir

void printingAndWritingFinalStatistics(int dataset_size_R,int dataset_size_S,int SHINGLESIZE,double simScore_threshold,unsigned long long simPairCount,unsigned long long int global_time,unsigned long long int totalTimeTaken,const string res_dir,vector<long long int>& global_score_freq)
{
    cout << "GSimRSJoin: Sequence Similarity(naive)" << endl;
	cout << "Dataset size of R: " << dataset_size_R << endl;
	cout << "Dataset size of S: " << dataset_size_S << endl;
	cout << "Shingle size: " << SHINGLESIZE << endl;
	cout << "Similarity Score Threshold: " << simScore_threshold << endl;
	cout << "Similar Graphs: " << simPairCount << endl;
	cout << "Memory used: " << memoryUsage()  << " MB" << endl;
	cout << "Similarity Time: "<< global_time << " milliseconds" << endl;
	cout << "Total Time Taken: "<< totalTimeTaken << " milliseconds" << endl;

	// Writing statistics to the final stat file
	ofstream stat_file("./"+res_dir+"/stat_file.txt");
	stat_file << "GSimRSJoin: Sequence Similarity(naive)" << endl;
	stat_file << "Dataset size of R: " << dataset_size_R << endl;
	stat_file << "Dataset size of S: " << dataset_size_S << endl;
	stat_file << "Shingle size: " << SHINGLESIZE << endl;
	stat_file << "Similarity Score Threshold: " << simScore_threshold << endl;
	stat_file << "Similar Graphs: " << simPairCount << endl;
	stat_file << "Memory used: " << memoryUsage() << " MB" << endl;
	stat_file << "Similarity Time: "<< global_time << " milliseconds" << endl;
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

vector<pair<unsigned, double>> writingResultsForEachGraph(const string res_dir,int gR,vector<pair<unsigned, double>> &g_res,vector<Graph> &graph_dataset_R,vector<int>& score_freq,vector<unsigned long long int>& g_time,unsigned long long int global_time)
{
    	ofstream gfile;
		gfile.open("./"+res_dir+"/graph_details/g_"+to_string(gR)+"_"+to_string(graph_dataset_R[gR].gid)+"_sim.txt");
		ofstream all_graph_file("./"+res_dir+"/all_graph_file.txt");
		all_graph_file.open("./"+res_dir+"/all_graph_file.txt",ios::app);

		// Writing the result-set for each graph to the file for each graph
		gfile << g_res.size() << endl;
		for(auto g_iter = g_res.begin(); g_iter != g_res.end(); g_iter++)
		{
			gfile << graph_dataset_R[gR].gid << " " << g_iter->first << " " << g_iter->second << endl;
			all_graph_file<< graph_dataset_R[gR].gid << " " << g_iter->first << " " << g_iter->second << endl;
		}
		
		g_res.clear();
		all_graph_file.close();

		// Writing g1's simScore-freq
		// for simScore==0
		gfile << "0 " << score_freq[0] << endl; 
		score_freq[0] = 0;
		for(int i = 1; i < 101; i++)
		{
			gfile << i << " " << score_freq[i] << endl;
			score_freq[i] = 0;
		}
		// for simScore==100
		gfile << "101 " << score_freq[101] << endl; 
		score_freq[101] = 0;
		gfile << g_time[gR] << " milliseconds" << endl;
		gfile << global_time << " milliseconds" << endl;
		gfile.close();
		
		return g_res;
}

vector<Graph> preprocessingRGraphs(vector<Graph>& graph_dataset_R)
{
    for(int graph_ind=0; graph_ind < graph_dataset_R.size(); graph_ind++)
	{
		graph_dataset_R[graph_ind].sortGraph(); // sort vertex-set wrt quality and edge-list of each vertex
		graph_dataset_R[graph_ind].walkAlgorithm(); // generate random walk of graph
		graph_dataset_R[graph_ind].computeShingles(); // compute shingles
	}
	
	return graph_dataset_R;
}
vector<Graph> preprocessingSGraphs(vector<Graph>& graph_dataset_S)
{
    for(int graph_ind=0; graph_ind < graph_dataset_S.size(); graph_ind++)
	{
		graph_dataset_S[graph_ind].sortGraph(); // sort vertex-set wrt quality and edge-list of each vertex
		graph_dataset_S[graph_ind].walkAlgorithm(); // generate random walk of graph
		graph_dataset_S[graph_ind].computeShingles(); // compute shingles
	}
	
	return graph_dataset_S;
}


int main(int argc, char const *argv[])
{
	if(argc!=8)
		usage();

	int SHINGLESIZE = stoi(argv[3]); // size of each shingle
	double simScore_threshold = stod(argv[4]); // threshold to write only those graph pairs to all_graph_file.txt
	int dataset_size_R = stoi(argv[5]); // dataset-size-R
	int dataset_size_S = stoi(argv[6]); // dataset-size-S
	const string res_dir = argv[7]; // directory in which all stat files would be stored
	mkdir(res_dir.c_str(),0777);
	mkdir((res_dir+"/graph_details/").c_str(),0777); // Details for each graph
	vector<Graph> graph_dataset_R; // Input graph dataset R
	vector<Graph> graph_dataset_S; // Input graph dataset S

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr << "Unable to open the dataset file." << endl;
		exit(0);
	}
	parseGraphDataset(dataset_file, graph_dataset_R, graph_dataset_S, dataset_size_R, dataset_size_S); // dataset parsed
	cout << "Parsing graph-dataset successfull." << endl; 
	
	ifstream pagerank_file(argv[2]);	
	if(!pagerank_file.is_open())
	{
		cerr << "Unable to open the pagerank file." << endl;
		exit(0);
	}
	// pageranks must be stored in same order of graphs as the input graph dataset file
	loadPageRanks(pagerank_file, graph_dataset_R, graph_dataset_S, dataset_size_R, dataset_size_S); 
	cout<<"PageRank Loaded successfull."<<endl;
 
	// timestamping start time
	chrono::high_resolution_clock::time_point cl0 = chrono::high_resolution_clock::now();

	// Preprocessing graph dataset R
	graph_dataset_R = preprocessingRGraphs(graph_dataset_R);
	// Preprocessing graph dataset S
        graph_dataset_S = preprocessingSGraphs(graph_dataset_S);
	
	double simScore; // similarity score
	unsigned long long simPairCount = 0; // Total similar graph pairs count

	ofstream all_graph_file("./"+res_dir+"/all_graph_file.txt");
	all_graph_file.close();

	// Result-set for each graph as vector of other graph's gid and their similarity score as double
	vector<pair<unsigned, double>> g_res;
	vector<unsigned long long int> g_time(graph_dataset_R.size());
	unsigned long long int global_time = 0;
	
	// Freq of simScore with range of 1% 0-1, 1-2, 1-3, ... 99-100% 
	vector<long long int> global_score_freq(102, 0);
	vector<int> score_freq(102,0);

	// For clock-time calculation
	chrono::high_resolution_clock::time_point clTemp0, clTemp1; 

	// Computing Sequence Similarity for each of all nC2 pairs
	for(int gR = 0; gR < graph_dataset_R.size(); gR++)
	{
		clTemp0 = chrono::high_resolution_clock::now();

		for(int gS = 0; gS < graph_dataset_S.size(); gS++)
		{
			// Similarity Calculation...
			simScore = computeSimilarity(graph_dataset_R[gR],graph_dataset_S[gS]);

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
				g_res.push_back(make_pair(graph_dataset_S[gS].gid, simScore));
				simPairCount++;
			}
		}

		clTemp1 = chrono::high_resolution_clock::now();

		g_time[gR] = clocksTosec(clTemp0,clTemp1); // Similarity Computation time for graph g1
		global_time += g_time[gR]; // Adding this to global similarity time
        
        g_res = writingResultsForEachGraph(res_dir,gR,g_res,graph_dataset_R,score_freq,g_time,global_time);
		// Creating Result Files for graph g1

	}

	chrono::high_resolution_clock::time_point cl1=chrono::high_resolution_clock::now();	
	unsigned long long int totalTimeTaken = (clocksTosec(cl0,cl1));

    printingAndWritingFinalStatistics(dataset_size_R,dataset_size_S,SHINGLESIZE,simScore_threshold,simPairCount,global_time,totalTimeTaken,res_dir,global_score_freq);
	return 0;
}

// parses the input graph dataset R and S
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, int &dataset_size_R, int &dataset_size_S)
{
	int size;
	dataset_file >> size; // input-file dataset size
	if(dataset_size_R + dataset_size_S > size)
	{
		cout << "Size of R and S exceed total size" << endl;
		exit(0);
	}
	int gcd_RS = (int)__gcd(dataset_size_R, dataset_size_S);
	int RCount = dataset_size_R/gcd_RS;
	int SCount = dataset_size_S/gcd_RS;
	cout << "R: " << RCount << endl;
	cout << "S: " << SCount << endl;
	graph_dataset_R.resize(dataset_size_R);
	graph_dataset_S.resize(dataset_size_S);
	
	for(int gR_ind = 0, gS_ind = 0; gR_ind < dataset_size_R && gS_ind < dataset_size_S; )
	{
		for(int r = 0; r < RCount; r++)
		{
			graph_dataset_R[gR_ind].readGraph(dataset_file);
			gR_ind++;
		}
		for(int s = 0; s < SCount; s++)
		{
			graph_dataset_S[gS_ind].readGraph(dataset_file);
			gS_ind++;
		}
	}
}

// loads the pageranks(quality) for given graph datasets R and S
void loadPageRanks(ifstream &pagerank_file, vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, int &dataset_size_R, int &dataset_size_S)
{
	unsigned gid, vCount, eCount, vtx_id;
	double vtx_quality;
	char tag;
	int size;
	pagerank_file >> size;

	if(dataset_size_R + dataset_size_S > size)
	{
		cout << "Size of R and S exceed total size" << endl;
		exit(0);
	}
	int gcd_RS = (int)__gcd(dataset_size_R, dataset_size_S);
	int RCount = dataset_size_R/gcd_RS;
	int SCount = dataset_size_S/gcd_RS;
	
	for(int gR_ind = 0, gS_ind = 0; gR_ind < dataset_size_R && gS_ind < dataset_size_S; )
	{
		for(int r = 0; r < RCount; r++)
		{
			pagerank_file >> tag >> vCount >> eCount >> gid;
			for(int vtx_ind=0; vtx_ind<vCount; vtx_ind++)
			{
				// each line for each vertex should be in the format like: "v vid(unsigned int) vertex quality(double)"
				pagerank_file >> tag >> vtx_id >> vtx_quality;
				graph_dataset_R[gR_ind].vertices[vtx_ind].quality = vtx_quality;
			}
			gR_ind++;
		}
		for(int s = 0; s < SCount; s++)
		{
			pagerank_file >> tag >> vCount >> eCount >> gid;
			for(int vtx_ind=0; vtx_ind<vCount; vtx_ind++)
			{
				// each line for each vertex should be in the format like: "v vid(unsigned int) vertex quality(double)"
				pagerank_file >> tag >> vtx_id >> vtx_quality;
				graph_dataset_S[gS_ind].vertices[vtx_ind].quality = vtx_quality;
			}
			gS_ind++;
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

// prints correct usage of program in case of an error
void usage()
{
	cerr << "usage: ./naive path/dataset_filename path/pagerank_filename SHINGLESIZE simScore_threshold dataset_size_R dataset_size_S res_dir_folder" <<endl;
	exit(0);
}
