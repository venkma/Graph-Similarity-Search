#include "common.h"
#include "seq_sim.h"

// For parsing the input graph datasets R and S
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, int &dataset_size_R, int &dataset_size_S);

// loads the pageranks(quality) for given graph datasets R and S
void loadPageRanks(ifstream &pagerank_file, vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, int &dataset_size_R, int &dataset_size_S);

// vertex comparator based on vertex ids
bool vertexComp2(Vertex &v1, Vertex &v2);

// Sorts vertex and edge set of graph dataset
void sortGraphDataset(vector<Graph> &graph_dataset_R,int dataset_size_R,vector<Graph> &graph_dataset_S,int dataset_size_S);


// final sequence similarity computation of pruned pairs
void computeSequenceSimilarity(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, unordered_map<unsigned, unordered_set<unsigned> > &final_cand_pairs, vector<int> &global_score_freq, double simScore_threshold, vector<pair<pair<unsigned, unsigned>, double>> &g_res, unsigned long long &simPairCount);

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

// $ ./filter inp_file pagerank_file SHINGLESIZE choice simScore_threshold [BANDS] [ROWS] dataset-size-R dataset-size-S res-dir
//                                                           			   |____________|
//							                        			            for choice 3

vector<int> printingAndWritingInitialStatistics(int dataset_size_R,int dataset_size_S,int SHINGLESIZE,int choice,double simScore_threshold,const char* path,const string res_dir,const char* bandspath,const char* rowspath)
{
    cout << "GSimRSJoin: Sequence Similarity(filters)" << endl;
	cout << "Dataset size of R: " << dataset_size_R << endl;
	cout << "Dataset size of S: " << dataset_size_S << endl;
	cout << "Shingle Size: " << SHINGLESIZE << endl;
	cout << "Choice of filter: " << choice << endl;
	cout << "Threshold: " << simScore_threshold << endl;
	cout << "Input file: " << path <<endl;
	cout << "Results Directory: " << res_dir <<endl;

	// Writing statistics to the stat file
	ofstream stat_file("./"+res_dir+"/stat_final.txt");
	stat_file << "GSimRSJoin: Sequence Similarity(filters)" << endl;
	stat_file << "Dataset size of R: " << dataset_size_R << endl;
	stat_file << "Dataset size of S: " << dataset_size_S << endl;
	stat_file << "Shingle size: " << SHINGLESIZE << endl;
	stat_file << "Similarity Score Threshold: " << simScore_threshold << endl;
	stat_file << "Filter choice: " << choice << endl;

	int BANDS,ROWS;
	if(choice == 3)
	{
		BANDS = stoi(rowspath);
		ROWS = stoi(bandspath);
		cout << "Bands: " << BANDS <<endl;
		cout << "Rows: " << ROWS <<endl;
		stat_file << "No. of Bands: " << BANDS << endl;
		stat_file << "No. of Rows: " << ROWS << endl;
	}
	stat_file.close();
	vector<int> v;
    v.push_back(BANDS);
    v.push_back(ROWS);

    return v; 
}
void printingAndWritingFinalStatistics(const string res_dir,long long loose_filter_count,long long strong_filter_count,long long candidate_graph_count_R,long long candidate_graph_count_S,int choice,long long banding_pair_count,unsigned long long simPairCount,unsigned long long int totalTimeTimetaken,vector<int>& global_score_freq,vector<pair<pair<unsigned, unsigned>, double>>& g_res)
{
	ofstream stat_file("./"+res_dir+"/stat_final.txt");
        stat_file.open("./"+res_dir+"/stat_final.txt", ios::app);
	stat_file << "Loose Size Filter count: " << loose_filter_count << endl;
	cout << "Loose size Filter count: " << loose_filter_count << endl;
	if(choice > 1)
	{
		stat_file << "Common Vertex Filter count: " << strong_filter_count << endl;
		cout << "Common Vertex Filter count: " << strong_filter_count << endl;
	}
	stat_file << "No. of Candidate Graphs from R: " << candidate_graph_count_R << endl << endl;
	cout << "No. of Candidate Graphs from R: " << candidate_graph_count_R << endl << endl;
	
	stat_file << "No. of Candidate Graphs from S: " << candidate_graph_count_S << endl << endl;
	cout << "No. of Candidate Graphs from S: " << candidate_graph_count_S << endl << endl;
	if(choice == 3)
	{
		stat_file << "Banding Technique filter candidates pair count: " << banding_pair_count << endl;
		cout << "Banding Technique filter candidates pair count: " << banding_pair_count << endl;
	}
	stat_file << "Similar Graphs: " << simPairCount << endl;
	cout << "Similar Graphs: " << simPairCount << endl;

	stat_file << "Memory used: " << memoryUsage() << " MB" << endl;
	cout << "Memory used: " << memoryUsage() << " MB" << endl; // for displaying total memory usage (in MB)
	stat_file << "Total Time Taken: "<< totalTimeTimetaken  << " milliseconds" << endl;
	cout << "Total Time Taken: "<< totalTimeTimetaken << " milliseconds" << endl;

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
		all_graph_file << g_iter->first.first << " " << g_iter->first.second << " " << g_iter->second << endl;
	all_graph_file.close();

    
}

