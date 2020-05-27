Program usage: programe used to find index and other information of a graph given its graph id

Compiling this code:
$ g++ findgraph.cpp graph.cpp -o findgraph

Running the code:
$ ./findgraph graph-dataset-file dataset-size graph-id

For Example:
$ ./findgraph ./datasets/sorted_dataset_5_40.txt 1000 105551

Input Arguements:

	1. graph-dataset-file:
		Requirements:
			1. dataset must be already sorted based on no. of vertices
			2. file should be in the following format:
			
				g vertexCount edgeCount graph_id1
				v vertex_id1
				v vertex_id2
				.
				.
				.
				e source_vertex_id1 destination_vertex_id1
				e source_vertex_id2 destination_vertex_id2
				.
				.
				.
				g vertexCount edgeCount graph_id2
				.
				.
				.

	2. dataset-size: size of dataset to be selected

	3. graph-id: id of graph to be searched

