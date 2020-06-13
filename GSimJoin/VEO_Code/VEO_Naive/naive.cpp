#include "veo.h"

using namespace std;

// For parsing the input graph dataset
void parseGraphDataset(ifstream &inp, vector<Graph> &graph_dataset, int &dataset_size);

// Sorts vertex and edge set of graph dataset
void sortGraphDataset(vector<Graph> &graph_dataset);

// Returns the time from start to end in seconds
unsigned long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end);

// Displays the memory used by the program(in MB)
double memoryUsage();

// prints correct usage of program in case of an error
void usage(); 

// $ ./naive inp-file simScore_threshold dataset-size res-file


vector<pair<unsigned, double>> creatingResultGraphsandUpdatingGRES(const string res_dir,int g1,vector<Graph> &graph_dataset,vector<pair<unsigned, double>> &g_res,vector<int> &score_freq,vector<unsigned long long int> &g_time,unsigned long long int global_time)
{
		// Creating Result Files for graph g1
		ofstream gfile;
		ofstream all_graph_file;
		all_graph_file.open("./"+res_dir+"/all_graph_file.txt",ios::app);
		gfile.open("./"+res_dir+"/graph_details/g_"+to_string(g1)+"_"+to_string(graph_dataset[g1].gid)+"_sim.txt");

		// Writing the result-set for each graph to the file for each graph
		gfile << g_res.size() << endl;
		for(auto g_iter = g_res.begin(); g_iter != g_res.end(); g_iter++)
		{
			gfile << graph_dataset[g1].gid << " " << g_iter->first << " " << g_iter->second << endl;
			all_graph_file << graph_dataset[g1].gid << " " << g_iter->first << " " << g_iter->second << endl;
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
		gfile << g_time[g1] << endl;
		gfile << global_time << endl;
		gfile.close();
		
		return g_res;
}

void printingAndWritingFinalStatistics(int dataset_size,double simScore_threshold,unsigned long long int simPairCount,unsigned long long int global_time,unsigned long long int totalTimeTaken,const string res_dir,vector<long long int> &global_score_freq)
{
    cout << "GSimJoin: VEO Similarity(naive)" << endl;
	cout << "Dataset size: " << dataset_size << endl;
	cout << "Similarity Score Threshold: " << simScore_threshold << endl;
	cout << "Similar Graph Pairs: " << simPairCount << endl;
	cout << "Memory used: " << memoryUsage() << " MB" << endl;
	cout << "Similarity Time: "<< global_time << " milliseconds" << endl;
	cout << "Total Time Taken: "<< totalTimeTaken  << " milliseconds" << endl;

	ofstream stat_file("./"+res_dir+"/stat_final.txt");
	stat_file << "GSimJoin: VEO Similarity(naive)" << endl;
	stat_file << "Dataset size: " << dataset_size << endl;
	stat_file << "Similarity Score Threshold: " << simScore_threshold << endl;
	stat_file << "Similar Graph Pairs: " << simPairCount << endl;
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

int main(int argc, char const *argv[])
{
	if(argc!=5)
		usage();

	double simScore_threshold = stod(argv[2]);  // threshold to write only those graph pairs to all_graph_file.txt
	int dataset_size = stoi(argv[3]); // size of input dataset
	const string res_dir = argv[4]; // directory in which all stat files would be stored
	mkdir(res_dir.c_str(),0777);
	mkdir((res_dir+"/graph_details/").c_str(),0777);
	vector<Graph> graph_dataset; // input graph dataset

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr << "Unable to open dataset file" << endl;
		exit(0);
	}

	// parsing input dataset 
	parseGraphDataset(dataset_file, graph_dataset, dataset_size);
	cout << argv[1] << ": Graph Dataset parsed." << endl;

	sortGraphDataset(graph_dataset); // to sort vertex and edge set
	cout << "All graphs in dataset sorted." << endl;

	ofstream all_graph_file("./"+res_dir+"/all_graph_file.txt");
	all_graph_file.close();

	// Result-set for each graph as vector of other graph's gid and their similarity score as double
	vector<pair<unsigned, double>> g_res; // stores graph pair with the score of a specific graph
	vector<unsigned long long int> g_time(graph_dataset.size()); // stores time required for each graph in the dataset
	unsigned long long int global_time = 0; // total time taken for similarity computation 
	// Freq of simScore with range of 1% 0-1, 1-2, 1-3, ... 99-100% 
	vector<int> score_freq(102,0); // stores sim-score frequency distribution of a particular graph
	vector<long long int> global_score_freq(102, 0); // stores sim-score frequency distribution of the dataset

	double simScore; // similarity score
	unsigned long long int simPairCount = 0; // no. of graph pairs having similarity score > threshold

 	// timestamping start time
	chrono::high_resolution_clock::time_point cl0 = chrono::high_resolution_clock::now();

	// For clock-time calculation
	chrono::high_resolution_clock::time_point clTemp0, clTemp1; 

	for(int g1 = 1; g1<graph_dataset.size(); g1++)
	{
		clTemp0 = chrono::high_resolution_clock::now();

		for(int g2 = g1-1; g2 >= 0; g2--)
		{
			// Similarity Calculation...
			double common = 0;
			double simScore = computeSimilarity(graph_dataset[g1], graph_dataset[g2], common);
			
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
				g_res.push_back(make_pair(graph_dataset[g2].gid, simScore));
				simPairCount++;
			}
		}

		clTemp1 = chrono::high_resolution_clock::now();
		g_time[g1] = clocksTosec(clTemp0,clTemp1); // graph's similarity calculation time
		global_time += g_time[g1]; // dataset's similarity calculation time
		
             g_res = creatingResultGraphsandUpdatingGRES(res_dir,g1,graph_dataset,g_res,score_freq,g_time,global_time);
	}
 	// timestamping end time
	chrono::high_resolution_clock::time_point cl1=chrono::high_resolution_clock::now();
	unsigned long long int totalTimeTaken = (clocksTosec(cl0,cl1));
	
    printingAndWritingFinalStatistics(dataset_size,simScore_threshold,simPairCount,global_time,totalTimeTaken,res_dir,global_score_freq);
    
	return 0;
}

// For parsing the input graph dataset
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset, int &dataset_size)
{
	int size;
	dataset_file >> size;
	if(dataset_size == -1)
		dataset_size = size;
	graph_dataset.resize(dataset_size);
	for(auto g_iter = graph_dataset.begin(); g_iter != graph_dataset.end(); g_iter++)
		g_iter->readGraph(dataset_file);	
}

// Sorts vertex and edge set of graph dataset
void sortGraphDataset(vector<Graph> &graph_dataset)
{
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
	cerr << "usage: ./naive inp-file simScore_threshold dataset-size res-file" <<endl;
	exit(0);
}
