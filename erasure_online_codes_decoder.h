//////////////////////////////////////////////////////////////////////////
// Online codes encoder header                                          //
// Developed by Anran													//
//////////////////////////////////////////////////////////////////////////

#ifndef ONLINE_CODES_DECODER_H
#define ONLINE_CODES_DECODER_H

#include <functional>
#include <vector>
#include "structures.h"

template<typename T>
class erasure_online_codes_decoder
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
	int auxiliary_length;//auxiliary block count

	//generate the degree distribution
	void generate_distribution();
	//generate the bipartite graph for recovering the original message from the composite message
	void generate_composite_graph(int aux_seed);
	//initialize the bipartite graph for obtaining the blocks of the composite message
	void initialize_composite_head();
	//BP algorithm to solve the bipartite graph for obtaining the blocks of the composite message
	void belief_propagation(block<T>* received);
	//BP algorithm to solve the bipartite graph for recovering the original message
	void LDPC_belief_propagation(int id);
	//check in a left node, and do the BP algorithm
	void solve_LDPC(int id, T* data);

	//all blocks received. Used for memory extraction
	std::vector<block<T>*> all_blocks;
	//elements that is allocated by calling alloc function 
	std::vector<T*> alloc_elements;

	//the bipartite graph for obtaining the blocks of the composite message
	std::vector<block<T>*>** composite_head;
	//the bipartite graph for recovering the original message from the composite message
	int** composite_graph;

	//buffer for random selection
	int* random_selection;

	//the table for checking the blocks in the composite message
	T** output_data;
	//the affacted blocks number for each auxiliary blocks
	int* aux_count;
	//decoded blocks in the original message
	int decoded_blocks_count;
public:
	//initialize a decoder
	erasure_online_codes_decoder(double e, double d, int k, std::function<T*(T&, T&, T*)> XOR, std::function<T*(int)> alloc, std::function<void(T*)> dealloc);
	
	//start decode with a given seed for generating auxiliary blocks
	void start(int length, int aux_seed);
	//input a received block (with its seed)
	bool input(T* block, int seed);
	//check if the original message is recovered
	bool is_finished();
	//get the progress of the recovery
	double get_progress();
	//copy the result to a successive memory space
	void copy_to(T* dest);

	~erasure_online_codes_decoder();
};

#endif