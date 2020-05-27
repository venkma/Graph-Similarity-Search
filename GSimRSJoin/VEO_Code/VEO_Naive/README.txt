Prgram Flow:

	1. Parse the input file for Graph Datasets R and S using parseGraphDataset().
		-In parseGraphDataset() we take ratio of |R|:|S| in its simplest form
			for ex. 2500:7500 becomes 1:3, 
			so we pick 1 graph for R and 3 for S and so on until it poppulates both the datasets.

	2. Sorts the graph datasets R and S:
	
	3. For each R-S graph pair among all |R|x|S| pairs:
		i) compute the similarity: computeSimilarity() function in veo.cpp
			simScore = 100*{2*[commonV/(gR.vertexCount+gS.vertexCount+gR.edgeCount+gS.edgeCount)]}

	4. Record statistics like similarity-score frequency, similarity-time for each graph.

	5. Store all graph pairs which have similarity score greater than given threshold in a file.

	6. Record and store final statistics like total similarity time taken and total memory used for the whole dataset.

	7. Record frequency distribution of similarity score of graph pairs in freq_distr_file.txt.


Compiling this code:
$ g++ naive.cpp veo.cpp graph.cpp -o naive

Running the code:
$ ./naive graph-dataset-file simScore_threshold dataset-size-R dataset-size-S res-dir

For Example:
$ ./naive ./datasets/sorted_dataset_5_40.txt 60 2500 7500 stat

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

	3. dataset-size-R: size of dataset R

	4. dataset-size-S: size of dataset S

	5. result-directory: directory in which all the output files will be stored

Output Files:

	The program creates a directory named 'res-dir' in which it has 2 files:

		1. all_graph_file.txt which has all graph pairs with similarity greater than simScore_threshold as follows:
			
			graph_id1 graph_id2 similarity_score

		2. stat_final.txt which will have all that stats as given below:
			
			GSimRSJoin: Sequence Similarity(naive)
			Dataset size of R: 2500
			Dataset size of S: 7500
			Shingle size: 3
			Similarity Score Threshold: 60
			Similar Graphs: 2248
			Memory used: 10.4961 MB
			Similarity Time: 55 milliseconds
			Total Time Taken: 1832 milliseconds
			
		3. graph_details: it stores all the statistics related to a particular graph(each file of the form: "g_graph-index_graph-id_sim.txt"):
			g_graph-index_graph-id_sim.txt contents:
				no. of graphs with simScore greater than the threshold
				graph-id graph-id2 simScore
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

