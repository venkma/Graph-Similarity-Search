Prgram Flow:

	1. Parse the input file for Graph Datasets R and S using parseGraphDataset().
		-In parseGraphDataset() we take ratio of |R|:|S| in its simplest form
			for ex. 2500:7500 becomes 1:3, 
			so we pick 1 graph for R and 3 for S and so on until it poppulates both the datasets.

	2. Parse the PageRank file for loading the pageranks of datasets R and S using loadPageRanks() in similar fashion as parseGraphDataset().

	3. Preprocess the graph datasets R and S as follows:
		i) sort the vertex set with respect to quality and edge-list of each vertex: sortGraph() function in graph.cpp
		ii) generate random walk for each graph: walkAlgorithm() function in graph.cpp
		iii) compute shingle-set for each graph: computeShingles() function in graph.cpp which further uses rolling hash for generating hash values from rollingHash.cpp

	4. For each R-S graph pair among all |R|x|S| pairs:
		i) compute the similarity: computeSimilarity() function in seq_sim.cpp
			similarity-score = 100*(common_shingle_count)/(|shingleset_R|+|shingleset_S|-common_shingle_count)

	5. Record statistics like similarity-score frequency, similarity-time for each graph.

	6. Store all graph pairs which have similarity score greater than given threshold in a file.

	7. Record frequency distribution of similarity score of graph pairs in freq_distr_file.txt.

	8. Record and store final statistics like total similarity time taken and total memory used for the whole dataset.

Compiling this code:
$ g++ naive.cpp seq_sim.cpp graph.cpp rollingHash.cpp -o naive

Running the code:
$ ./naive graph-dataset-file pagerank-file SHINGLESIZE simScore_threshold dataset-size_R dataset-size_S res-dir

For Example:
$ ./naive ./datasets/sorted_dataset_5_40.txt ./datasets pageranks_sorted_5_40.txt 3 60 2500 7500 stat

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

	5. dataset-size-R: size of dataset of R

	6. dataset-size-S: size of dataset of S

	7. result-directory: directory in which all the output files will be stored

Output Files:

	The program creates a directory named 'res-dir' in which it has 2 files:

		1. all_graph_file.txt which has all graph pairs with similarity greater than simScore_threshold as follows:
			
			graph_id-R graph_id-S similarity_score

		2. stat_final.txt which will have all that stats as given below:

			GSimRSJoin: Sequence Similarity(naive)
			Dataset size of R: 2500
			Dataset size of S: 7500
			Shingle size: 3
			Similarity Score Threshold: 60
			Similar Graphs: 2248
			Memory used: 10.4922 MB
			Similarity Time: 50 milliseconds
			Total Time Taken: 1801 milliseconds

		3. graph_details: it stores all the statistics related to a particular graph of R(each file of the form: "g_graph-index_graph-id_sim.txt"):
			g_graph-index_graph-id_sim.txt contents:
				no. of graphs with simScore greater than the threshold
				graph-id graph-id-S simScore
				.
				.
				.
				followed by 102 lines of simScore frequency for that graph
				similarity time for that graph
	
		4. freq_distr_file.txt

			102 lines of pair of nos as follows
			<sim-score> <sim-pair-count>
			where,
				the first line indicates no of graph pairs with similarity score 0
				the last line indicates no of isomorphic graph pairs 
				and the rest of the line indicates no of graph pairs with similarity score sim-score to sim-score+1

