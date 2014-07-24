//////////////////////////////////////////////////////////////////////////
// Online codes encoder header                                          //
// Developed by Anran													//
//////////////////////////////////////////////////////////////////////////

#ifndef ONLINE_CODES_ENCODER_H
#define ONLINE_CODES_ENCODER_H

#include <functional>
#include "structures.h"

template<typename T>
class erasure_online_codes_encoder
{
private:
	int length;//original data length
	double e, d;//two parameters for online codes
	int k;//parameter for composite messages
	std::function<T*(T&, T&, T*)> XOR; //XOR operator
	std::function<T*(int)> alloc; //new operator
	std::function<void(T*)> dealloc; //delete operator
	int F;//auxiliary degree
	double* distribution;//degree probability distribution

	T* auxiliary;//auxiliary blocks
	int auxiliary_length;//auxiliary block count
	T* original;//original blocks

	//get the ith block in the composite message
	T* composite(int i);
	//generate the degree probability distribution
	void generate_distribution();
	//initialize (set zero) the auxiliary blocks
	void initialize_auxiliary();
	//generate the auxiliary blocks
	void generate_auxiliary(int);

	//buffer for random selection
	int* random_selection;
public:
	//initialize an encoder
	erasure_online_codes_encoder(double e, double d, int k, std::function<T*(T&, T&, T*)> XOR, std::function<T*(int)> alloc, std::function<void(T*)> dealloc);
	//set input original message, and a seed for generating auxiliary blocks
	void input(T* data,int length,int aux_seed);
	//get one block output. degree and dest are outputs.
	void output(int seed, int* degree, T* dest);

	~erasure_online_codes_encoder();
};

#endif