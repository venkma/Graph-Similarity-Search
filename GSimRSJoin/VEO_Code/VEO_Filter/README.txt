Program Flow:

	1. Parse the input file for Graph Datasets R and S using parseGraphDataset().
		-In parseGraphDataset() we take ratio of |R|:|S| in its simplest form
			for ex. 2500:7500 becomes 1:3, 
			so we pick 1 graph for R and 3 for S and so on until it poppulates both the datasets.

	2. Sorts each graph's vertex and edge-set in the datasets R and S.

	3. Sorts the graph datasets R and S wrt the graph (vertex_count+edge_count).

	4. Apply Loose-size and Common-vertex filter over graph dataset.

	5. If index filter is being applied, index graph dataset to form rank-list 

	6. For each graph pair apply:
		i) loose size filter
		ii) strict size filter
		iii) index filter
		iv) mismatching filter

	7. For each of the filtered graph pair:
		- Compute the similarity: computeSimilarity() function as in veo.cpp
			similarity-score = 100*{2*[commonV/(gR.vertexCount+gS.vertexCount+gR.edgeCount+gS.edgeCount)]}

	8. Record and store final statistics like total similarity time taken, total memory used and no of graphs filtered out after each filter in stat_file.txt.

	9. Record frequency distribution of similarity score of graph pairs in freq_distr_file.txt.

	10. Record graph ids of the pair along with the similarity score if it is greater than threshold in all_graph_file.txt.

Compiling this code:
$ g++ -std=c++14 filter_p.cpp veo.cpp graph.cpp -o filter

Running the code:
./filter inp_file 1 simScore_threshold dataset-size-R dataset-size-S res-dir
./filter inp_file 2 simScore_threshold mismatch dataset-size-R dataset-size-S res-dir
./filter inp_file 3 simScore_threshold mismatch noofbuckets dataset-size-R dataset-size-S res-dir
./filter inp_file 4 simScore_threshold mismatch noofbuckets dataset-size-R dataset-size-S res-dir

For Example:
./filter ./datasets/sorted_dataset_5_40.txt 4 60 0 2 2500 7500 stat

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

	2. choice: valid inputs : 1 for only loose size filter, 2 for loose size and common vertex filter, 3 for loose size, common vertex and banding technique filter.

	3. simScore_threshold: only those graph pairs with similarity score greater than this threshold will be stored in a file

	4. mismatch: 0 or 1 for true/false, if we want to apply mismatching filter or not

	5. noofbuckets: no. of buckets used while applying indexing filter

	6. dataset-size-R: size of dataset R

	7. dataset-size-S: size of dataset S

	8. result-directory: directory in which all the output files will be stored

Output Files:

	The program creates a directory named 'res-dir' in which it has 2 files:

		1. all_graph_pair.txt which has all graph pairs with similarity greater than simScore_threshold as follows:
			
			graph_id1 graph_id2 similarity_score

		2. stat_final.txt which will have all that stats as given below:

			GSimRSJoin: Sequence Similarity(filters)
			Dataset size of R: 2500
			Dataset size of S: 7500
			Shingle size: 3
			Similarity Score Threshold: 90
			Filter choice: 3
			No. of Bands: 4
			No. of Rows: 25
			Loose Size Filter count: 18750000
			Common Vertex Filter count: 2289
			Banding Technique filter candidates pair count: 2248
			Similar Graphs: 2248
			Memory used: 11.0977 MB
			Total Time Taken: 1002 milliseconds

		3. freq_distr_file.txt

			102 lines of pair of nos as follows
			<sim-score> <sim-pair-count>
		where,
			the first line indicates no of graph pairs with similarity score 0
			the last line indicates no of isomorphic graph pairs 
			and the rest of the line indicates no of graph pairs with similarity score sim-score to sim-score+1
