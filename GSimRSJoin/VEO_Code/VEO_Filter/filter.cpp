#include "veo.h"

using namespace std;

// For parsing the input graph dataset
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, int &dataset_size_R, int &dataset_size_S);

// Sorts vertex and edge set of graph dataset
void sortGraphDataset(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S);

//computeSimilarity
void computeSequenceSimilarity(vector<Graph> &graph_dataset_R,int gR,vector<Graph> &graph_dataset_S,int gS,double simScore,double simScore_threshold,vector<long long int>& global_score_freq,vector<pair<pair<unsigned, unsigned>, double>> &g_res,unsigned long long &simPairCount);

// Graph comparator
bool graphComp(Graph &g1, Graph &g2);

// Returns the time from start to end in milliseconds
unsigned long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end);

// Displays the memory used by the program(in MB)
double memoryUsage();
	
// prints correct usage of program in case of an error
void usage(); 

struct GraphComparator
{
	// Compare 2 Player objects using name
	bool operator ()(const Graph &g1,const Graph &g2)
	{
		if((g1.vertexCount+g1.edgeCount) == (g2.vertexCount+g2.edgeCount))
			return g1.gid < g2.gid;
		return (g1.vertexCount+g1.edgeCount) < (g2.vertexCount+g2.edgeCount);
	}
};

// loose:   ./filter inp_file 1 simScore_threshold dataset-size_R dataset-size_S res-file
// strict:  ./filter inp_file 2 simScore_threshold mismatch dataset-size_R dataset-size_S res-file
//									     		  false/true : 0/1
// static:  ./filter inp_file 3 simScore_threshold mismatch noofbuckets dataset-size_R dataset-size_S res-file
// dynamic: ./filter inp_file 4 simScore_threshold mismatch noofbuckets dataset-size_R  dataset-size_S res-file


void printingAndWritingInitialStatistics(int choice,double simScore_threshold,int dataset_size_R,int dataset_size_S,const string res_dir,bool mismatch,int no_of_buckets)
{
    
	cout << "GSimRSJoin: VEO Similarity(filters)" << endl;
	cout << "Choice: " << choice << endl;
	cout << "Similarity Score Threshold: " << simScore_threshold << endl;
	cout << "Dataset Size of R: " << dataset_size_R << endl;
	cout << "Dataset Size of S: " << dataset_size_S << endl;

	ofstream stat_file(res_dir+"/stat_file.txt");
	stat_file << "GSimSearch: VEO Similarity(filters)" << endl;
	stat_file << "Choice: " << choice << endl;
	stat_file << "Similarity Score Threshold: " << simScore_threshold << endl;
	stat_file << "Dataset Size of R: " << dataset_size_R << endl;
	stat_file << "Dataset Size of S: " << dataset_size_S << endl;
	
	if(choice >= 2)
	{
		cout << "Mismatch: " << mismatch << endl;
		cout << "No of Buckets: " << no_of_buckets << endl;
		stat_file << "Mismatch: " << mismatch << endl;
		stat_file << "No of Buckets: " << no_of_buckets << endl;
	}
    
}

