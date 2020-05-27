#pragma once
#include "common.h"

class MinHash
{
	private:
		static const int K = 96, MOD = 1000000009, MAXSHINGLE = 1000000007;
		vector<pair<int,int>> hashFunc; // Storing a,b in (ax + b) % MOD where x is an input
	public:
		MinHash();	// Intialize K random hash functions 
		void computeMinHash(vector<int> &shingles, vector<int> &minhash);
};