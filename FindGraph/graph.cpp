#include "common.h"
#include "graph.h"

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

