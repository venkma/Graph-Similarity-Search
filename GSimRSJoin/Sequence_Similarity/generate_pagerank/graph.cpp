#include "common.h"
#include "graph.h"

using namespace std;

void Graph:: readGraph(ifstream &inp)
{
	char tag;
	// first line should be of the format "g vertexCount(unsigned int) edgeCount(unsigned int) gid(unsigned int)"
	inp >> tag; // first tag should be 'g'
	inp >> vertexCount;
	inp >> edgeCount;
	inp >> gid;

	vertices.resize(vertexCount);
	unsigned vid;

	for(int vtx_ind = 0; vtx_ind<vertexCount; vtx_ind++)
	{
		// each line for each vertex should be in the format like: "v vid(unsigned int)"
		inp >> tag >> vid;
		vertices[vtx_ind].vid = vid;
		vid_to_ind[vid] = vtx_ind;
	}

	int src_vtx, dest_vtx;

	for(int edg_ind = 0; edg_ind<edgeCount; edg_ind++)
	{
		// each line for each edge should be in the format like: "e vid_src(unsigned int) vid_dest(unsigned int)"
		inp >> tag >> src_vtx >> dest_vtx;
		vertices[vid_to_ind[src_vtx]].edges.push_back(&vertices[vid_to_ind[dest_vtx]]);
		vertices[vid_to_ind[dest_vtx]].edges.push_back(&vertices[vid_to_ind[src_vtx]]);
	}
}


void Graph::displayGraph()
{
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

bool vertexComp(Vertex &v1, Vertex &v2)
{
	if(v1.quality == v2.quality)
		return v1.vid < v2.vid;
	return v1.quality > v2.quality;
}

bool edgeComp(Vertex* &e1, Vertex* &e2) 
{
	if(e1->quality == e2->quality)
		return e1->vid < e2->vid;
	return e1->quality < e2->quality;
}

void Graph::sortGraph()
{
	// First sorted the vertices based on the vertex quality
	sort(vertices.begin(),vertices.end(),vertexComp);
	for(unsigned ind=0; ind<vertexCount; ind++)
	{
		vid_to_ind[vertices[ind].vid] = ind;
	}
	// Then sort the edge list of each vertex based on destn vertex's quality
	for(unsigned i=0; i<vertexCount; i++)
	{
		sort(vertices[i].edges.begin(),vertices[i].edges.end(),edgeComp);
	}

}
