#include "common.h"
#include "seq_sim.h"

// For parsing the input graph dataset
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset, int &dataset_size);

// loads the pageranks(quality) for given graph dataset
void loadPageRanks(ifstream &pagerank_file, vector<Graph> &graph_dataset, int &dataset_size);

// vertex comparator based on vertex ids
bool vertexComp2(Vertex &v1, Vertex &v2);

// final sequence similarity computation of pruned pairs
void computeSequenceSimilarity(vector<Graph> &graph_dataset, unordered_map<unsigned, unordered_set<unsigned> > &candidate_pairs, vector<int> &global_score_freq, double simScore_threshold, string res_dir);

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

// $ ./filter inp_file pagerank_file SHINGLESIZE choice simScore_threshold [BANDS] [ROWS] dataset-size res-dir
//                                                           			   |____________|
// for choice 3

unordered_map<unsigned, unordered_set<unsigned> > computeFinalCandidatePairs(unordered_map<unsigned, unordered_set<unsigned> > &banding_pairs,unordered_map<unsigned, unordered_set<unsigned> > &final_candidate_pairs,unordered_map<unsigned, unordered_set<unsigned> > &candidate_pairs,const string res_dir)
{
    unsigned long long banding_filter_count = 0, final_cand_pair_count = 0;
		// Performs intersection between pruned graphs (after filter 1 & 2) with pruned graphs (after filter 3)
		for(auto g1 = banding_pairs.begin(); g1 != banding_pairs.end(); g1++)
		{
			for(auto g2 = banding_pairs[g1->first].begin(); g2 != g1->second.end(); g2++)
			{
				banding_filter_count++;
				if( 
					(
						candidate_pairs.find(g1->first)!=candidate_pairs.end() 
						&& 
						candidate_pairs[g1->first].find(*g2)!=candidate_pairs[g1->first].end()
					) 
						|| 
					(
						candidate_pairs.find(*g2)!=candidate_pairs.end() 
						&& 
						candidate_pairs[*g2].find(g1->first)!=candidate_pairs[*g2].end()
					)
				)
				{
					final_candidate_pairs[g1->first].insert(*g2);
					final_cand_pair_count++;
				}
			}
		}
		cout << "Banding Technique filter candidates pair count: " << banding_filter_count << endl;
		cout << "Final candidates pair count: " << final_cand_pair_count << endl;

		// Writing statistics to the stat file
		ofstream stat_file("./"+res_dir+"/stat_file.txt");
		stat_file.open("./"+res_dir+"/stat_file.txt",ios::app);
		stat_file << "Banding Technique filter candidates pair count: " << banding_filter_count << endl;
		stat_file << "Final candidates pair count: " << final_cand_pair_count << endl;
		stat_file.close();
		
		return final_candidate_pairs;
}


vector<int> printingAndWritingInitialStatistics(int dataset_size,int SHINGLESIZE,int choice,double simScore_threshold,const char* path,const string res_dir,const char* bandspath,const char* rowspath)
{
    cout << "GSimJoin: Sequence Similarity(filters)" << endl;
	cout << "Dataset size: " << dataset_size << endl;
	cout << "Shingle Size: " << SHINGLESIZE << endl;
	cout << "Choice of filter: " << choice << endl;
	cout << "Threshold: " << simScore_threshold << endl;
	cout << "Input file: " << path <<endl;
	cout << "Results Directory: " << res_dir <<endl;

	// Writing statistics to the stat file
	ofstream stat_file("./"+res_dir+"/stat_file.txt");
	stat_file.open("./"+res_dir+"/stat_file.txt",ios::app);
	stat_file << "GSimJoin: Sequence Similarity(filters)" << endl;
	stat_file << "Dataset size: " << dataset_size << endl;
	stat_file << "Shingle size: " << SHINGLESIZE << endl;
	stat_file << "Similarity Score Threshold: " << simScore_threshold << endl;
	stat_file << "Filter choice: " << choice << endl;
       stat_file.close();


        int BANDS,ROWS;
	if(choice == 3)
	{
		BANDS = stoi(bandspath);
		ROWS = stoi(rowspath);
		cout << "Bands: " << BANDS <<endl;
		cout << "Rows: " << ROWS <<endl;
		ofstream stat_file("./"+res_dir+"/stat_file.txt");
	        stat_file.open("./"+res_dir+"/stat_file.txt",ios::app);
		stat_file << "No. of Bands: " << BANDS << endl;
		stat_file << "No. of Rows: " << ROWS << endl;
		stat_file.close();
	}

     vector<int> v;
     v.push_back(BANDS);
     v.push_back(ROWS);

     return v; 

}


