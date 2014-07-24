//Rewrite the rand & srand in C for consistency

void _srand(unsigned int);
unsigned int _rand(void);

#define RAND_MAX 0x8000

#define __rand rand
#define __srand srand

#define rand _rand
#define srand _srand