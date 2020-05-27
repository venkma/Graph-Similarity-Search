#include "veo.h"

using namespace std;

// For parsing the input graph dataset
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, int &dataset_size_R, int &dataset_size_S);

// Sorts vertex and edge set of graph dataset
void sortGraphDataset(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S);

// Returns the time from start to end in seconds
unsigned long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end);

// Displays the memory used by the program(in MB)
double memoryUsage();

// prints correct usage of program in case of an error
void usage(); 

// $ ./naive inp-file simScore_threshold dataset-size-R dataset-size-S res-file

int main(int argc, char const *argv[])
{
	if(argc!=6)
		usage();

	vector<Graph> graph_dataset_R; // input graph dataset R
	vector<Graph> graph_dataset_S; // input graph dataset S

	double simScore_threshold = stod(argv[2]);  // threshold to write only those graph pairs to all_graph_file.txt

	int dataset_size_R = stoi(argv[3]); // size of input dataset R
	int dataset_size_S = stoi(argv[4]); // size of input dataset S

	const string res_dir = argv[5]; // directory in which all stat files would be stored
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
	cout << "All graphs in dataset sorted." << endl;

	ofstream all_graph_file("./"+res_dir+"/all_graph_file.txt");
	all_graph_file.close();

	// Result-set for each graph as vector of other graph's gid and their similarity score as double
	vector<pair<unsigned, double>> g_res;
	vector<long long int> g_time(graph_dataset_R.size());
	// Freq of simScore with range of 1% 0-1, 1-2, 1-3, ... 99-100% 
	vector<int> score_freq(102,0);
	vector<long long int> global_score_freq(102, 0);
	long long int global_time = 0;
	double simScore;
	unsigned long long simPairCount = 0; 

	// For clock-time calculation
	chrono::high_resolution_clock::time_point cl0 = chrono::high_resolution_clock::now();

	for(int gR = 0; gR < graph_dataset_R.size(); gR++)
	{
		
		chrono::high_resolution_clock::time_point clTemp0 = chrono::high_resolution_clock::now();
		for(int gS = 0; gS < graph_dataset_S.size(); gS++)
		{
			// Similarity Calculation...
			double common = 0;
			simScore = computeSimilarity(graph_dataset_R[gR], graph_dataset_S[gS], common);

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
			if(simScore >= simScore_threshold)
			{
				g_res.push_back(make_pair(graph_dataset_S[gS].gid, simScore));
				simPairCount++;
			}
		}
		
		// For clock-time calculation
		chrono::high_resolution_clock::time_point clTemp1 = chrono::high_resolution_clock::now();

		g_time[gR] = clocksTosec(clTemp0,clTemp1); // graph's similarity calculation time
		global_time += g_time[gR]; // dataset's similarity calculation time

		// Creating Result Files for graph g1
		ofstream gfile;
		all_graph_file.open("./"+res_dir+"/all_graph_file.txt",ios::app);
		gfile.open("./"+res_dir+"/graph_details/g_"+to_string(gR)+"_"+to_string(graph_dataset_R[gR].gid)+"_sim.txt");

		// Writing the result-set for each graph to the file for each graph
		gfile << g_res.size() << endl;
		for(auto g_iter = g_res.begin(); g_iter != g_res.end(); g_iter++)
		{
			gfile << graph_dataset_R[gR].gid << " " << g_iter->first << " " << g_iter->second << endl;
			all_graph_file << graph_dataset_R[gR].gid << " " << g_iter->first << " " << g_iter->second << endl;
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
		gfile << g_time[gR] << endl;
		gfile << global_time << endl;
		gfile.close();
	}

	chrono::high_resolution_clock::time_point cl1 = chrono::high_resolution_clock::now();

	cout << "GSimRSJoin: VEO Similarity(naive)" << endl;
	cout << "Dataset size of R: " << dataset_size_R << endl;
	cout << "Dataset size of S: " << dataset_size_S << endl;
	cout << "Similarity Score Threshold: " << simScore_threshold << endl;
	cout << "Similarity pairs: " << simPairCount << endl;
	cout << "Memory used: " << memoryUsage()  << " MB" << endl;
	cout << "Total Time Taken: "<< (clocksTosec(cl0,cl1)) << " milliseconds" << endl;

	// Writing statistics to the final stat file
	ofstream stat_file("./"+res_dir+"/stat_file.txt");
	stat_file << "GSimRSJoin: VEO Similarity(naive)" << endl;
	stat_file << "Dataset size of R: " << dataset_size_R << endl;
	stat_file << "Dataset size of S: " << dataset_size_S << endl;
	stat_file << "Similarity Score Threshold: " << simScore_threshold << endl;
	stat_file << "Similarity pairs: " << simPairCount << endl;
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
	cerr << "usage: ./naive inp-file simScore_threshold dataset-size-R dataset-size-S res-file" << endl;
	exit(0);
}

