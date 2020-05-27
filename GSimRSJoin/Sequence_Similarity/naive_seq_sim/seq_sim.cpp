#include "seq_sim.h"

// computes Sequence Similarity of 2 input graphs
double computeSimilarity(Graph &g1, Graph &g2)
{
	double commonShingles = 0;
	
	// finds no. of common shingles between the 2 graphs
	for(int i=0,j=0; i < g1.shingles.size() && j < g2.shingles.size(); ){
		if(g1.shingles[i] < g2.shingles[j]){
			i++;
		}
		else if(g1.shingles[i] > g2.shingles[j]){
			j++;
		}
		else{
			commonShingles++;
			i++; j++;
		}

	}

	double simScore = 100.0*(commonShingles / (g1.shingles.size()+g2.shingles.size()-commonShingles));
	return simScore;
}