void printingAndWritingFinalStatistics(int choice,unsigned long looseCount,unsigned long strictCount,unsigned long staticCount,bool isBucket,unsigned long partitionCount,unsigned long dynamicCount,bool mismatch,unsigned long mismatchCount,unsigned long simPairCount,int totalTimeTaken,const string res_dir,vector<long long int>& global_score_freq,vector<pair<pair<unsigned, unsigned>, double>>& g_res)                 
{
	// Displaying stat file...
	if(choice >= 1)
		cout << "Loose Filter Count: " << looseCount << endl;
	if(choice >= 2)
		cout << "Strict Filter Count: " << strictCount << endl;
	if(choice == 3)
	{
		cout << "Static Filter Count: " << staticCount << endl;
		if(isBucket)
			cout << "Partiiton Filter Count: " << partitionCount << endl;
	}
	if(choice == 4)
	{
		cout << "Dynamic Filter Count: " << dynamicCount << endl;
		if(isBucket)
			cout << "Partiiton Filter Count: " << partitionCount << endl;
	}
	if(mismatch)
		cout << "Mismatch Filter Count: " << mismatchCount << endl;
	cout << "Final Similar Pair Count: " << simPairCount << endl;
	cout << "Memory used: " << memoryUsage() << " MB" << endl;
	cout <<"Total Time Taken: "<< totalTimeTaken << " milliseconds" << endl;

        ofstream stat_file(res_dir+"/stat_file.txt");
	// Writing counts to stat file
	if(choice >= 1)
		stat_file << "Loose Filter Count: " << looseCount << endl;
	if(choice >= 2)
		stat_file << "Strict Filter Count: " << strictCount << endl;
	if(choice == 3)
	{
		stat_file << "Static Filter Count: " << staticCount << endl;
		if(isBucket)
			stat_file << "Partiiton Filter Count: " << partitionCount << endl;
	}
	if(choice == 4)
	{
		stat_file << "Dynamic Filter Count: " << dynamicCount << endl;
		if(isBucket)
			stat_file << "Partiiton Filter Count: " << partitionCount << endl;
	}
	if(mismatch)
		stat_file << "Mismatch Filter Count: " << mismatchCount << endl;
	stat_file << "Final Similar Pair Count: " << simPairCount << endl;

	stat_file << "Memory used: " << memoryUsage() << " MB" << endl;
	stat_file <<"Total Time Taken: "<< totalTimeTaken << " milliseconds" << endl;
	stat_file.close();
	
	ofstream freq_file("./"+res_dir+"/freq_distr_file.txt");
	// for simScore==0
	freq_file << "0 " << global_score_freq[0] << endl; 
	for(int i=1; i<101; i++)
		freq_file << i << " " << global_score_freq[i] << endl;
	// for simScore==100
	freq_file << "101 " << global_score_freq[101] << endl; 
	freq_file.close();

	// Writing the result-set for each graph pair to the all-graph-file
        ofstream all_graph_file("./"+res_dir+"/all_graph_file.txt");
	all_graph_file.open("./"+res_dir+"/all_graph_file.txt");	
	for(auto g_iter = g_res.begin(); g_iter != g_res.end(); g_iter++)
	{
		all_graph_file << g_iter->first.first << " " << g_iter->first.second << " " << g_iter->second << endl;
	}
	all_graph_file.close();
}

