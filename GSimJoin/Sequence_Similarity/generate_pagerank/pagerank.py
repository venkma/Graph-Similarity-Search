import networkx as nx

def computePR(*args):
	vertices,edges=args;

	graph = nx.DiGraph();
	
	for v in vertices:
		graph.add_node(v);

	for e in edges:
		graph.add_edge(e[0],e[1]);

	pagerank_dict = nx.pagerank(graph);
	pagerank_list = [];

	for v in vertices:
		pagerank_list.append(pagerank_dict[v]);

	return pagerank_list
	






