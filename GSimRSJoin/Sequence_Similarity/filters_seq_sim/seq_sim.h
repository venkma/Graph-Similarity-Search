#pragma once
#include "graph.h"

double computeSimilarity(Graph &g1, Graph &g2);
int commonVertices(Graph &g1, Graph &g2);
void applyFilters(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, vector<bool> &candidate_graphs_R, vector<bool> &candidate_graphs_S, unordered_map<unsigned, unordered_set<unsigned> > &candidate_pairs, int dataset_size_R, int dataset_size_S, int choice, double simScore_threshold, string res_dir);
void preProcess(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, vector<bool> &candidate_graphs_R, vector<bool> &candidate_graphs_S);
void bandingTech(vector<Graph> &graph_dataset_R, vector<Graph> &graph_dataset_S, unordered_map<unsigned, unordered_set<unsigned> > &candidate_pairs, unordered_map<unsigned, unordered_set<unsigned> > &banding_pairs, int BANDS, int ROWS, string res_dir);