int main(int argc, char const *argv[])
{
	if(argc < 7)
		usage();

	vector<Graph> graph_dataset_R; // input graph dataset R
	vector<Graph> graph_dataset_S; // input graph dataset S

	// applying mismatch filter 
	bool mismatch=false; 
	// no. of buckets used in dynamic filter
	int no_of_buckets=0; 
	// true if no. of buckets is greater than 0 
	bool isBucket=false; 

	int choice = stoi(argv[2]);
	
	// Verifying args
	if(choice==1)
	{
		if(argc!=7)
			usage();
	}
	else if(choice==2)
	{
		if(argc!=8)
			usage();
		mismatch = (stoi(argv[4])==1);
	}
	else if(choice > 2)
	{
		if(argc!=9)
			usage();
		mismatch = (stoi(argv[4])==1);
		no_of_buckets = stoi(argv[5]);
		isBucket = (no_of_buckets > 0);
	}
	else
		usage();

	double simScore_threshold = stod(argv[3]);  // threshold to write only those graph pairs to all_graph_file.txt
	int dataset_size_R = stoi(argv[argc-3]); // size of input dataset R
	int dataset_size_S = stoi(argv[argc-2]); // size of input dataset S
	// directory in which all stat files would be stored 
	const string res_dir = argv[argc-1]; 
	mkdir(res_dir.c_str(),0777);
	mkdir((res_dir+"/graph_details/").c_str(),0777);

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr << "Unable to open dataset file" << endl;
		exit(0);
	}
	// parsing input dataset 
	parseGraphDataset(dataset_file, graph_dataset_R, graph_dataset_S, dataset_size_R, dataset_size_S);
	cout << argv[1] << ": Graph Dataset parsed." << endl;

	sortGraphDataset(graph_dataset_R, graph_dataset_S); // to sort vertex and edge set
	cout << "All graphs in datasets R and S sorted." << endl;

	// sorts the graph dataset
	sort(graph_dataset_R.begin(), graph_dataset_R.end(), graphComp);
	sort(graph_dataset_S.begin(), graph_dataset_S.end(), graphComp);
	cout << "Graph Datasets R and S sorted.\n";

	unsigned long long looseCount = 0;
	unsigned long long strictCount = 0;
	unsigned long long staticCount = 0;
	unsigned long long dynamicCount = 0;
	unsigned long long mismatchCount = 0;
	unsigned long long partitionCount = 0;
	unsigned long long simPairCount = 0;
	bool out = false;
	double simScore; 
	
	printingAndWritingInitialStatistics(choice,simScore_threshold,dataset_size_R,dataset_size_S,res_dir,mismatch,no_of_buckets);


	VEO veo_sim = VEO(simScore_threshold);

	// static/dyanmic partition filter
	if(choice > 2) 
		veo_sim.index(graph_dataset_R, graph_dataset_S, choice, isBucket, no_of_buckets); // index input graphs

	ofstream gfile, all_graph_file("./"+res_dir+"/all_graph_file.txt");
	all_graph_file.close();
	
	// Result-set for each graph as vector of other graph's gid and their similarity score as double
	vector<pair<pair<unsigned, unsigned>, double>> g_res;
	unsigned long long int global_time = 0;

	// Freq of simScore with range of 1% 0-1, 1-2, 1-3, ... 99-100% 
	vector<long long int> global_score_freq(102, 0);

	chrono::high_resolution_clock::time_point cl0 = chrono::high_resolution_clock::now();
	
	for(int gR = 0; gR < dataset_size_R; gR++)
	{
		unsigned gS_index = lower_bound(graph_dataset_S.begin(), graph_dataset_S.end(), graph_dataset_R[gR], GraphComparator()) - graph_dataset_S.begin();
		// size of current graph g1
		long double currSize = graph_dataset_R[gR].vertexCount + graph_dataset_R[gR].edgeCount; 
		//loose bound of PrevSize
		long double minPrevSize = ceil(currSize/(long double)veo_sim.ubound);
		for(int gS = gS_index; gS >= 0 && gS < dataset_size_S; gS--)
		{
			double common = 0;
			out = false;
			// size of current graph g2
			long double PrevSize = graph_dataset_S[gS].vertexCount + graph_dataset_S[gS].edgeCount; 

			// loose filter
			if(PrevSize >= minPrevSize)	
				looseCount++;
			else
				break;

			if(choice > 1)
			{
				double minIntersection = min(graph_dataset_R[gR].vertexCount, graph_dataset_S[gS].vertexCount) + min(graph_dataset_R[gR].edgeCount, graph_dataset_S[gS].edgeCount);
				// strict bound
				double strictBound = (double)200.0*minIntersection/(currSize+PrevSize); 
				//strict filter
				if(simScore_threshold <= strictBound) 
					strictCount++;
				else
					continue;
			}
			if(choice == 3)
				out = veo_sim.indexFilter(graph_dataset_R[gR], graph_dataset_S[gS], gR, gS, choice, isBucket, no_of_buckets, staticCount, partitionCount, simScore_threshold);
			if(choice == 4)
				out = veo_sim.indexFilter(graph_dataset_R[gR], graph_dataset_S[gS], gR, gS, choice, isBucket, no_of_buckets, dynamicCount, partitionCount, simScore_threshold);
			
			if(out)
				continue;
			else if(mismatch) 
			{
				// mismatching filter
				out = veo_sim.mismatchingFilter(graph_dataset_R[gR], graph_dataset_S[gS], common, simScore_threshold);
				if(!out)
					mismatchCount++;
			}

			if(!out)
			{
				// naive computation of VEO similarity
				simScore = veo_sim.computeSimilarity(graph_dataset_R[gR], graph_dataset_S[gS], common);
				computeSequenceSimilarity(graph_dataset_R,gR,graph_dataset_S,gS,simScore,simScore_threshold,global_score_freq,g_res,simPairCount);
			}

		}
		for(int gS = gS_index+1; gS < dataset_size_S; gS++)
		{

			double common = 0;
			out = false;
			// size of next graph g2
			long double nextSize = graph_dataset_S[gS].vertexCount + graph_dataset_S[gS].edgeCount; 
			//loose bound of currSize
			long double minCurrSize = ceil(currSize/(long double)veo_sim.ubound);

			// loose filter
			if(currSize >= minCurrSize)
				looseCount++;
			else
				break;

			if(choice > 1)
			{ 
				double minIntersection = min(graph_dataset_R[gR].vertexCount, graph_dataset_S[gS].vertexCount) + min(graph_dataset_R[gR].edgeCount, graph_dataset_S[gS].edgeCount);
				// strict bound
				double strictBound = (double)200.0*minIntersection/(currSize+nextSize); 

				//strict filter
				if(simScore_threshold <= strictBound) 
					strictCount++;
				else
					continue;
			}
			if(choice == 3)
				out = veo_sim.indexFilter(graph_dataset_R[gR], graph_dataset_S[gS], gR, gS, choice, isBucket, no_of_buckets, staticCount, partitionCount, simScore_threshold);
			if(choice == 4)
				out = veo_sim.indexFilter(graph_dataset_R[gR], graph_dataset_S[gS], gR, gS, choice, isBucket, no_of_buckets, dynamicCount, partitionCount, simScore_threshold);
			
			if(out)
				continue;
			else if(mismatch) 
			{
				// mismatching filter
				out = veo_sim.mismatchingFilter(graph_dataset_R[gR], graph_dataset_S[gS], common, simScore_threshold);
				if(!out)
					mismatchCount++;
			}
			if(!out)
			{
				// naive computation of VEO similarity
				simScore = veo_sim.computeSimilarity(graph_dataset_R[gR], graph_dataset_S[gS], common);
				computeSequenceSimilarity(graph_dataset_R,gR,graph_dataset_S,gS,simScore,simScore_threshold,global_score_freq,g_res,simPairCount);
			}

		}

	}	
	chrono::high_resolution_clock::time_point cl1 = chrono::high_resolution_clock::now();
	int totalTimeTaken = (clocksTosec(cl0,cl1));
	
	printingAndWritingFinalStatistics(choice,looseCount,strictCount,staticCount,isBucket,partitionCount,dynamicCount,mismatch,mismatchCount,simPairCount,totalTimeTaken,res_dir,global_score_freq,g_res);
	 
	
	return 0;
}

