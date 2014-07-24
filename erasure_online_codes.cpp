//////////////////////////////////////////////////////////////////////////
// Implementation of Online codes										//
// Developed by Anran													//
//////////////////////////////////////////////////////////////////////////

#include <functional>
#include <assert.h>
#include <cmath>
#include <vector>
#include "rand.h"
#include "structures.h"
#include "erasure_online_codes_encoder.h"
#include "erasure_online_codes_decoder.h"
#include "log.h"

#define __DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define DEL(x) if(x!=NULL){delete(x);x=NULL;}

using namespace std;

//////////////////////////////////////////////////////////////////////////
//PUBLIC                                                                //
//////////////////////////////////////////////////////////////////////////

//generate the degree distribution and save to array distribution
void __generate_distribution(int length, double d, double e, int k, int& F, double*& distribution)
{
	assert(distribution == NULL);
	
	F = (int)((log(d) + log(e / 2)) / log(1 - d));
	if (F > length)F = length;
	distribution = new double[F + 1];
	distribution[1] = 1 - (1 + 1 / (double)F) / (1 + e);
	for (int i = 2; i < F + 1; i++)
	{
		distribution[i] = (1 - distribution[1]) / ((1 - 1 / (double)F) * i * (i - 1));
		distribution[i] += distribution[i - 1];
	}
}

//randomly select k number from 0 until n, save to dest. Note that the state of rand() will change.
void __rand_n_k(int n, int k, int* dest)
{
	memset(dest, 0, sizeof(int)*k);
	
	if (n <= k*k)
	{
		//O(n) implementation
		int t = 0;
		for (int i = 0; i < n && t < k; i++){
			if (rand()*(n - i) < (k - t)*RAND_MAX)
				dest[t++] = i;
		}
	}
	else
	{
		//O(k^2) implementation
		for (int j = 0; j < k; j++)
		{
			int x = rand() % n;
			for (int t = 0; t < j; t++)
				if (dest[t] == x){
				x = rand() % n;
				t = -1;
				}
			dest[j] = x;
		}
	}
}

//binary find the lower bound of to_find in arr
int __binary_find(double* arr, int len, double to_find)
{
	if (to_find < arr[0])return -1;
	if (to_find >= arr[len - 1])return len - 1;
	int s = 0, e = len - 1;
	while (e - s > 1)
	{
		int mid = (s + e) / 2;
		if (to_find < arr[mid])
			e = mid;
		else s = mid;
	}
	return s;
}

int __get_degree(double* distribution, int F)
{
	int x = rand() + 1;
	return __binary_find(distribution, F, (double)x / (0x8000)) + 1;
}

//////////////////////////////////////////////////////////////////////////
//ENCODER                                                               //
//////////////////////////////////////////////////////////////////////////

//return ith block in composite message
template<typename T>
T* erasure_online_codes_encoder<T>::composite(int i)
{
	assert(i < length + auxiliary_length);
	if (i < length)return original + i;
	else return auxiliary + i - length;
}

template<typename T>
void erasure_online_codes_encoder<T>::generate_distribution()
{
	__generate_distribution(length, d, e, k, F, distribution);
}

template<typename T>
void erasure_online_codes_encoder<T>::initialize_auxiliary()
{
	auxiliary_length = (int)(k*d*length);
	auxiliary = alloc(auxiliary_length);
	for (int i = 0; i < auxiliary_length; i++)
	{
		//set zero
		XOR(auxiliary[i], auxiliary[i], auxiliary + i);
	}
}

template<typename T>
void erasure_online_codes_encoder<T>::generate_auxiliary(int aux_seed)
{
	initialize_auxiliary();
	srand(aux_seed);
	int* check = new int[k];
	for (int i = 0; i < length; i++)
	{
		__rand_n_k(auxiliary_length, k, check);
		for (int j = 0; j < k; j++)
			XOR(original[i], auxiliary[check[j]], auxiliary + check[j]);
	}

	delete(check);
}

template<typename T>
erasure_online_codes_encoder<T>::erasure_online_codes_encoder
(double e, double d, int k, function<T*(T&, T&, T*)> XOR, function<T*(int)> alloc, function<void(T*)> dealloc)
{
	this->e = e;
	this->d = d;
	this->k = k;
	this->XOR = XOR;
	this->alloc = alloc;
	this->dealloc = dealloc;

	this->distribution = NULL;
	this->auxiliary = NULL;
	this->original = NULL;
	this->random_selection = NULL;
}

