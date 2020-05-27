#include "common.h"
#include "graph.h"


// For parsing the input graph dataset
void parseGraphDataset(ifstream &dataset_file, vector<Graph> &graph_dataset, int &dataset_size); 

// Displays the memory used by the program(in MB)
double memoryUsage();

// Returns the time from start to end in seconds
long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end); 

// prints correct usage of program in case of an error
void usage(); 

// $ ./naive inp-file dataset-size graph-id

int main(int argc, char const *argv[])
{
	if(argc!=4)
		usage();
	
	int dataset_size = stoi(argv[2]); // dataset-size
	int graph_id = stoi(argv[3]); // dataset-size
	vector<Graph> graph_dataset; // Input graph dataset

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr << "Unable to open the dataset file." << endl;
		exit(0);
	}
	parseGraphDataset(dataset_file, graph_dataset, dataset_size); // dataset parsed	
	cout << "Parsing graph-dataset successfull." << endl; 

	for(int g_ind = 0; g_ind < dataset_size; g_ind++)
	{
		if(graph_dataset[g_ind].gid == graph_id)
		{
			cout << "Graph index: " << g_ind << endl;
			graph_dataset[g_ind].displayGraph();
			return 0;
		}
	}
	cout << "Graph not found" << endl;
	return 0;
}

// prints correct usage of program in case of an error
void usage()
{
	cerr << "usage: ./naive path/dataset_filename dataset_size graph_id" <<endl;
	exit(0);
}

// Returns the time from start to end in seconds
long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end)
{
	return (long long int)(1e-6*chrono::duration_cast<chrono::nanoseconds>(end - start).count());
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
