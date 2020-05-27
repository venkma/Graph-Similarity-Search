#include "common.h"
#include "graph.h"

void usage();
bool graphComp(Graph &g1, Graph &g2);

void parseGraphDataset(ifstream &inp, vector<Graph> &graph_dataset, int dataset_size);

double memoryUsage();
char* currentTime();
long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end); // Returns the time from start to end in seconds

int main(int argc, char const *argv[])
{
	/*verifying args*/
	if(argc!=4)
		usage();

	int dataset_size = stoi(argv[3]);
	vector<Graph> graph_dataset;

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr<<"Unable to open the given input file"<<endl;
		exit(0);
	}

	parseGraphDataset(dataset_file, graph_dataset, dataset_size); 

	cout<< "parsing is done...\n" <<endl; 

	sort(graph_dataset.begin(), graph_dataset.end(), graphComp); /*based on vertex sizes*/

	cout<< "sorting is done...\n" <<endl;

	ofstream sorted_dataset_file(argv[2]);
	
	sorted_dataset_file << graph_dataset.size() << endl;

	for(int g_ind = 0;g_ind < graph_dataset.size(); g_ind++)
	{
		sorted_dataset_file << "g " << graph_dataset[g_ind].vertexCount << " " << graph_dataset[g_ind].edgeCount << " " << graph_dataset[g_ind].gid << endl;
		for(int vtx_ind = 0; vtx_ind < graph_dataset[g_ind].vertexCount; vtx_ind++)
		{
			sorted_dataset_file << "v " << graph_dataset[g_ind].vertices[vtx_ind].vid << endl;
		}
		for(int edge_ind = 0;edge_ind < graph_dataset[g_ind].edgeCount; edge_ind++)
		{
			sorted_dataset_file << "e " << graph_dataset[g_ind].edges[edge_ind].first << " " << graph_dataset[g_ind].edges[edge_ind].second << endl;	
		}
	}
	sorted_dataset_file.close();
	return 0;

}

void parseGraphDataset(ifstream &inp, vector<Graph> &graph_dataset, int dataset_size)
{
	int size;
	inp >> size;
	if(dataset_size==-1)
		dataset_size = size;
	graph_dataset.resize(dataset_size);
	for(auto g_iter=graph_dataset.begin();g_iter<graph_dataset.end();g_iter++)
	{
		g_iter->readGraph(inp);
	}
}

bool graphComp(Graph &g1, Graph &g2)
{
	if(g1.vertexCount == g2.vertexCount)
		return g1.gid < g2.gid;
	return g1.vertexCount < g2.vertexCount;
}


void usage()
{
	cerr << "How to execute:" << endl;
	cerr <<	"./sort_dataset inp_unsorted_file out_sorted_file dataset_size" << endl;
	exit(0);
}

long long int clocksTosec(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end)
{
	return (long long int)(1e-6*chrono::duration_cast<chrono::nanoseconds>(end - start).count());
}

double memoryUsage()
{
	struct rusage r_usage;
	getrusage(RUSAGE_SELF, &r_usage);
	return r_usage.ru_maxrss/1024.0;
}

char* currentTime()
{
	time_t cur_t = time(NULL);
	return ctime(&cur_t);
}
