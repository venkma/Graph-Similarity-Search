Prgram Flow:

	1. Parse the input file for Graph Dataset using parseGraphDataset().

	2. Parse the PageRank file for loading the pageranks using loadPageRanks().

	3. Apply Loose size and Common Vertex filter over graph dataset.

	4. Sort all graphs' vertex-set and edge-list of each vertex based on quality of vertices 

	5. Preprocess the graph dataset:
		i) sort the vertex set with respect to quality and edge-list of each vertex: sortGraph() function in graph.cpp
		ii) generate random walk for each graph: walkAlgorithm() function in graph.cpp
		iii) compute shingle-set for each graph: computeShingles() function in graph.cpp which further uses rolling hash for generating hash values from rollingHash.cpp

	6. Apply Filtering using Banding technique with the help of minhashes of shingles of each graph.
		- We do this by grouping graphs in a set of buckets repeatedly.
		- Add all possible pair in each bucket to the set of candidate pairs.
	7. Intersection between filtered pairs after loose size and common vertex filter and filtered pairs after banding technique will form the final candidate pairs

	8. For each of the final candidate graph pair:
		i) compute the similarity: computeSimilarity() function in seq_sim.cpp
			similarity-score = 100*(common_shingle_count)/(|shingleset1|+|shingleset2|-common_shingle_count)

	9. Record graph ids of the pair along with the similarity score if it is greater than threshold.

	10. Record and store final statistics like total similarity time taken, total memory used and no of graphs filtered out after each filter.

Compiling this code:
$ g++ naive.cpp seq_sim.cpp graph.cpp rollingHash.cpp -o naive

Running the code:
$ ./naive graph-dataset-file pagerank-file SHINGLESIZE simScore_threshold dataset-size res-dir

For Example:
$ ./naive ./datasets/sorted_dataset_5_40.txt ./datasets pageranks_sorted_5_40.txt 3 60 1000 stat

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

	4. choice: valid inputs : 1 for only loose size filter, 2 for loose size and common vertex filter, 3 for loose size, common vertex and banding technique filter.

	5. simScore_threshold: only those graph pairs with similarity score greater than this threshold will be stored in a file

	6. BANDS: Used in Banding Technique filter to divide the signature matrix to form different buckets

	7. ROWS: Used in Banding Technique filter to divide the signature matrix to form different buckets

	8. dataset-size: size of dataset to be selected

	9. result-directory: directory in which all the output files will be stored

Output Files:

	The program creates a directory named 'res-dir' in which it has 2 files:

		1. all_graph_file.txt which has all graph pairs with similarity greater than simScore_threshold as follows:
			
			graph_id1 graph_id2 similarity_score

		2. stat_final.txt which will have all that stats as given below:

			GSimSearch: Sequence Similarity(filters)
			Dataset size: 7499
			Shingle size: 3
			Similarity Score Threshold: 20
			Filter choice: 2
			Loose Size Filter count: 7498
			Common Vertex Filter count: 3
			Similar Graphs: 3
			Memory used: 8.33203 MB
			Total Time Taken: 1 milliseconds
		3. freq_distr_file.txt

			102 lines of pair of nos as follows
			<sim-score> <sim-pair-count>
			where,
				the first line indicates no of graph pairs with similarity score 0
				the last line indicates no of isomorphic graph pairs 
				and the rest of the line indicates no of graph pairs with similarity score sim-score to sim-score+1


