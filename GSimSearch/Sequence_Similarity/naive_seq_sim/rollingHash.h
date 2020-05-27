#pragma once
#include "common.h"

class RollingHash
{
	public:
		RollingHash();
		int computeHash(vector<unsigned> &walk, int i);	
		int computeShiftHash(unsigned del, unsigned hash, unsigned add);
	private:
		static const int BASE = 1009, MOD = 1000000007;
		static long long BASEPOWER; 
};

