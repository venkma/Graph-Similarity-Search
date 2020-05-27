#pragma once

#include "common.h"

class Graph
{
	public:
		unsigned gid, vertexCount, edgeCount;
		vector<unsigned> vertices;
		vector<unsigned> degrees;
		unordered_map<unsigned,unsigned> vid_to_ind; // vid to index in adjacency list of graph
		vector<pair<unsigned, unsigned> > edges;

		Graph(){
			gid = 0;
			vertexCount = 0;
			edgeCount = 0;
		}

		void readGraph(istream &inp);
		void pushEdge(unsigned u, unsigned v);
		void displayGraph();
};

