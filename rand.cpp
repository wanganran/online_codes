#include "rand.h"

unsigned long m_seed = 1;

void _srand(unsigned int seed){
	m_seed = seed;
}
unsigned int _rand(void){
	return ((m_seed = m_seed * 214013L + 2531011L) >> 16) & 0x7fff;
}