int main(int argc, char const *argv[])
{
	/*verifying args*/
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
	int dataset_size_R = stoi(argv[argc-3]); // dataset-size of R 
	int dataset_size_S = stoi(argv[argc-2]); // dataset-size of S 
	const string res_dir = argv[argc-1]; // directory in which all stat files would be stored
	mkdir(res_dir.c_str(),0777);

	vector<Graph> graph_dataset_R; // Input graph dataset R
	vector<Graph> graph_dataset_S; // Input graph dataset S
	unsigned long long simPairCount = 0; // count of allsimilar pairs
	long long loose_filter_count = 0, strong_filter_count = 0, candidate_graph_count_R = 0, candidate_graph_count_S = 0, banding_pair_count = 0; // count of pruned graphs after each filter
	vector<pair<pair<unsigned, unsigned>, double>> g_res; // stores all the similar graphs 

	vector<bool> candidate_graphs_R; // pruned graphs
	vector<bool> candidate_graphs_S; // pruned graphs	
	unordered_map<unsigned, unordered_set<unsigned> > candidate_pairs; // candidate pairs after filters 1 & 2
	unordered_map<unsigned, unordered_set<unsigned> > banding_pairs; // candidate pairs after banding technique

	vector<int> v1 = printingAndWritingInitialStatistics(dataset_size_R,dataset_size_S,SHINGLESIZE,choice,simScore_threshold,argv[1],res_dir,argv[6],argv[7]);
    
    int BANDS = v1[0];
    int ROWS  = v1[1];

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr<<"Unable to open the given input file"<<endl;
		exit(0);
	}
	parseGraphDataset(dataset_file, graph_dataset_R, graph_dataset_S, dataset_size_R, dataset_size_S); // Parsing Graph Dataset
	cout<< "Parsing of input graph dataset is done." <<endl; 

	candidate_graphs_R.assign(graph_dataset_R.size(), false);	
	candidate_graphs_S.assign(graph_dataset_S.size(), false);
	
	// Parsing Pageranks
	ifstream pagerank_file(argv[2]);
	if(!pagerank_file.is_open())
	{
		cerr << "Unable to open the pagerank file." << endl;
		exit(0);
	}
	// Pageranks must be stored in same order of graphs as the input graph dataset file
	loadPageRanks(pagerank_file, graph_dataset_R, graph_dataset_S, dataset_size_R, dataset_size_S); // Parsing Pageranks
	cout<<"Parsing of Pageranks is done."<<endl;

	// sort R dataset's each graph's vertex-set based on vertex ids 
	sortGraphDataset(graph_dataset_R,dataset_size_R,graph_dataset_S,dataset_size_S);
	
        
	
	// timestamping start time
	chrono::high_resolution_clock::time_point cl0 = chrono::high_resolution_clock::now();
	
	// Applies Loose size and Strong size filter to input graph dataset
	applyFilters(graph_dataset_R, graph_dataset_S, candidate_graphs_R, candidate_graphs_S, candidate_pairs, dataset_size_R, dataset_size_S, choice, simScore_threshold, loose_filter_count, strong_filter_count, candidate_graph_count_R, candidate_graph_count_S);
	cout << "Loose-size and Common-vertex Filters are done." <<endl;
	
	// Preprocesses the pruned graph pairs 
	preProcess(graph_dataset_R, graph_dataset_S, candidate_graphs_R, candidate_graphs_S);
	cout<<"Preprocessing is done."<<endl;

	// Freq of simScore with range of 1% 0-1, 1-2, 1-3, ... 99-100% 
	vector<int> global_score_freq(102, 0);

	if(choice == 3)
	{
		/*Banding Technique*/
		// Applies filter using Banding Technique
		bandingTech(graph_dataset_R, graph_dataset_S, candidate_pairs, banding_pairs, BANDS, ROWS, banding_pair_count);
		cout<<"Banding Technique filter is done." <<endl;
		// Computing naive sequence similarity
		computeSequenceSimilarity(graph_dataset_R, graph_dataset_S, banding_pairs, global_score_freq, simScore_threshold, g_res, simPairCount);
	}
	else
	{
		// Computing naive sequence similarity
		computeSequenceSimilarity(graph_dataset_R, graph_dataset_S, candidate_pairs, global_score_freq, simScore_threshold, g_res, simPairCount);
	}
	chrono::high_resolution_clock::time_point cl1=chrono::high_resolution_clock::now();	
	cout << "Similarity Computation is done" << endl;
    unsigned long long int totalTimeTimetaken = (clocksTosec(cl0,cl1));
	// Writing statistics to the stat file
	
						               		printingAndWritingFinalStatistics(res_dir,loose_filter_count,strong_filter_count,candidate_graph_count_R,candidate_graph_count_S,choice,banding_pair_count,simPairCount,totalTimeTimetaken,global_score_freq,g_res);

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