void printingAndWritingFinalStatistics(const string res_dir,int timetaken,vector<int>& global_score_freq)
{
    cout << "Memory used: " << memoryUsage() << " MB" << endl; // for displaying total memory usage (in MB)
	cout << "Total Time Taken: "<< timetaken << " milliseconds" << endl;
    
	// Writing statistics to the stat file
	ofstream stat_file("./"+res_dir+"/stat_file.txt");
	stat_file.open("./"+res_dir+"/stat_file.txt",ios::app);
	stat_file << "Memory used: " << memoryUsage() << " MB" << endl;
	stat_file << "Total Time Taken: "<< timetaken  << " milliseconds" << endl;
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
	// Verifying args
	if(argc < 8)
		usage();

	int choice = stoi(argv[4]); // Choice of filter

	if(choice==1 || choice==2)
		if(argc!=8)
			usage();
	else if(choice == 3)
		if(argc!=10)
			usage();
	
	SHINGLESIZE = stoi(argv[3]); // size of each shingle
	double simScore_threshold = stod(argv[5]); // threshold to write only those graph pairs to all_graph_file.txt
	int dataset_size = stoi(argv[argc-2]); // dataset-size
	const string res_dir = argv[argc-1]; // directory in which all stat files would be stored
	mkdir(res_dir.c_str(),0777);
	vector<Graph> graph_dataset; // Input graph dataset

	vector<bool> candidate_graphs; // pruned graphs
	unordered_map<unsigned, unordered_set<unsigned> > candidate_pairs; // candidate pairs after filters 1 & 2
	unordered_map<unsigned, unordered_set<unsigned> > banding_pairs; // candidate pairs after banding technique
	unordered_map<unsigned, unordered_set<unsigned> > final_candidate_pairs; // final candidate pairs from filters
	unordered_map<unsigned, unordered_set<unsigned> > final_our_candidate_pairs;
	
	vector<int> v1 = printingAndWritingInitialStatistics(dataset_size,SHINGLESIZE,choice,simScore_threshold,argv[1],res_dir,argv[6],argv[7]);
    
        int BANDS = v1[0];
        int ROWS  = v1[1];

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr<<"Unable to open the given input file"<<endl;
		exit(0);
	}
	// Parsing Graph Dataset
	parseGraphDataset(dataset_file, graph_dataset, dataset_size);
	cout<< "Parsing of input graph dataset is done." <<endl; 

	candidate_graphs.assign(graph_dataset.size(), false);
	
	ifstream pagerank_file(argv[2]);
	if(!pagerank_file.is_open())
	{
		cerr << "Unable to open the pagerank file." << endl;
		exit(0);
	}
	// Pageranks must be stored in same order of graphs as the input graph dataset file
	loadPageRanks(pagerank_file, graph_dataset, dataset_size); // Parsing Pageranks
	cout<<"Parsing of Pageranks is done."<<endl;

	sort(graph_dataset.begin(), graph_dataset.end(), graphComp); // sort graph dataset wrt vertex count

	// sort each graph's vertex-set based on vertex ids
	for(int g_ind = 0; g_ind < dataset_size; g_ind++)
	{
		sort(graph_dataset[g_ind].vertices.begin(), graph_dataset[g_ind].vertices.end(), vertexComp2);
		for(int vtx_ind = 0; vtx_ind < graph_dataset[g_ind].vertexCount; vtx_ind++)
			graph_dataset[g_ind].vid_to_ind[graph_dataset[g_ind].vertices[vtx_ind].vid] = vtx_ind;
	}

	// timestamping start time
	chrono::high_resolution_clock::time_point cl0 = chrono::high_resolution_clock::now();
	
	// Applies Loose size and Strong size filter to input graph dataset
	applyFilters(graph_dataset, candidate_graphs, candidate_pairs, dataset_size, choice, simScore_threshold, res_dir);
	cout << "Loose-size and Common-vertex Filters are done." <<endl;

	// Preprocesses the pruned graph pairs 
	preProcess(graph_dataset, candidate_graphs); 
	cout<<"Preprocessing is done."<<endl;
	
	// Freq of simScore with range of 1% 0-1, 1-2, 1-3, ... 99-100% 
	vector<int> global_score_freq(102, 0);
			
	if(choice == 3)
	{
		/*Banding Technique*/
		// Applies filter using Banding Technique
		bandingTech(graph_dataset, candidate_graphs, banding_pairs, BANDS, ROWS);
		cout<<"Banding Technique filter is done." <<endl;
        
		//final_candidate_pairs obtained after the execution of below function will be the final candidate_pairs we need for computing SequenceSimilarity
		final_our_candidate_pairs = computeFinalCandidatePairs(banding_pairs, final_candidate_pairs, candidate_pairs,res_dir);
		
		// Computing naive sequence similarity
		computeSequenceSimilarity(graph_dataset, final_our_candidate_pairs, global_score_freq, simScore_threshold, res_dir);
	}	
	else
	{
		// Computing naive sequence similarity
		computeSequenceSimilarity(graph_dataset, candidate_pairs, global_score_freq, simScore_threshold, res_dir);
	}
	cout << "Similarity Computation is done" << endl;

	// timestamping end time
	chrono::high_resolution_clock::time_point cl1=chrono::high_resolution_clock::now();	
        int timetaken = (clocksTosec(cl0,cl1));
    
    printingAndWritingFinalStatistics(res_dir,timetaken,global_score_freq);
    
	return 0;
}

