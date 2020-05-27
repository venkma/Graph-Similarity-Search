Prgram Flow:

	1. Parse the input file for Graph Dataset and the Query Graph using parseGraphDataset().
		-In parseGraphDataset() we take take as input dataset size and the graph index 
			So, if datset-size > the index then it reads index_th graph to query graph while the rest of the graphs into the graph-dataset
			And if dataset-size > the index we first read the dataset till it has dataset-size graphs and then skip reading graphs until we read the index_th graph to the query-graph.

	2. Sorts the graphs in graph-dataset and the query-graph:
	
	3. For each graph g in the dataset:
		i) compute the similarity: computeSimilarity() function in veo.cpp
			simScore = 100*{2*[commonV/(graph-dataset[g].vertexCount+query-graph.vertexCount+graph-dataset[g].edgeCount+query-graph.edgeCount)]}

	4. Store all graphs which have similarity score greater than given threshold with the query-graph in a file.

	6. Record and store final statistics like total similarity time taken and total memory used for the whole dataset.

	7. Record frequency distribution of similarity score of graphs in freq_distr_file.txt.


Compiling this code:
$ g++ naive.cpp veo.cpp graph.cpp -o naive

Running the code:
$ ./naive graph-dataset-file simScore_threshold dataset-size query-index res-dir

For Example:
$ ./naive ./datasets/sorted_dataset_5_40.txt 60 7500 850 stat

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

	2. simScore_threshold: only those graph pairs with similarity score greater than this threshold will be stored in a file

	3. dataset-size: size of dataset 

	4. query-index: index of query graph

	5. result-directory: directory in which all the output files will be stored

Output Files:

	The program creates a directory named 'res-dir' in which it has 2 files:

		1. all_graph_file.txt which has all graph pairs with similarity greater than simScore_threshold as follows:
			
			graph_id1 graph_id2 similarity_score

		2. stat_final.txt which will have all that stats as given below:
			
			GSimSearch: VEO Similarity(naive)
			Dataset size: 7499
			Query Index: 850
			Query Graph id: 103296
			Similarity Score Threshold: 20
			Similarity pairs: 20
			Memory used: 6.34766 MB
			Total Time Taken: 4 milliseconds
				
		3. freq_distr_file.txt

			102 lines of pair of nos as follows
			<sim-score> <sim-pair-count>
			where,
				the first line indicates no of graph pairs with similarity score 0
				the last line indicates no of isomorphic graph pairs 
				and the rest of the line indicates no of graph pairs with similarity score sim-score to sim-score+1