template<typename T>
erasure_online_codes_encoder<T>::~erasure_online_codes_encoder()
{
	DEL(this->distribution);
	dealloc(this->auxiliary);
	DEL(this->random_selection);
#ifdef __DEBUG
	logi("DELETE ENCODER");
#endif
}

template<typename T>
void erasure_online_codes_encoder<T>::input(T* data, int length, int aux_seed)
{
	this->length = length;
	original = data; 

	generate_distribution();
	generate_auxiliary(aux_seed);

	random_selection = new int[F];
}

template<typename T>
void erasure_online_codes_encoder<T>::output(int seed, int* degree, T* dest)
{
	assert(auxiliary != NULL);

	XOR(*dest, *dest, dest);
	srand(seed);
	*degree = __get_degree(distribution, F);
	__rand_n_k(length + auxiliary_length, *degree, random_selection);
	for (int i = 0; i < *degree; i++)
	{
		T* b = composite(random_selection[i]);
		XOR(*b, *dest, dest);
	}
}

//////////////////////////////////////////////////////////////////////////
//DECODER                                                               //
//////////////////////////////////////////////////////////////////////////

template<typename T>
erasure_online_codes_decoder<T>::erasure_online_codes_decoder
(double e, double d, int k, std::function<T*(T&, T&, T*)> XOR, std::function<T*(int)> alloc, std::function<void(T*)> dealloc)
{
	this->e = e;
	this->d = d;
	this->k = k;
	this->XOR = XOR;;
	this->alloc = alloc;
	this->dealloc = dealloc;

	this->distribution = NULL;
	this->composite_graph = NULL;
	this->composite_head = NULL;
	this->random_selection = NULL;
	this->output_data = NULL;
	this->aux_count = NULL;
}

template<typename T>
erasure_online_codes_decoder<T>::~erasure_online_codes_decoder()
{
	DEL(distribution);
	DEL(aux_count);
	DEL(random_selection);
	DEL(output_data);
;
	for (int i = 0; i < length + auxiliary_length; i++)
		DEL(composite_graph[i]);
	DEL(composite_graph);

	for (int i = 0; i < length + auxiliary_length; i++)
		DEL(composite_head[i]);
	DEL(composite_head);

	for (auto ptr = alloc_elements.begin(); ptr != alloc_elements.end(); ptr++)
		dealloc((*ptr));
	
	for (auto ptr = all_blocks.begin(); ptr != all_blocks.end(); ptr++)
	{
		//this should be done by application
		dealloc((*ptr)->data);

		if ((*ptr) != NULL){
			delete(*ptr);
			*ptr = NULL;
		}
	}
#ifdef __DEBUG
	logi("DELETE DECODER");
#endif
}

template<typename T>
void erasure_online_codes_decoder<T>::generate_distribution()
{
	__generate_distribution(length, d, e, k, F, distribution);
}

template<typename T>
void erasure_online_codes_decoder<T>::generate_composite_graph(int aux_seed)
{
	this->auxiliary_length = (int)(k*d*length);
	composite_graph = new int*[length + auxiliary_length];
	
	//set the original blocks' links
	for (int i = 0; i < length; i++)
		composite_graph[i] = new int[k];
	aux_count = new int[auxiliary_length];
	memset(aux_count, 0, sizeof(int)*auxiliary_length);

	//set the auxiliary blocks' links
	srand(aux_seed);
	for (int i = 0; i < length; i++)
	{
		__rand_n_k(auxiliary_length, k, composite_graph[i]);
		for (int j = 0; j < k; j++)
			aux_count[composite_graph[i][j]]++;
	}

	for (int j = 0; j < auxiliary_length; j++)
		composite_graph[length + j] = new int[aux_count[j]];

	int* back_aux_count = new int[auxiliary_length];
	memset(back_aux_count, 0, sizeof(int)*auxiliary_length);

	for (int i = 0; i < length; i++)
		for (int j = 0; j < k; j++)
			composite_graph[length + composite_graph[i][j]][back_aux_count[composite_graph[i][j]]++] = i;
	
	delete back_aux_count;
}

template<typename T>
void erasure_online_codes_decoder<T>::initialize_composite_head()
{
	this->composite_head = new vector<block<T>*>*[auxiliary_length + length];
	for (int i = 0; i < auxiliary_length + length; i++)this->composite_head[i] = new vector<block<T>*>();
}

