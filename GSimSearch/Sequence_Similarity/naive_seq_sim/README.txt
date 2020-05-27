Prgram Flow:

	1. Parse the input file for Graph Dataset and the Query Graph using parseGraphDataset().
		-In parseGraphDataset() we take take as input dataset size and the graph index 
			So, if datset-size > the index then it reads index_th graph to query graph while the rest of the graphs into the graph-dataset
			And if dataset-size > the index we first read the dataset till it has dataset-size graphs and then skip reading graphs until we read the index_th graph to the query-graph.

	2. Parse the PageRank file for loading the pageranks in the same way as above using loadPageRanks().

	3. Preprocess the graph-dataset and the query-graph:
		i) sort the vertex set with respect to quality and edge-list of each vertex: sortGraph() function in graph.cpp
		ii) generate random walk for each graph: walkAlgorithm() function in graph.cpp
		iii) compute shingle-set for each graph: computeShingles() function in graph.cpp which further uses rolling hash for generating hash values from rollingHash.cpp

	4. For each graph in the dataset:
		i) compute the similarity: computeSimilarity() function in seq_sim.cpp
			similarity-score = 100*(common_shingle_count)/(|shingleset_i|+|shingleset_query-graph|-common_shingle_count)

	5. Record statistics like similarity-score frequency, similarity-time for each graph.

	6. Store all graphs which have similarity score with query-graph greater than given threshold in a file.

	7. Record and store final statistics like total similarity time taken, total memory used and similarity-score frequency for the whole dataset.

Compiling this code:
$ g++ naive.cpp seq_sim.cpp graph.cpp rollingHash.cpp -o naive

Running the code:
$ ./naive graph-dataset-file pagerank-file SHINGLESIZE simScore_threshold dataset-size query-index res-dir

For Example:
$ ./naive ./datasets/sorted_dataset_5_40.txt ./datasets pageranks_sorted_5_40.txt 3 60 1000 953 stat

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

	2. pagerank-file:
		Requirements:
			1. order of the pagerank file must be properly aligned with the each graph and each vertex
			2. file should be in the following format:

				g vertexCount edgeCount graph_id1
				v vertex_id1 vertex_quality1
				v vertex_id2 vertex_quality2
				.
				.
				.
				g vertexCount edgeCount graph_id2
				.
				.
				.
	3. SHINGLESIZE: size of each shingle while computing shingle-sets for each graph

	4. simScore_threshold: only those graph pairs with similarity score greater than this threshold will be stored in a file

	5. dataset-size: size of dataset to be selected

	6. query-index: index of query graph

	7. result-directory: directory in which all the output files will be stored

Output Files:

	The program creates a directory named 'res-dir' in which it has 2 files:

		1. all_graph_file.txt which has all graph pairs with similarity greater than simScore_threshold as follows:
			
			graph_id1 graph_id2 similarity_score

		2. stat_final.txt which will have all that stats as given below:
			
			GSimSearch: Sequence Similarity(naive)
			Dataset size: 7499
			Query Index: 956
			Query Graph id: 105551
			Shingle size: 3
			Similarity Score Threshold: 20
			Similarity pairs: 3
			Memory used: 8.6875 MB
			Total Time Taken: 62 milliseconds

		3. freq_distr_file.txt

			102 lines of pair of nos as follows
			<sim-score> <sim-pair-count>
			where,
				the first line indicates no of graph pairs with similarity score 0
				the last line indicates no of isomorphic graph pairs 
				and the rest of the line indicates no of graph pairs with similarity score sim-score to sim-score+1
