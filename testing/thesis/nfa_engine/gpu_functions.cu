// Original work:
// Copyright (C) 2010  
// Pierluigi Rolando (pierluigi.rolando@polito.it)
// Netgroup - DAUIN - Politecnico di Torino
//
// Niccolo' Cascarano (niccolo.cascarano@polito.it)
// Netgroup - DAUIN - Politecnico di Torino
//
// Modified work:
// Copyright (C) 2017  
// Vinh Dang (vqd8a@virginia.edu)
// University of Virginia
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifdef DEVICE_EMU_DBG
#include <stdio.h>
#endif
#include <assert.h>
#include "common.h"
#include "gpu_functions.h"

#define myId		    threadIdx.x
#define thread_count    blockDim.x
#define nstreams        gridDim.x
#define BITS 33

extern __constant__ unsigned int alphabet_size;

extern __shared__ ST_BLOCK shared_base[];
__global__ void nfa_kernel(	st_t *nfa_tables,
							st_t *src_tables,
							unsigned int *input_transition_tables,
							symbol_fetch *input,
							unsigned long *cur_size_vec,
							ST_BLOCK *_svs,// ST_BLOCK is just a fancy name for a chunk of 32-bit unsigned data
							unsigned int *st_vec_lengths,
							ST_BLOCK *persistents,
							unsigned int *match_count, match_type *match_array, unsigned int match_vec_size,
							unsigned int *accum_nfa_table_lengths, unsigned int *accum_offset_table_lengths, unsigned int *accum_state_vector_lengths,
							st_t *filter_table, 
							unsigned int *symbol_counts, unsigned int *symbol_offset, unsigned short *helper_table, /* unsigned long long int * trans_per_thread , unsigned long long int *active_bits,
							unsigned long long int *num_active_groups */
#ifdef COUNTERS
							, unsigned long long int *table_access_count
#endif							
							){
	
	__shared__ unsigned int shr_match_count;//Note: initializing is not allowed for shared variable
	shr_match_count = 0;
	
	
	//unsigned int num = alphabet_size;
	__shared__ unsigned int s_symbol_offset[257];
	__shared__ unsigned int s_symbol_counts[257];
	for (int i = myId; i < 257; i += thread_count) {
		s_symbol_offset[i] = symbol_offset[i];
		s_symbol_counts[i] = symbol_counts[i];
	}
	
	unsigned int tmp_match_count;
	match_type tmp_match;
	
	// cur_size is the input string length
	size_t cur_size = (blockIdx.x == 0 ? cur_size_vec[blockIdx.x] : cur_size_vec[blockIdx.x] - cur_size_vec[blockIdx.x - 1]);
	//int num_symbols = 0;
	// skip to the right input string
	if(blockIdx.x > 0)
		input += (cur_size_vec[blockIdx.x - 1]/fetch_bytes); 

	// get the right final_vector for reading the initial state and
	// storing the final output
	unsigned int st_vec_length             = st_vec_lengths            [blockIdx.y];
	unsigned int accum_state_vector_length = accum_state_vector_lengths[blockIdx.y]; 
	unsigned int accum_nfa_table_length    = accum_nfa_table_lengths   [blockIdx.y];
	unsigned int accum_offset_table_length = accum_offset_table_lengths[blockIdx.y];
	
	ST_BLOCK *final_vector = _svs + st_vec_length*blockIdx.x + accum_state_vector_length*nstreams;

	// shared_base points to the beginning of shared memory
	ST_BLOCK *status_vector = &shared_base[0];	
	ST_BLOCK *future_status_vector = shared_base + st_vec_length;

	//Copy the initial status vector from global to shared to set the input state
	for(unsigned int j = myId; j < st_vec_length; j += thread_count)
		//final_vector here is a misnomer as it is set with the initial state bit enabled
		status_vector[j] = final_vector[j];
	__syncthreads();

	unsigned int limit = cur_size; 
	//printf("cur_size %d\n",cur_size);
	
	// // Number of transitions processed by thread
	// unsigned int thread_transitions = 0;

	//Payload loop
	for(unsigned int p=0; p<limit; p+=fetch_bytes, input++){	
		symbol_fetch Input_ = *input;//fetch 4 bytes from the input string
		for (unsigned int byt = 0; byt < fetch_bytes; byt++) {
			unsigned int Input = Input_ & 0xFF;//extract 1 byte
			Input_  = Input_ >> 8;//Input_ right-shifted by 8 bits
			// thread_transitions++;
			// Filter metadata in global memory
			// unsigned int filter_base = symbol_offset[Input   + accum_offset_table_length];
			// unsigned int filter_number = symbol_counts[Input + accum_offset_table_length];

			// Filter metadata in shared memory
			unsigned int filter_base = s_symbol_offset[Input]; // start of symbol's states in filter
			unsigned int filter_number = s_symbol_counts[Input]; // number of unique states for this symbol
			
			// Reset the future status vector
			// Persistent (self-loop'd) states are never reset once reached.
#pragma unroll 2
			for(unsigned int w = myId; w < st_vec_length; w += thread_count) {
				future_status_vector[w] = 0;//might work too, since persistents vector is not used
			}
				
			__syncthreads();
			// int groups_active;
			int num = 1;
			int thread_id = threadIdx.x;
			for(unsigned int i=thread_id/num; i<filter_number; i+=thread_count/num) {
				// Each thread reads 1 transition at each step.
#ifdef GROUPS
	#ifdef SELECT 
				st_t src_group = __ldca(&filter_table[i + filter_base]);
	#else
				st_t src_group = filter_table[i + filter_base];
	#endif
				ST_BLOCK lo_block = status_vector[src_group];
				
				// // Code to determine number of active states in a group and increment count
				// ST_BLOCK bits = lo_block;
				
				// // find num of active bits
				// int num_bits = 0;
				
				// while (bits) {
				// 	num_bits += bits & 1;
				// 	bits >>= 1;
				// }
				
				// atomicAdd(&active_bits[num_bits], 1);
				
				// Code to determine number of groups active at a time
				// // loop through status_vector and count number of groups active
				// groups_active = 0;
				// for(int j = 0; j < st_vec_length; j++) {
				// 	ST_BLOCK group = status_vector[j];
				// 	groups_active += !!group;					
				// }
				// //if thread.id = 0, add to array of size num_groups
				// if (i == 0) {
				// 	num_active_groups[groups_active]++;
				// }
				
				if(lo_block) {
					//unsigned short real_number = helper_table[i + filter_base + 1] - real_base_offset;
					// input_transition_table contains the cumulative number of transitions for each input symbol
					unsigned int tr_base   = input_transition_tables[Input+ accum_offset_table_length];
					unsigned int tr_number = input_transition_tables[Input+1+ accum_offset_table_length] - tr_base;
	#ifdef TRIMMED
		#ifdef SELECT
					unsigned short real_base_offset = __ldca(&helper_table[i + filter_base]);
					unsigned short next_offset = __ldca(&helper_table[i + filter_base + 1]);
		#else
					unsigned short real_base_offset = helper_table[i + filter_base];
					unsigned short next_offset = helper_table[i + filter_base + 1];
		#endif					
					if (next_offset < real_base_offset) {
						next_offset = tr_number;
					}
					// // Decrement because of redundant adding
					// thread_transitions--;
					unsigned short jk;
					for (jk = real_base_offset + (thread_id%num); jk < next_offset; jk += num) {
						// thread_transitions++;
						//atomicAdd(table_access_count, 1);
						st_t src_state = src_tables[jk + tr_base+ accum_nfa_table_length];
						st_t dst_state = nfa_tables[jk + tr_base+ accum_nfa_table_length];
	#else // not TRIMMED
						st_t src_state = src_tables[i + tr_base+ accum_nfa_table_length];
						st_t dst_state = nfa_tables[i + tr_base+ accum_nfa_table_length];	
	#endif

#ifdef COUNTERS
						atomicAdd(table_access_count, 1);
#endif

// These macros are there to extract the relevant fields.
// Bits and chunks are there to select the right bit in the state vectors.
#define src_bit  (1 << (src_state % bit_sizeof(ST_BLOCK)))
#define src_chunk (src_state / bit_sizeof(ST_BLOCK))

						lo_block = src_bit & status_vector[src_chunk];
						if (lo_block) {
							if (dst_state < 0) {//Added for matching operation: check if the dst state is an accepting state
								dst_state = -dst_state;
								tmp_match_count = atomicAdd(&shr_match_count, 1);//printf("Inside kernel-low, offset: %d, state: %d, count %d\n",p, dst_state, shr_match_count);
								//match_offset[match_vec_size*blockIdx.x + shr_match_count-1 + blockIdx.y*match_vec_size*nstreams] = p + byt;
								//match_states[match_vec_size*blockIdx.x + shr_match_count-1 + blockIdx.y*match_vec_size*nstreams] = dst_state;
								tmp_match.off = p + byt;
								tmp_match.stat= dst_state;
								match_array[tmp_match_count + match_vec_size*(blockIdx.x + blockIdx.y*nstreams)] = tmp_match;
							}
#define dst_bit  (1 << (dst_state % bit_sizeof(ST_BLOCK)))
#define dst_chunk (dst_state / bit_sizeof(ST_BLOCK))
							atomicOr(&future_status_vector[dst_chunk], dst_bit);    //unsigned int atomicOr(unsigned int* address, unsigned int val);
						}
#ifdef TRIMMED
					}
#endif
				}
			}
#else // not GROUPS
	#ifdef SELECT
				st_t src_state = __ldca(&filter_table[i + filter_base]); 
	#else
				st_t src_state = filter_table[i + filter_base];
	#endif
				   
				
// These macros are there to extract the relevant fields.
// Bits and chunks are there to select the right bit in the state vectors.
#define src_bit  (1 << (src_state % bit_sizeof(ST_BLOCK)))
#define src_chunk (src_state / bit_sizeof(ST_BLOCK))

#ifdef EXTRAWORK
				ST_BLOCK lo_block = 1;
#else
				ST_BLOCK lo_block = src_bit & status_vector[src_chunk];
#endif
				if(lo_block) {

					//unsigned short real_number = helper_table[i + filter_base + 1] - real_base_offset;
					// input_transition_table contains the cumulative number of transitions for each input symbol
					unsigned int tr_base   = input_transition_tables[Input+ accum_offset_table_length];
					unsigned int tr_number = input_transition_tables[Input+1+ accum_offset_table_length] - tr_base;
	#ifdef TRIMMED
		#ifdef SELECT
						// Selectively load into cache
						unsigned short real_base_offset = __ldca(&helper_table[i + filter_base]);
						unsigned short next_offset = __ldca(&helper_table[i + filter_base + 1]);
		#else
						unsigned short real_base_offset = helper_table[i + filter_base];
						unsigned short next_offset = helper_table[i + filter_base + 1];
		#endif
					// Edge case
					if (next_offset < real_base_offset) {
						next_offset = tr_number;
					}

					// For each transitions starting with src_state
					unsigned short jk;
					for (jk = real_base_offset + (thread_id%num); jk < next_offset; jk += num) {
						st_t dst_state = nfa_tables[jk + tr_base+ accum_nfa_table_length];
		#ifdef EXTRAWORK
						// Unnecessarily access src_table
						src_state = src_tables[jk + tr_base+ accum_nfa_table_length];
		#endif
	#else // not TRIMMED
					// Only need to handle one transition!
					st_t dst_state = nfa_tables[i + tr_base+ accum_nfa_table_length];
	#endif
					

						
						
#ifdef COUNTERS
						atomicAdd(table_access_count, 1);
#endif

#define dst_bit  (1 << (dst_state % bit_sizeof(ST_BLOCK)))
#define dst_chunk (dst_state / bit_sizeof(ST_BLOCK))

#ifdef EXTRAWORK
						lo_block = src_bit & status_vector[src_chunk];
						if (lo_block) {
#endif
							if (dst_state < 0) {//Added for matching operation: check if the dst state is an accepting state
								
								dst_state = -dst_state;
								tmp_match_count = atomicAdd(&shr_match_count, 1);//printf("Inside kernel-low, offset: %d, state: %d, count %d\n",p, dst_state, shr_match_count);
								//match_offset[match_vec_size*blockIdx.x + shr_match_count-1 + blockIdx.y*match_vec_size*nstreams] = p + byt;
								//match_states[match_vec_size*blockIdx.x + shr_match_count-1 + blockIdx.y*match_vec_size*nstreams] = dst_state;
								tmp_match.off = p + byt;
								tmp_match.stat= dst_state;
								match_array[tmp_match_count + match_vec_size*(blockIdx.x + blockIdx.y*nstreams)] = tmp_match;
							}
							atomicOr(&future_status_vector[dst_chunk], dst_bit);    //unsigned int atomicOr(unsigned int* address, unsigned int val);
#ifdef EXTRAWORK
						}
#endif				
	#ifdef TRIMMED
					}	
	#endif
				}
			}
#endif
			// Swap status_vector and future_status_vector
			if(status_vector == shared_base){
				status_vector = shared_base + st_vec_length;
				future_status_vector = shared_base;
			} else {
				status_vector = shared_base;
				future_status_vector = shared_base + st_vec_length;
			}

			// // Increment array at index of number of transitions for this thread and reset
			// trans_per_thread[thread_transitions]++;
			// thread_transitions = 0;
			
			__syncthreads();

		
		}
	}
	
	
	//Copy the result vector from shared to device memory
#pragma unroll
	for(unsigned int j = myId; j < st_vec_length; j += thread_count) {
		final_vector[j] = status_vector[j];
	}	
	match_count[blockIdx.x + blockIdx.y*gridDim.x] = shr_match_count;
	
	__syncthreads();
}