template<typename T>
void erasure_online_codes_decoder<T>::start(int length, int aux_seed)
{
	this->length = length;
	generate_distribution();
	generate_composite_graph(aux_seed);
	initialize_composite_head();
	random_selection = new int[F];
	output_data = new T*[length + auxiliary_length];
	memset(output_data, 0, sizeof(T*)*(length + auxiliary_length));
}

template<typename T>
bool erasure_online_codes_decoder<T>::input(T* rec_block, int seed)
{
	if (is_finished())return false;
	srand(seed);
	int degree = __get_degree(distribution, F);
	__rand_n_k(length + auxiliary_length, degree, random_selection);
	block<T>* received = new block<T>();
	received->data = rec_block;
	received->node_xor = 0;
	all_blocks.push_back(received);

	for (int i = 0; i < degree; i++)
	{
		//if the related block is already recovered
		if (output_data[random_selection[i]] != NULL)
			XOR(*(received->data), *(output_data[random_selection[i]]), received->data);
		else{
			received->node_xor ^= random_selection[i];
			received->node_num++;
			composite_head[random_selection[i]]->push_back(received);
		}
	}

	//only one related block, recover it
	if (received->node_num == 1)
		belief_propagation(received);
	return true;
}

template<typename T>
void erasure_online_codes_decoder<T>::belief_propagation(block<T>* received){
	assert(received->node_num == 1);

	vector<block<T>*> to_bp;
	to_bp.push_back(received);
	
	//BFS for better performance
	int i=0;
	while(true){
		block<T>* curr = to_bp[i];
		int id = curr->node_xor;
		curr->node_num = 0;

		//the block to recover is already recovered. Ignore it.
		if (output_data[id] != NULL)goto cont;

		std::vector<block<T>*>* list = composite_head[id];
		
		for (auto ptr = list->begin(); ptr != list->end(); ptr++){
			if ((*ptr) == curr)continue;
			(*ptr)->node_num--;
			(*ptr)->node_xor ^= id;
			XOR(*((*ptr)->data), *(curr->data), (*ptr)->data);

			if ((*ptr)->node_num == 1)
				to_bp.push_back((*ptr));
		}
		//the recovered block will have no links
		list->clear();

		//submit the recovered block
		solve_LDPC(id, curr->data);

	cont:
		i++;
		if (i >= to_bp.size())break;
	}
}

template<typename T>
void erasure_online_codes_decoder<T>::LDPC_belief_propagation(int id)
{
	//find the uncovered block
	int residue = 0;
	int residue_c = 0;
	for (int i = 0; i < aux_count[id - length]; i++)
		if (output_data[composite_graph[id][i]] == NULL)
		{
			residue = composite_graph[id][i];
			residue_c++;
		}
	if (residue_c == 1)
	{
		auto nt = this->alloc(1);
		alloc_elements.push_back(nt);
		//clear
		XOR(*nt, *nt, nt);
		//copy
		XOR(*nt, *(output_data[id]), nt);
		//recover
		for (int i = 0; i < aux_count[id - length]; i++)
			if (composite_graph[id][i] != residue)
				XOR(*nt, *(output_data[composite_graph[id][i]]), nt);
		//submit
		solve_LDPC(residue, nt);
		//iterate
		for (int i = 0; i < k; i++)
			if (output_data[composite_graph[residue][i] + length] != NULL)LDPC_belief_propagation(composite_graph[residue][i] + length);
	}
}

template<typename T>
void erasure_online_codes_decoder<T>::solve_LDPC(int id, T* data)
{
	if (output_data[id] != NULL){
		return;
	}
	output_data[id] = data;
	if (id >= length)
		LDPC_belief_propagation(id);
	else{
		decoded_blocks_count++;
		for (int i = 0; i < k; i++)
			if (output_data[composite_graph[id][i] + length] != NULL)LDPC_belief_propagation(composite_graph[id][i] + length);
	}
}

template<typename T>
double erasure_online_codes_decoder<T>::get_progress()
{
	return (double)decoded_blocks_count / length;
}
 
template<typename T>
bool erasure_online_codes_decoder<T>::is_finished()
{
	return decoded_blocks_count == length;
}

template<typename T>
void erasure_online_codes_decoder<T>::copy_to(T* dest)
{
	assert(is_finished());
	for (int i = 0; i < length; i++)
		memcpy(dest + i, output_data[i], sizeof(T));
}

//generate type-specific classes
template class erasure_online_codes_encoder < int > ;
template class erasure_online_codes_decoder < int > ;