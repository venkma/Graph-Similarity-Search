#pragma once
#include "common.h"

class Graph
{
	public:
		unsigned gid, vertexCount, edgeCount; // graph-id, no. of vertices, no. of edges
		vector<unsigned> vertices; // Vertex-set
		vector<unsigned> degrees; // Degree-set
		unordered_map<unsigned,unsigned> vid_to_ind; // vid to index in adjacency list of graph
		vector<pair<unsigned, unsigned> > edges; // Edge-Set 

		Graph(){
			gid = 0;
			vertexCount = 0;
			edgeCount = 0;
		}

		void readGraph(istream &inp); // reads the graph from input file 
		void pushEdge(unsigned u, unsigned v); // adds an edge to the graph 
		void displayGraph(); // prints details of the graph

};