// parses the input graph dataset
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset, int &dataset_size)
{
	int size;
	dataset_file >> size; // input-file dataset size
	if(dataset_size==-1)
		dataset_size = size; // use the whole dataset
	graph_dataset.resize(dataset_size);
	for(auto g_iter=graph_dataset.begin();g_iter<graph_dataset.end();g_iter++)
	{
		g_iter->readGraph(dataset_file);
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

// vertex comparator based on vertex ids
bool vertexComp2(Vertex &v1, Vertex &v2)
{
	return v1.vid < v2.vid;
}

// final sequence similarity computation of pruned pairs
void computeSequenceSimilarity(vector<Graph> &graph_dataset, unordered_map<unsigned, unordered_set<unsigned> > &final_cand_pairs, vector<int> &global_score_freq, double simScore_threshold, string res_dir)
{
	unsigned long long simPairCount = 0;

	ofstream all_graph_file("./" + res_dir + "/all_graph_pair.txt");
	for(auto g1 = final_cand_pairs.begin(); g1 != final_cand_pairs.end(); g1++)
	{
		for(auto g2 = final_cand_pairs[g1->first].begin(); g2 != final_cand_pairs[g1->first].end(); g2++)
		{
			double simScore = computeSimilarity(graph_dataset[g1->first], graph_dataset[*g2]);
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
				all_graph_file << graph_dataset[g1->first].gid << " " << graph_dataset[*g2].gid << " " << simScore << endl;
			}
		}
	}	
	cout << "Similar Graphs: " << simPairCount << endl;
	all_graph_file.close();

	// Writing statistics to the stat file
	ofstream stat_file("./"+res_dir+"/stat_file.txt",ios::app);
	stat_file << "Similar Graphs: " << simPairCount << endl;
	stat_file.close();
}

// graph comparator for sorting graph dataset
bool graphComp(Graph &g1, Graph &g2)
{
	return g1.vertexCount < g2.vertexCount;
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
