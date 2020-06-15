#include "veo.h"

using namespace std;

// For parsing the input graph dataset
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset, Graph &query_graph, int &dataset_size, int &query_index);

// Sorts vertex and edge set of each graph in the  dataset
void sortGraphDataset(vector<Graph> &graph_dataset, Graph &query_graph); 

// Graph comparator
bool graphComp(Graph &g1, Graph &g2);

// Returns the time from start to end in milliseconds
unsigned long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end);

// Displays the memory used by the program(in MB)
double memoryUsage();
	
// prints correct usage of program in case of an error
void usage(); 

//
void computeSequenceSimilarity(vector<Graph> &graph_dataset,int g2,double simScore,double simScore_threshold,vector<int>& global_score_freq,vector<pair<unsigned, double>> &g_res,
unsigned long &simPairCount);


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

// loose:   ./filter inp_file 1 simScore_threshold dataset-size query_index res-file
// strict:  ./filter inp_file 2 simScore_threshold mismatch dataset-size query_index res-file
//									     		  false/true : 0/1
// static:  ./filter inp_file 3 simScore_threshold mismatch noofbuckets dataset-size query_index res-file
// dynamic: ./filter inp_file 4 simScore_threshold mismatch noofbuckets dataset-size query_index res-file

void printingAndWritingInitialStatistics(int choice,double simScore_threshold,int dataset_size,const string res_dir,bool mismatch,int no_of_buckets)
{

        cout << "GSimSearch: VEO Similarity(filters)" << endl;
	cout << "Choice: " << choice << endl;
	cout << "Similarity Score Threshold: " << simScore_threshold << endl;
	cout << "Dataset Size: " << dataset_size << endl;

	ofstream stat_file(res_dir+"/stat_file.txt");
	stat_file << "GSimSearch: VEO Similarity(filters)" << endl;
	stat_file << "Choice: " << choice << endl;
	stat_file << "Similarity Score Threshold: " << simScore_threshold << endl;
	stat_file << "Dataset Size: " << dataset_size << endl;
	
	if(choice >= 2)
	{
		cout << "Mismatch: " << mismatch << endl;
		cout << "No of Buckets: " << no_of_buckets << endl;
		stat_file << "Mismatch: " << mismatch << endl;
		stat_file << "No of Buckets: " << no_of_buckets << endl;
	}

}
void printingAndWritingFinalStatistics(int choice,unsigned long looseCount,unsigned long strictCount,unsigned long staticCount,bool isBucket,unsigned long partitionCount,unsigned long dynamicCount,bool mismatch,unsigned long mismatchCount,unsigned long simPairCount,int totalTimeTaken,const string res_dir,vector<int>& global_score_freq,vector<pair<unsigned, double>>& g_res,Graph& query_graph)
{     // Displaying stat file...
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
	
	// Writing counts to stat file
	ofstream stat_file(res_dir+"/stat_file.txt");
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

	ofstream all_graph_file("./"+res_dir+"/all_graph_file.txt");
	// Writing the result-set for each graph to the file for each graph
	for(auto g_iter = g_res.begin(); g_iter != g_res.end(); g_iter++)
	{
		all_graph_file << query_graph.gid << " " << g_iter->first << " " << g_iter->second << endl;
	}
	all_graph_file.close();    
}

