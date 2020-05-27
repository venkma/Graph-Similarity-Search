#include "minhash.h"

bool SETSEED = false;

MinHash::MinHash()
{
	if(!SETSEED)
	{
		srand(time(NULL));
		SETSEED = true;
	}
	int r;
	unordered_set<int> uset;
	hashFunc.resize(K);

	// Generating K random numbers(for 'a') from 1 to maxshingle 
	for(int i = 0; i < K;)
	{
		r = rand()%MAXSHINGLE + 1;
		if(uset.find(r)==uset.end())
		{
			uset.insert(r);
			hashFunc[i++].first = r;
		}
	}
	uset.clear();

	// Genrating K random numbers(for 'b') from 1 to maxshingle
	for(int i = 0; i < K;)
	{
		r = rand()%MAXSHINGLE + 1;
		if(uset.find(r)==uset.end())
		{
			uset.insert(r);
			hashFunc[i++].second = r;
		}
	}
}

void MinHash::computeMinHash(vector<int> &shingles, vector<int> &minhash)
{
	minhash.resize(K, INT_MAX);
	long long int a,b;
	// Going through all shingles
	for(int i=0; i < shingles.size(); i++)
	{
		// Going through all random hash functions
		for(int j=0; j < K; j++)
		{
			a = hashFunc[j].first;
			b = hashFunc[j].second;
			minhash[j] = (int)min((long long int)minhash[j], ((a*shingles[i]+b) % MOD));
		}
	}
}
