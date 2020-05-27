Program Flow:

	1. Parse the input file for Graph Dataset using parseGraphDataset().

	2. Sorts each graph's vertex and edge-set in the dataset.

	3. Sorts the graph dataset wrt the graph (vertex_count+edge_count).

	4. Apply Loose size and Common Vertex filter over graph dataset.

	5. Sort all graphs' vertex-set and edge-list of each vertex based on quality of vertices 

	6. If index filter is being applied, index graph dataset to form rank-list 

	7. For each graph pair apply:
		i) loose size filter
		ii) strict size filter
		iii) index filter
		iv) mismatching filter

	8. For each of the filtered graph pair:
		- Compute the similarity: computeSimilarity() function as in veo.cpp
			similarity-score = 100*{2*[commonV/(g1.vertexCount+g2.vertexCount+g1.edgeCount+g2.edgeCount)]}

	9. Record and store final statistics like total similarity time taken, total memory used and no of graphs filtered out after each filter in stat_file.txt.

	10. Record frequency distribution of similarity score of graph pairs in freq_distr_file.txt.

	11. Record graph ids of the pair along with the similarity score if it is greater than threshold in all_graph_file.txt.

Compiling this code:
$ g++ -std=c++14 filter_p.cpp veo.cpp graph.cpp -o filter

Running the code:
./filter inp_file 1 simScore_threshold dataset-size res-dir
./filter inp_file 2 simScore_threshold mismatch dataset-size res-dir
./filter inp_file 3 simScore_threshold mismatch noofbuckets dataset-size res-dir
./filter inp_file 4 simScore_threshold mismatch noofbuckets dataset-size res-dir

For Example:
./filter ./datasets/sorted_dataset_5_40.txt 4 60 0 2 1000 stat

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

	8. dataset-size: size of dataset to be selected

	9. result-directory: directory in which all the output files will be stored

Output Files:

	The program creates a directory named 'res-dir' in which it has 2 files:

		1. all_graph_pair.txt which has all graph pairs with similarity greater than simScore_threshold as follows:
			
			graph_id1 graph_id2 similarity_score

		2. stat_final.txt which will have all that stats as given below:

			GSimJoin: VEO Similarity(filters)
			Choice: 4
			Similarity Score Threshold: 60
			Dataset Size: 1000
			Mismatch: 0
			No of Buckets: 0
			Loose Filter Count: 487918
			Strict Filter Count: 487918
			Dynamic Filter Count: 488
			Final Similar Pair Count: 190
			Memory used: 4.41016 MB
			Total Time Taken: 91 milliseconds

		3. freq_distr_file.txt

			102 lines of pair of nos as follows
			<sim-score> <sim-pair-count>
		where,
			the first line indicates no of graph pairs with similarity score 0
			the last line indicates no of isomorphic graph pairs 
			and the rest of the line indicates no of graph pairs with similarity score sim-score to sim-score+1