int main(int argc, char const *argv[])
{
	if(argc < 7)
		usage();

	vector<Graph> graph_dataset; // input graph dataset
	Graph query_graph; // query-graph

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

	// threshold to write only those graph pairs to all_graph_file.txt
	double simScore_threshold = stod(argv[3]); 
	// size of input dataset
	int dataset_size = stoi(argv[argc-3]);
	// query graph's index in input file dataset
	int query_index = stoi(argv[argc-2]);
	// directory in which all stat files would be stored 
	const string res_dir = argv[argc-1]; 
	mkdir(res_dir.c_str(),0777);

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr << "Unable to open dataset file" << endl;
		exit(0);
	}
	// parsing input graph-dataset
	parseGraphDataset(dataset_file, graph_dataset, query_graph, dataset_size, query_index);
	cout << "Graph Dataset parsed.\n";

	// Sorts vertex and edge set of each graph in the dataset
	sortGraphDataset(graph_dataset, query_graph);

	// sorts the graph dataset
	sort(graph_dataset.begin(), graph_dataset.end(), graphComp);
	cout << "Graph Dataset sorted.\n";

	unsigned long looseCount = 0;
	unsigned long strictCount = 0;
	unsigned long staticCount = 0;
	unsigned long dynamicCount = 0;
	unsigned long mismatchCount = 0;
	unsigned long partitionCount = 0;
	unsigned long simPairCount = 0;
	bool out = false;
	double simScore; 

       printingAndWritingInitialStatistics(choice,simScore_threshold,dataset_size,res_dir,mismatch,no_of_buckets);

	VEO veo_sim = VEO(simScore_threshold);

	// static/dyanmic partition filter
	if(choice > 2) 
		veo_sim.index(graph_dataset, query_graph, choice, isBucket, no_of_buckets); // index input graphs
	
	// Result-set for each graph as vector of other graph's gid and their similarity score as double
	vector<pair<unsigned, double>> g_res;
	// Freq of simScore with range of 1% 0-1, 1-2, 1-3, ... 99-100% 
	vector<int> global_score_freq(102, 0);

	chrono::high_resolution_clock::time_point cl0 = chrono::high_resolution_clock::now();
	
	unsigned qg_index = lower_bound(graph_dataset.begin(), graph_dataset.end(), query_graph, GraphComparator()) - graph_dataset.begin();
	
	// size of current graph g1
	long double currSize = query_graph.vertexCount + query_graph.edgeCount; 
	//loose bound of PrevSize
	long double minPrevSize = ceil(currSize/(long double)veo_sim.ubound);

	for(int g2 = qg_index-1; g2 >= 0; g2--)
	{
		double common = 0;
		out = false;
		// size of current graph g2
		long double PrevSize = graph_dataset[g2].vertexCount + graph_dataset[g2].edgeCount; 

		// loose filter
		if(PrevSize >= minPrevSize)	
			looseCount++;
		else
			break;

		if(choice > 1)
		{
			double minIntersection = min(query_graph.vertexCount, graph_dataset[g2].vertexCount) + min(query_graph.edgeCount, graph_dataset[g2].edgeCount);
			// strict bound
			double strictBound = (double)200.0*minIntersection/(currSize+PrevSize); 

			//strict filter
			if(simScore_threshold <= strictBound) 
				strictCount++;
			else
				continue;
		}
		if(choice == 3)
			out = veo_sim.indexFilter(graph_dataset[g2], query_graph, g2, choice, isBucket, no_of_buckets, staticCount, partitionCount, simScore_threshold);
		if(choice == 4)
			out = veo_sim.indexFilter(graph_dataset[g2], query_graph, g2, choice, isBucket, no_of_buckets, dynamicCount, partitionCount, simScore_threshold);
		if(out)
			continue;
		else if(mismatch) 
		{
			// mismatching filter
			out = veo_sim.mismatchingFilter(graph_dataset[g2], query_graph, common, simScore_threshold);
			if(!out)
				mismatchCount++;
		}
		if(!out)
		{
			// naive computation of VEO similarity
			simScore = veo_sim.computeSimilarity(query_graph, graph_dataset[g2], common);
			computeSequenceSimilarity(graph_dataset,g2,simScore,simScore_threshold,global_score_freq,g_res,simPairCount);
		}

	}

	for(int g2 = qg_index+1; g2 < dataset_size; g2++)
	{
		double common = 0;
		out = false;
		// size of next graph g2
		long double nextSize = graph_dataset[g2].vertexCount + graph_dataset[g2].edgeCount; 
		//loose bound of currSize
		long double minCurrSize = ceil(currSize/(long double)veo_sim.ubound);

		// loose filter
		if(currSize >= minCurrSize)
			looseCount++;
		else
			break;

		if(choice > 1)
		{
			double minIntersection = min(query_graph.vertexCount, graph_dataset[g2].vertexCount) + min(query_graph.edgeCount, graph_dataset[g2].edgeCount);
			// strict bound
			double strictBound = (double)200.0*minIntersection/(currSize+nextSize); 

			//strict filter
			if(simScore_threshold <= strictBound) 
				strictCount++;
			else
				continue;
		}
		if(choice == 3)
			out = veo_sim.indexFilter(graph_dataset[g2], query_graph, g2, choice, isBucket, no_of_buckets, staticCount, partitionCount, simScore_threshold);
		if(choice == 4)
			out = veo_sim.indexFilter(graph_dataset[g2], query_graph, g2, choice, isBucket, no_of_buckets, dynamicCount, partitionCount, simScore_threshold);
			
		if(out)
			continue;
		else if(mismatch) 
		{
			// mismatching filter
			out = veo_sim.mismatchingFilter(graph_dataset[g2], query_graph, common, simScore_threshold);
			if(!out)
				mismatchCount++;
		}

		if(!out)
		{
			
			// naive computation of VEO similarity
			simScore = veo_sim.computeSimilarity(query_graph, graph_dataset[g2], common);
			computeSequenceSimilarity(graph_dataset,g2,simScore,simScore_threshold,global_score_freq,g_res,simPairCount);
		}

	}	
	chrono::high_resolution_clock::time_point cl2=chrono::high_resolution_clock::now();	
        int totalTimeTaken = (clocksTosec(cl0,cl2));
	printingAndWritingFinalStatistics(choice,looseCount,strictCount,staticCount,isBucket,partitionCount,dynamicCount,mismatch,mismatchCount,simPairCount,totalTimeTaken,res_dir,global_score_freq,g_res,query_graph);
       
	
	
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
void computeSequenceSimilarity(vector<Graph> &graph_dataset,int g2,double simScore,double simScore_threshold,vector<int>& global_score_freq,vector<pair<unsigned, double>> &g_res,
unsigned long &simPairCount)
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
				g_res.push_back(make_pair(graph_dataset[g2].gid, simScore));
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
	cerr << "usage: ./filter input_file choice simScore-threshold dataset-size query_index res-dir" <<endl;

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

	cerr << "loose:   ./filter inp_file 1 simScore_threshold dataset-size query_index res-dir\n";
	cerr << "strict:  ./filter inp_file 2 simScore_threshold mismatch dataset-size query_index res-dir\n";
//		     											  false/true : 0/1
	cerr << "static:  ./filter inp_file 3 simScore_threshold mismatch noofbuckets dataset-size query_index res-dir\n";
	cerr << "dynamic: ./filter inp_file 4 simScore_threshold mismatch noofbuckets dataset-size query_index res-dir\n";
	exit(0);
}
