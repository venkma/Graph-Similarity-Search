#include "common.h"
using namespace std;

class Vertex{
	public:
		unsigned vid; // Vertex Label
		double quality; // Vertex Quality (PageRank)
		vector<Vertex*> edges; // List of Edges adjacent to the vertex along with its quality values

		Vertex()
		{
			vid = 0;
			quality = 0;
		}
};
class Graph{
	public:
		unsigned gid; // graph-id 
		unsigned vertexCount; // No. of Vertices
		unsigned edgeCount; // No. of Edges
		vector<Vertex> vertices;
		vector<pair<unsigned,unsigned> > edges; // Mapping between Vertex Label and its Index in adjacancy list 
		unordered_map<unsigned,unsigned> vid_to_ind; // Mapping between Vertex Label and its Index in adjacancy list 
		vector<unsigned> walk; // To store the random walk of the graph
		vector<int> shingles; // shingle-set's hash values
		vector<int> minhashes;
		
		Graph(){
			gid = 0;
			vertexCount = 0;
			edgeCount = 0;
		}
		void displayGraph(); // displays the graph's details
		void readGraph(ifstream &inp); // Parsing an input file to create the graph
		void walkAlgo(); // Finds walk using a traversal of the graph based on the quality
		void computeShingles(); // Computes shingles based on the walk of a graph
		void computeMinHashes();
		void sortGraph(); // Sort a graph vertices: based on quality of vertices and edges: based on quality of destn vts

};