// parses the input graph dataset and query graph
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

// Sorts vertex and edge set of graph dataset
void sortGraphDataset(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S)
{
	for(int i = 0; i < graph_dataset_R.size(); i++)
	{
		sort(graph_dataset_R[i].vertices.begin(), graph_dataset_R[i].vertices.end()); // sort vertex-set 
		sort(graph_dataset_R[i].edges.begin(), graph_dataset_R[i].edges.end()); // sort edge-set
	}
	for(int i = 0; i < graph_dataset_S.size(); i++)
	{
		sort(graph_dataset_S[i].vertices.begin(), graph_dataset_S[i].vertices.end()); // sort vertex-set 
		sort(graph_dataset_S[i].edges.begin(), graph_dataset_S[i].edges.end()); // sort edge-set
	}
}

void computeSequenceSimilarity(vector<Graph> &graph_dataset_R,int gR,vector<Graph> &graph_dataset_S,int gS,double simScore,double simScore_threshold,vector<long long int>& global_score_freq,vector<pair<pair<unsigned, unsigned>, double>> &g_res,unsigned long long &simPairCount)
{
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

					g_res.push_back(make_pair(make_pair(graph_dataset_R[gR].gid, graph_dataset_S[gS].gid), simScore));
					simPairCount++;
				}
}

bool graphComp(Graph &g1, Graph &g2)
{
	return g1.vertexCount+g1.edgeCount < g2.vertexCount+g2.edgeCount;
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
void usage(){
	cerr << "usage: ./filter input_file choice simScore-threshold dataset-size-R dataset-size-S  res-dir" <<endl;

	cerr << "Available choices: " << endl;
	cerr << endl;

	cerr << "1 loose : 1" << endl;
	cerr << endl;

	cerr << "2 loose + strict 			  : 2, mismatch=false noofbuckets=0" << endl;
	cerr << "3 loose + strict + mismatch  : 2, mismatch=true  noofbuckets=0" << endl;
	cerr << endl;

	cerr << "4 loose + strict + static 				: 3, mismatch=false noofbuckets = 0" << endl;
	cerr << "5 loose + strict + static + 2 buckets  : 3, mismatch=false noofbuckets = 2" << endl;
	cerr << "6 loose + strict + static + 5 buckets  : 3, mismatch=false noofbuckets = 5" << endl;
	cerr << "7 loose + strict + static + 10 buckets : 3, mismatch=false noofbuckets = 10" << endl;
	cerr << "8 loose + strict + static + 10 buckets : 3, mismatch=true  noofbuckets = 10" << endl;
	cerr << "9  loose + strict + dynamic 			  :  4, mismatch=false noofbuckets = 0" << endl;
	cerr << "10 loose + strict + dynamic + 2 buckets  :  4, mismatch=false noofbuckets = 2" << endl;
	cerr << "11 loose + strict + dynamic + 5 buckets  :  4, mismatch=false noofbuckets = 5" << endl;
	cerr << "12 loose + strict + dynamic + 10 buckets :  4, mismatch=false noofbuckets = 10" << endl;
	cerr << "13 loose + strict + dynamic + 10 buckets :  4, mismatch=true  noofbuckets = 10" << endl;
	cerr << endl;

	cerr << "loose:   ./filter inp_file 1 simScore_threshold dataset-size-R dataset-size-S res-dir\n";
	cerr << "strict:  ./filter inp_file 2 simScore_threshold mismatch dataset-size-R dataset-size-S res-dir\n";
//		     											  false/true : 0/1
	cerr << "static:  ./filter inp_file 3 simScore_threshold mismatch noofbuckets dataset-size-R dataset-size-S res-dir\n";
	cerr << "dynamic: ./filter inp_file 4 simScore_threshold mismatch noofbuckets dataset-size-R dataset-size-S res-dir\n";
	exit(0);
}
