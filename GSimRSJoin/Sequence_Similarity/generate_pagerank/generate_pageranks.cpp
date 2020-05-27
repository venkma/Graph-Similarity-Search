/*
Compilation:
CPLUS_INCLUDE_PATH=/usr/include/python3.6/
export CPLUS_INCLUDE_PATH
g++ -std=c++11 generate_pageranks.cpp graph.cpp -o naive $(/usr/bin/python3.6-config --ldflags)

*/

#include "common.h"
#include "graph.h"
using namespace std;

wchar_t *program;
PyObject *pythonFunc;

void computePageRank(Graph &g);
void initializePython( const char *execfile);
void exitPython();
void parseGraphDataset(ifstream &inp, vector<Graph> &graph_dataset);
void usage();

int main(int argc, char const *argv[])
{
	/*verifying args*/
	if(argc!=3)
		usage();

	vector<Graph> graph_dataset;

	ifstream dataset_file(argv[1]);
	if(!dataset_file.is_open())
	{
		cerr<<"Unable to open the given input file"<<endl;
		exit(0);
	}

	parseGraphDataset(dataset_file, graph_dataset); 

	cout<< "parsing is done...\n" <<endl; 
	
	ofstream pagerank_file(argv[2]);

	pagerank_file << graph_dataset.size() << endl;

	initializePython(argv[0]);
	cout << "Python initailized\n";

	for(int g_ind = 0; g_ind < graph_dataset.size(); g_ind++)
	{
		computePageRank(graph_dataset[g_ind]);
		pagerank_file << "g " << graph_dataset[g_ind].vertexCount << " " << graph_dataset[g_ind].edgeCount << " " << graph_dataset[g_ind].gid << endl;
		for(int vtx_ind = 0; vtx_ind < graph_dataset[g_ind].vertexCount; vtx_ind++)
		{
			pagerank_file << "v " << graph_dataset[g_ind].vertices[vtx_ind].vid << " " << graph_dataset[g_ind].vertices[vtx_ind].quality << endl;
		}
	}
	pagerank_file.close();

	cout << "PageRanks computed...\n" << endl;

	exitPython();

	return 0;
}
void usage()
{
	cerr << "How to execute:" << endl;
	cerr <<	"./generate_pageranks sorted_dataset_file pagerank_file" << endl;
	exit(0);
}

void parseGraphDataset(ifstream &inp, vector<Graph> &graph_dataset)
{
	int size;
	inp >> size;
	graph_dataset.resize(size);
	for(auto g_iter = graph_dataset.begin(); g_iter < graph_dataset.end(); g_iter++)
		g_iter->readGraph(inp);
}

void initializePython( const char *execfile)
{
	const char *program_name = "pagerank", *function_name = "computePR";

	PyObject *pName, *pModule;

	program = Py_DecodeLocale(execfile, NULL);
	if (program == NULL) 
	{
        cerr << "Fatal error: cannot decode argv[0]" << endl;
        exit(1);
    }
    Py_SetProgramName(program);

    /*initailizing python interpreter*/
    Py_Initialize();

    /*adding currunt path to system*/
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\".\")");

    /*python program as a module*/
    pName = PyUnicode_DecodeFSDefault(program_name);
    pModule = PyImport_Import(pName);

    if(pModule==NULL)
    {
    	PyErr_Print();
    	exit(0);
    }

    pythonFunc =  PyObject_GetAttrString(pModule, function_name);

    if(!PyCallable_Check(pythonFunc)){
    	PyErr_Print();
    	exit(0);
    }
}

void exitPython(){
	Py_Finalize();
    PyMem_RawFree(program);
}
void computePageRank(Graph &g)
{
   	if(g.edgeCount==0)
   	{
		for(int i=0; i < g.vertexCount; i++)
		{
			g.vertices[i].quality = 0;
		}
		return;
	}
	PyObject *vertices, *edges, *e, *pythonArgs, *returnList;

	vertices = PyList_New(g.vertexCount);
	edges = PyTuple_New(2*g.edgeCount);

	int edgeInd = 0;
	/*copying graph*/
	for(int i=0; i < g.vertexCount; i++)
	{
		PyList_SetItem(vertices,i,PyLong_FromLong(g.vertices[i].vid));
		for(int j=0; j < g.vertices[i].edges.size(); j++){
			/*an edge*/
			e = PyTuple_New(2);
			PyTuple_SetItem(e,0,PyLong_FromLong(g.vertices[i].vid));
			PyTuple_SetItem(e,1,PyLong_FromLong(g.vertices[i].edges[j]->vid));
			/*adding e to edges*/
			PyTuple_SetItem(edges,edgeInd++,e);
		}
	}
	pythonArgs = PyTuple_New(2);

	PyTuple_SetItem(pythonArgs,0,vertices);
	PyTuple_SetItem(pythonArgs,1,edges);

	returnList = PyObject_CallObject(pythonFunc, pythonArgs);

	/*copying quality from return list to graph*/
	for(int i=0; i < g.vertexCount; i++)
	{
		 g.vertices[i].quality = PyFloat_AsDouble(PyList_GetItem(returnList, i));
	}

	Py_DECREF(vertices);
	Py_DECREF(edges);
	Py_DECREF(e);
	Py_DECREF(pythonArgs);
	Py_DECREF(returnList);
}