bool vertexComp2(Vertex &v1, Vertex &v2)
{
	return v1.vid < v2.vid;
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

// Sorts vertex and edge set of graph dataset
void sortGraphDataset(vector<Graph> &graph_dataset_R,int dataset_size_R,vector<Graph> &graph_dataset_S,int dataset_size_S)
{
	for(int g_ind = 0; g_ind < dataset_size_R; g_ind++)
	{
		sort(graph_dataset_R[g_ind].vertices.begin(), graph_dataset_R[g_ind].vertices.end(), vertexComp2);
		for(int vtx_ind = 0; vtx_ind < graph_dataset_R[g_ind].vertexCount; vtx_ind++)
			graph_dataset_R[g_ind].vid_to_ind[graph_dataset_R[g_ind].vertices[vtx_ind].vid] = vtx_ind;
	}
	for(int g_ind = 0; g_ind < dataset_size_S; g_ind++)
	{
		sort(graph_dataset_S[g_ind].vertices.begin(), graph_dataset_S[g_ind].vertices.end(), vertexComp2);
		for(int vtx_ind = 0; vtx_ind < graph_dataset_S[g_ind].vertexCount; vtx_ind++)
			graph_dataset_S[g_ind].vid_to_ind[graph_dataset_S[g_ind].vertices[vtx_ind].vid] = vtx_ind;
	}
}

// final sequence similarity computation of pruned pairs
void computeSequenceSimilarity(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, unordered_map<unsigned, unordered_set<unsigned> > &final_cand_pairs, vector<int> &global_score_freq, double simScore_threshold, vector<pair<pair<unsigned, unsigned>, double>> &g_res, unsigned long long &simPairCount)
{
	for(auto gR = final_cand_pairs.begin(); gR != final_cand_pairs.end(); gR++)
	{
		for(auto gS = final_cand_pairs[gR->first].begin(); gS != final_cand_pairs[gR->first].end(); gS++)
		{
			double simScore = computeSimilarity(graph_dataset_R[gR->first], graph_dataset_S[*gS]);
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
			if(simScore >= simScore_threshold)
			{
				simPairCount++;
				g_res.push_back(make_pair(make_pair(graph_dataset_R[gR->first].gid, graph_dataset_S[*gS].gid), simScore));
			}
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
	cerr <<	"Only 1: ./filter inp_file pagerank_file SHINGLESIZE 1 simScore_threshold dataset-size-R dataset-size-S res-dir" << endl;
	cerr <<	"Only 1+2: ./filter inp_file pagerank_file SHINGLESIZE 2 simScore_threshold dataset-size-R dataset-size-S res-dir" << endl;
	cerr <<	"all 1+2+3: ./filter inp_file pagerank_file SHINGLESIZE 3 simScore_threshold [BANDS] [ROWS] dataset-size-R dataset-size-S res-dir" << endl;
	exit(0);
}
