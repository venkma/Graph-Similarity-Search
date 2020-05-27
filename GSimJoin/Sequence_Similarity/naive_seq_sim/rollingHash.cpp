#include "rollingHash.h"

long long RollingHash::BASEPOWER = 1; 

RollingHash::RollingHash(){
	for(int i=0; i < SHINGLESIZE-1; i++){
		BASEPOWER = (BASEPOWER*BASE) % MOD;
	}
}

int RollingHash::computeHash(vector<unsigned> &walk, int i)
{
	long long hashVal = 0;
	for(int j = i; j < SHINGLESIZE+i; j++)
	{
		hashVal = (hashVal * BASE + walk[j]) % MOD;
	}
	return (int)hashVal;
}

int RollingHash::computeShiftHash(unsigned del, unsigned hash, unsigned add)
{
	return (int) ( ( (hash + MOD - (del * BASEPOWER) % MOD ) * BASE + add) % MOD);
}


