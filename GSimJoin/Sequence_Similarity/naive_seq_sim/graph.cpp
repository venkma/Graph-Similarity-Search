#include "common.h"
#include "graph.h"
#include "rollingHash.h"

using namespace std;

// reads the graph from input file 
void Graph:: readGraph(ifstream &inp)
{
	char tag;
	unsigned vid;
	// first line should be of the format "g vertexCount(unsigned int) edgeCount(unsigned int) gid(unsigned int)"
	inp >> tag; // the tag 'g'
	inp >> vertexCount; // the no. of vertices in the graph
	inp >> edgeCount; // the no. of edges in the graph
	inp >> gid; // the graph-id of the graph
	vertices.resize(vertexCount);

	for(int vtx_ind = 0; vtx_ind<vertexCount; vtx_ind++)
	{
		// each line for each vertex should be in the format like: "v vid(unsigned int)"
		inp >> tag >> vid; // the tag 'v' along with the vertex-id
		vertices[vtx_ind].vid = vid;
		vid_to_ind[vid] = vtx_ind; // mapping vertex-id to its index
	}

	int src_vtx, dest_vtx;

	for(int edg_ind = 0; edg_ind<edgeCount; edg_ind++)
	{
		// each line for each edge should be in the format like: "e vid_src(unsigned int) vid_dest(unsigned int)"
		inp >> tag >> src_vtx >> dest_vtx; // the tag 'e' along with the source and destination vertex-ids
		// Undirected graph : adding edge source to destination and destination to source
		vertices[vid_to_ind[src_vtx]].edges.push_back(&vertices[vid_to_ind[dest_vtx]]);
		vertices[vid_to_ind[dest_vtx]].edges.push_back(&vertices[vid_to_ind[src_vtx]]);
	}
}

// prints details of the graph
void Graph::displayGraph(){

	cout <<"g "<< gid << ":" << endl;
	cout <<"Vertex Count: "<< vertexCount << endl;
	cout <<"Edge Count: "<< edgeCount << endl;

	for(int vtx_ind = 0; vtx_ind<vertexCount; vtx_ind++)
	{
		cout << "v" << vtx_ind << ": " << vertices[vtx_ind].vid << endl;
		int edgeCount = vertices[vtx_ind].edges.size();
		for(int edg_ind = 0; edg_ind<edgeCount; edg_ind++)
		{
			cout << "e" << edg_ind << ": " << vertices[vtx_ind].vid << ", " << vertices[vtx_ind].edges[edg_ind]->vid << endl;
		}
	}
}

// Walk Algorithm : generates a 'walk' of each graph
void Graph::walkAlgorithm()
{
	int ind = 0;
	int walkInd = 0;
	bool neighbour_found;
	unsigned curr_vtx = vertices[0].vid; // current-vertex
	walk.resize(vertices.size()); // walk of a graph
	unordered_set<unsigned> visited; // set of visited vertices

	while(ind < vertexCount)
	{
		visited.insert(curr_vtx); // making current vertex visited
		walk[walkInd++] = curr_vtx; // adding current vertex to walk
		// checking for an unvisited neighbour of current vertex
		neighbour_found = false;
		for(int e=0; e < vertices[vid_to_ind[curr_vtx]].edges.size(); e++)
		{
			// if neighbouring vertex is not visited
			if(visited.find(vertices[vid_to_ind[curr_vtx]].edges[e]->vid)==visited.end())
			{ 
				curr_vtx = vertices[vid_to_ind[curr_vtx]].edges[e]->vid; // update the current vertex to neighbouring vertex
				neighbour_found = true; 
				break;	
			}
		}
		// if current vertex doesn't have any neighbouring unvisited vertex
		if(!neighbour_found)
		{
			// find next unvisited vertex
			while(ind < vertexCount && visited.find(vertices[ind].vid)!=visited.end())
				ind++;

			if(ind < vertexCount)
				curr_vtx = vertices[ind].vid; // update the current vertex
		}
	}
}

// computes shingles from the walk and stores their hash values
void Graph::computeShingles()
{
	// no. of shingles that can be generated from the walk of a graph
	int no_of_shingles = walk.size() - SHINGLESIZE + 1;

	if(no_of_shingles <= 0)
		return;
	
	// Using Rolling Hash function we compute each shingle's Hash value.
	static RollingHash rollingHash;

	shingles.resize(no_of_shingles);
	int shingle_hash = rollingHash.computeHash(walk, 0);
	shingles[0] = shingle_hash;

	for(int i=0; i < no_of_shingles-1; i++)
	{
		shingle_hash = rollingHash.computeShiftHash(walk[i], shingle_hash, walk[i+SHINGLESIZE]);
		shingles[i+1] = shingle_hash;
	}
}

// vertex comparator
bool vertexComp(Vertex &v1, Vertex &v2)
{
	if(v1.quality == v2.quality)
		return v1.vid < v2.vid;
	return v1.quality > v2.quality;
}

// edge comparator
bool edgeComp(Vertex* &e1, Vertex* &e2) 
{
	if(e1->quality == e2->quality)
		return e1->vid < e2->vid;
	return e1->quality > e2->quality;
}

// sorts the graph's vertices and and edge-list of each vertex
void Graph::sortGraph()
{
	// First sorted the vertices based on the vertex quality
	sort(vertices.begin(),vertices.end(),vertexComp);
	for(unsigned ind=0; ind<vertexCount; ind++)
	{
		vid_to_ind[vertices[ind].vid] = ind;
	}
	// Then sort the edge list of each vertex based on the destination vertex's quality
	for(unsigned i=0; i<vertexCount; i++)
	{
		sort(vertices[i].edges.begin(),vertices[i].edges.end(),edgeComp);
	}

}
