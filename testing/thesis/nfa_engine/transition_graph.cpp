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

#include <boost/algorithm/string/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/iterator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/multi_array.hpp>
#include <boost/range.hpp>
#include <boost/regex.hpp>
#include <cstdlib>
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <pthread.h>//parallelize transition duplicating section
#include <time.h>
#include <sys/time.h>

#include "config_options.h"
#include "cuda_allocator.h"
#include "globals.h"
#include "half_trie.h"
#include "transition_graph.h"

#include <algorithm>//for "find" function

using namespace std;
using namespace half_trie;

extern ConfigOptions cfg;

int print_en_transition_graph=0;//debug purpose: 0-don't print; 1-print
/*------------------------------------------------------------------------------------*/
struct MyComparator {
    const vector<unsigned short> & value_vector;

    MyComparator(const vector<unsigned short> & val_vec): value_vector(val_vec) {}

    bool operator()(int i1, int i2)
    {
        return value_vector[i1] < value_vector[i2];
    }
};
/*------------------------------------------------------------------------------------*/
void set_bit_(unsigned int which, ST_BLOCK *vector) {
	unsigned int chunk = which/bit_sizeof(ST_BLOCK);
	unsigned int off   = which%bit_sizeof(ST_BLOCK);
	vector[chunk] |= 1 << off;
}
/*------------------------------------------------------------------------------------*/
struct thread_data{
		std::vector< std::vector<unsigned int> > dst_state_groups;
		std::vector< std::vector<st_t> > src_table_vec;
		std::vector< std::vector<st_t> > nfa_table_vec;
		int symstart;
        int Symspercpu;
     	st_t *nfa_table;
		st_t *src_table;
		unsigned int *offset_table;
        int trans_cnt;
		int threadID;
};
/*------------------------------------------------------------------------------------*/
void *SearchAndDup (void *p) {
	struct thread_data *my_data;  
	vector<st_t> src_state_ids;
	vector<st_t> dst_state_ids;
	
	my_data = (struct thread_data *) p;
	
	unsigned int trans_cnt_=0;
	for (unsigned short j = 0; j < my_data->Symspercpu; j++) {
		unsigned base = my_data->offset_table[j   + my_data->symstart];
		unsigned end  = my_data->offset_table[j+1 + my_data->symstart];
		cout << "Thread " << my_data->threadID <<", Duplicating: symbol "<< j << endl;
		for (unsigned short prev_j = 0; prev_j < cfg.get_alphabet_size(); prev_j++) {
			unsigned int counters_=0;
			for (unsigned jj = base; jj < end; jj++){
				if (my_data->src_table[jj] != 0xFFFF) {
					std::vector<unsigned int>::iterator it;
					it = std::find(my_data->dst_state_groups[prev_j].begin(), my_data->dst_state_groups[prev_j].end(), my_data->src_table[jj]);
					if ( it != my_data->dst_state_groups[prev_j].end() ){
						src_state_ids.push_back(my_data->src_table[jj]);
						dst_state_ids.push_back(my_data->nfa_table[jj]);
						counters_++;
						trans_cnt_++;
					}
				}
			}
			if(counters_ % 2) {
				src_state_ids.push_back(0xFFFF);//???necessary???
				dst_state_ids.push_back(0xFFFF);//???necessary???
				trans_cnt_++;
			}
			my_data->src_table_vec.push_back(src_state_ids);
			my_data->nfa_table_vec.push_back(dst_state_ids);
			src_state_ids.clear();
			dst_state_ids.clear();
		}
	}
	my_data->trans_cnt = trans_cnt_;
	pthread_exit(NULL);
	return NULL;
}
/*------------------------------------------------------------------------------------*/
TransitionGraph::TransitionGraph(istream &file, CudaAllocator &allocator, unsigned int gid, unsigned int *trans_per_sym) ://Changed
	all_(allocator), persistent_states_(cfg.get_state_count(gid), all_), initial_states_(cfg.get_state_count(gid), all_), accept_states_(cfg.get_state_count(gid), all_)
{
	using boost::algorithm::split_regex;
	using boost::regex;
	using boost::lexical_cast;

	transition_count_ = 0;

	Graph tnsn_graph;
	string line;
    
	unsigned int persistent_count = 0;
	
	while(getline(file, line)) {
		list<string> parts;

		// Skip comments
		if (line[0] == '#')
			continue;

		split_regex(parts, line, regex("[[:blank:]]*:[[:blank:]]*"));

		// Skip empty lines
		if(parts.size() == 0)
			continue;

		vector<string> srcdst;
		split_regex(srcdst, parts.front(), regex("[[:blank:]]*->[[:blank:]]*"));

		parts.pop_front();

		// Skip empty atoms
		if(parts.size() == 0)
			continue;

		assert(srcdst.size() >= 1);
		assert(parts.front().size());

		unsigned src = lexical_cast<unsigned>(srcdst.front());

		if(srcdst.size() == 2) {
			/* Handle a transition */
			unsigned dst = lexical_cast<unsigned>(srcdst.back());
			
			vector<string> atoms;
			split_regex(atoms, parts.front(), regex("[[:blank:]]+(?!\\|)[[:blank:]]*"));
            
			//cout << "      MY TEST: src= " << src << " dst= " << dst << ", atoms.size()= " << atoms.size() << endl;
			//cout << "      ";
			//for(int i = 0; i < atoms.size(); ++i) cout << atoms[i] << " ";
			//cout << endl;
	
			BOOST_FOREACH(string s, atoms) {
				if(s.size() > 0) {
					vector<string> ssrcend;
					split_regex(ssrcend, s, regex("\\|"));
					assert(ssrcend.size() <= 2);

					unsigned srange, erange;
					srange = erange = lexical_cast<unsigned>(ssrcend.front());
					if(ssrcend.size() > 1) {
						erange = lexical_cast<unsigned>(ssrcend.back());
					}
					assert(erange < cfg.get_alphabet_size()); //cout << "      srange= "<< srange << ", erange= " << erange << ", cfg.get_alphabet_size()= " << cfg.get_alphabet_size() << endl;
					
					if ((src==dst)&&(srange==0)&&(erange==255)) {
						//persistent_states_.set_bit(src);//test: states with a self-transition on every character of the alphabet
						persistent_count ++;
					}
					//else {
						for(unsigned i = srange; i <= erange; ++i)
							add(tnsn_graph, src, dst, Ranges(Range(srange, erange)));
					//}
				}
			}
		} else if(parts.front().find("accepting") != string::npos) {
			/* Handle an accepting state and its related rules */
			vector<string> atoms;
			split_regex(atoms, parts.front(), regex("[[:blank:]]+"));

			BOOST_FOREACH(string s, atoms) {
				if(s.size() > 0 && isdigit(s[0])) {
					accepting_states_[src].insert(lexical_cast<unsigned>(s)); //insert a new value of rule to the set of this state
					//cout << "ACCE_states_ " << src << ", Rule: " << s << endl;
				}
			}
			//accepting_states_[23].insert(100);//TEST
			
            //printf("persistent_states_[%d]\n",src);//Check content of "persistent_states_" vector?? It seems to me that this vector is the same as "accepting_states_" vector
			
			//persistent_states_.set_bit(src);//Commented
			
			//Commenting the line aboved seems working fine. Matchings are kept track by other data structures: match_count, match_offset, match_states
			//The purpose of commenting is for the simplicity of coding in the optimization part: searching for compatible groups; 
			//and for having the same output as vasim: in some NFAs, accepting states are not the last states, but they transition to other states, so keeping these states as persistent states is not a good idea 
			
			accept_states_.set_bit(src);
		} else if(parts.front().find("initial") != string::npos) {
			/* Enable the initial state in all the required state vector */
			initial_states_.set_bit(src);
		}  else {
			cerr << "[warning] cannot parse line '" << line << "'\n";
		}
	}

	//clog << "Read " << transition_count_ << " transitions.\n";
	//clog << "Have " << persistent_count << " persistent states.\n";

	/* Allocate the iNFAnt NFA data structure in host memory and fill it */
#if DEBUG
	cout << "Alphabet size: " << cfg.get_alphabet_size() << endl;
#endif
	
	offset_table_size_ = (cfg.get_alphabet_size()+1)*sizeof(*offset_table_);
	alphabet_size_ = cfg.get_alphabet_size();
	// padding is not required as in the original iNFAnt 
	nfa_table_size_ = transition_count_*sizeof(*nfa_table_); 
	nfa_table_      = all_.alloc_host<st_t>(nfa_table_size_);
	src_table_      = all_.alloc_host<st_t>(nfa_table_size_);
	offset_table_   = all_.alloc_host<unsigned int>(offset_table_size_);
	filter_symbol_offset = all_.alloc_host<unsigned int>(offset_table_size_);
	filter_symbol_counts = all_.alloc_host<unsigned int>(offset_table_size_);
	
	//cout << sizeof(*nfa_table_) << "," << sizeof(st_t) << endl;

	//cout<< "sizeof(*offset_table_) = " << sizeof(*offset_table_) << " and sizeof(*nfa_table_) = " << sizeof(*nfa_table_) << endl;
	//cout << " sizeof(*nfa_table_) = " << sizeof(*nfa_table_) << ", nfa_table_ size (bytes)= " << nfa_table_size_ << endl;
	
	hash_map<symbol_t, map<Graph::state_t, set<Graph::state_t> > > hm;
	tnsn_graph.project(hm);

	typedef boost::multi_array<unsigned, 2> array_t;
	typedef array_t::index index_t;
	array_t tnsn_per_char(
			boost::extents[cfg.get_alphabet_size()][cfg.get_state_count(gid)]);
	for(index_t i = 0; i < cfg.get_alphabet_size(); ++i)
		for(index_t j = 0; j < cfg.get_state_count(gid); ++j)
			tnsn_per_char[i][j] = 0;

	unsigned nfa_current = 0;
	for (symbol_t s = 0; s < cfg.get_alphabet_size(); ++s) {
		offset_table_[s] = nfa_current;
		
		hash_map<symbol_t, map<Graph::state_t, set<Graph::state_t> > >::iterator ss;
		if((ss = hm.find(s)) == hm.end())
			continue;

		map<Graph::state_t, set<Graph::state_t> >::iterator ii;
		for(ii = ss->second.begin(); ii != ss->second.end(); ++ii) {
			set<Graph::state_t>::iterator jj;
			for(jj = ii->second.begin(); jj != ii->second.end(); ++jj) {
				tnsn_per_char[ss->first][ii->first]++;
				nfa_table_[nfa_current] = *jj;
				src_table_[nfa_current++] = ii->first;
			}
		}
	}
	offset_table_[cfg.get_alphabet_size()] = nfa_current;

	//iNFAnt2: Represent accepting states in the NFA state table as negative numbers 
	for (unsigned int i = 0; i < transition_count_; i++) {
		unsigned int state_bit   =  1 << (((unsigned int)nfa_table_[i]) % bit_sizeof(ST_BLOCK));
		unsigned int state_chunk =        ((unsigned int)nfa_table_[i]) / bit_sizeof(ST_BLOCK);
		ST_BLOCK *tmp_st_vector = accept_states_.get_host(false);
		ST_BLOCK match_check = state_bit & tmp_st_vector[state_chunk];
		if(match_check)
			nfa_table_[i] = -nfa_table_[i];	
	}
	//
	//cout << "ORIGINAL iNFAnt CODE" << endl;
	//Keep track of the number of transitions of each symbol
	unsigned int cnt_=0;
	for (unsigned short j = 0; j < cfg.get_alphabet_size(); j++) {
		trans_per_sym[j]=offset_table_[j+1] - offset_table_[j];
		cnt_ += offset_table_[j+1] - offset_table_[j];
	}
	cout << "Total transitions (including paddings): " << cnt_<< endl;
	// ------------ Modifications begin here ------------

	// Count number of states that have at least one transition back to a start state.
	/* 
	int last_start_state = 0;
	for (int i = 0; i < cnt_; i++) {
		if ((src_table_[i] == 0) && (nfa_table_[i] != 0)) {
			if (nfa_table_[i] > last_start_state)
				last_start_state = nfa_table_[i];
			//start_states.push_back(transitions[i].second);
		}
	}
	map<int,bool> back_to_start;

	for (int i = 0; i < cnt_; i++) {
		if (nfa_table_[i] < last_start_state) {
			back_to_start[src_table_[i]] = true;
		}
	}
	
	cout << "total states " << cfg.get_state_count(gid) << endl;
	cout << "states w/ transition back to start " << back_to_start.size() << endl;	
	*/

	// Count number of transitions of each symbol per group
	vector<vector<int>> group_trans(cfg.get_alphabet_size());
	int num_groups = cfg.get_state_count(gid)/32;

	for (int i = 0; i < cfg.get_alphabet_size(); i++) {
		int st = offset_table_[i];
		int en = offset_table_[i + 1];
	
		group_trans[i] = vector<int>(num_groups + 1, 0);
		for (int j = st; j < en; j++) {
			int src_group = src_table_[j] / 32;
			group_trans[i][src_group]++;
		}
		
	}

	
	vector<int> num_empty_per_sym(cfg.get_alphabet_size(), 0);
	vector<int> max_group_per_sym(cfg.get_alphabet_size(), 0);
	map<pair<int, int>, int> group_max_map;

	//cout << "Transitions per group per symbol (min, median, max, empty):" << endl;
	for (int i = 0; i < cfg.get_alphabet_size(); i++) {
		int max, min, min_index;
		int max_group = 0;
		//int num_empty = 0;
		int num_sorted = 0;
		// sort transitions in ascending order
		while (num_sorted < num_groups) {
			min = -1;
			max = 0;
			min_index = num_sorted;
			for (int j = num_sorted; j < num_groups; j++) {
				if (group_trans[i][j] < min || min == -1) {
					min = group_trans[i][j];
					min_index = j;
				}
				if (num_sorted == 0) {
					if (group_trans[i][j] == 0) {
					 	num_empty_per_sym[i]++;
					 }
					if (group_trans[i][j] > max) {
						max = group_trans[i][j];
						max_group_per_sym[i] = j;
					}
				}
			}
			// swap min and current
			group_trans[i][min_index] = group_trans[i][num_sorted];
			group_trans[i][num_sorted] = min;
			num_sorted++;
		}


		// Calculate min/median/max transitions for each symbol
		min = group_trans[i][0];
		max = group_trans[i][num_groups - 1];
		int median;
		if (num_groups % 2 == 0) {
			median = group_trans[i][num_groups / 2];
		}
		else {
			median = (group_trans[i][num_groups / 2] + group_trans[i][(num_groups / 2) + 1]) / 2;
		}

		//Print info for each symbol
		cout << i << ": " << min << ", " << median << ", " 
			 << max << ", " << num_empty_per_sym[i] << endl;
		// cout << max << ": " << max_group_per_sym[i] << endl;
		
		//Print comparison between original trans/sym to trans/sym using max transitions/group
		//cout << i << ", " << trans_per_sym[i] << ", " << max*num_groups << endl;
	
	 	// Construct map with max transitions and corresponding group
	 	pair<int, int> max_pair = make_pair(max, max_group_per_sym[i]);
	 	pair<map<pair<int, int>, int>::iterator, bool> ret;
	 	ret = group_max_map.insert(make_pair(max_pair, 1));
	 	if (ret.second == false) {
	 		//get existing element and increment count
	 		group_max_map.at(max_pair)++;
	 	}		
	}


	// // Print map of max groups
	// cout << endl << "Max group count (max, corresponding group, count)" << endl;
	// for (auto& s : group_max_map) {
	// 	cout << s.first.first << ", " << s.first.second << ", " << s.second << endl;
	// }

	// // Caculate frequencies for number of transitions per symbol per group
	// vector<map<int, int>> trans_freq_per_sym;
	// for (int i = 0; i < cfg.get_alphabet_size(); i++) {
	// 	map<int, int> trans_freq;
	// 	for (int j = 0; j < num_groups; j++) { 	
	// 		// insert num transitions into map
	// 		pair<map<int, int>::iterator, bool> ret;
	// 		ret = trans_freq.insert(make_pair(group_trans[i][j], 1));
	// 		if (ret.second == false) {
	// 			// get existing element and increment count
	// 			trans_freq.at(group_trans[i][j])++;
	// 		}
	// 	}
	// 	trans_freq_per_sym.push_back(trans_freq);
	// }

	// // Print map of number of frequency of transitions per symbol per group
	// cout << endl << "Frequency of number of transitions/group for each symbol" << endl;
	// int symbol = 0;
	// for (auto& map : trans_freq_per_sym) {
	// 	cout << endl << "Symbol " << symbol << ":" << endl;
	// 	for (auto& trans: map) {
	// 		cout << trans.first << " transitions " << trans.second << " time(s)." << endl;
	// 	}
	// 	symbol++;
	// }

	// level order/BFS traversal
	set<pair<st_t, st_t>> transition_set;
	set<st_t> state_set;
	state_set.insert(0);
	for (int i = 0; i < cnt_; ++i) {
		pair<st_t, st_t> trn;
		trn.first = src_table_[i];
		trn.second = nfa_table_[i];
		transition_set.insert(trn);
		state_set.insert(trn.second);
	}
	int level = 0;
	int group_counter = 0;
	st_t state_id;
	set<pair<st_t, st_t>> used_set;
	set<st_t> next_level, group_level, seen_group, doubled_group;
	unordered_map<st_t, int> estimated_hot;
	estimated_hot.insert(make_pair(0, level)); // start state
	while (estimated_hot.size() < state_set.size() && !transition_set.empty()) {
		//cout << "Level " << level << ": ";
		set<pair<st_t, st_t>>::iterator itr;
		pair<st_t, st_t> current_tr;
		for (itr = transition_set.begin(); itr != transition_set.end(); itr++) { 
			current_tr = *itr;
			if (estimated_hot.count(current_tr.first) && estimated_hot.size() < state_set.size()) {
				next_level.insert(current_tr.second);
				used_set.insert(current_tr);
				
				st_t state_id = current_tr.second;
				if (current_tr.second < 0) {
					state_id = -state_id;
				}	
				group_level.insert(state_id/32);
			}
    	} 

		set<st_t>::iterator st_itr;
		st_t current_st;
		for (st_itr = next_level.begin(); st_itr != next_level.end(); st_itr++) {
			current_st = *st_itr;
			estimated_hot.insert(make_pair(current_st, level));
		}
		for (st_itr = group_level.begin(); st_itr != group_level.end(); st_itr++) {
			current_st = *st_itr;
			if (seen_group.count(current_st)) {
				//cout << "!";
				group_counter++;
				doubled_group.insert(current_st);
			}
			//cout << current_st << ",";

			seen_group.insert(current_st);
		}
		//cout << endl;
		//cout << "Level Count: " << next_level.size() << endl;
		if (next_level.size() == 0) {
			break;
		}
		group_level.clear();
		next_level.clear();
		++level;

		for (itr = used_set.begin(); itr != used_set.end(); itr++) {
			transition_set.erase(*itr);
		}
		used_set.clear(); // already removed
	}
	//cout << "Total levels: " << level << endl;
	//cout << estimated_hot.size() << endl;
	//cout << doubled_group.size() << "/" << seen_group.size() << endl;


	// Build Filter
	int start, end;
	//int symbols_done = 1;
	filter_symbol_offset[0] = 0;
	st_t src_st;
	for (unsigned int i = 0; i < cfg.get_alphabet_size(); ++i) {

		start = offset_table_[i];
		end = offset_table_[i+1];

		bool match_found = false;

		// Check to see if current symbol i matches collections with prior symbols j
		for (int j = 0; (j < i) && (j < cfg.get_alphabet_size()) && !match_found; ++j) {
			int filter_index = 0;
			unsigned int uniq_count = 0;
			st_t prev = -1, temp = -1;
			for (int k = start; k <= end; ++k) {
				
				// found a matching subset of filter!
				if (k == end) {
					filter_symbol_offset[i] = filter_symbol_offset[j];
					filter_symbol_counts[i] = uniq_count;
					match_found = true;
				} else {
					
					// Gonna go out of bounds
					if (filter_symbol_offset[j] + filter_index >= state_filter.size()) {
						break;
					}

					temp = src_table_[k];

					// Not a match
					if (temp != src_table_[k-start+offset_table_[j]]) {
						break;
					}
#ifdef GROUPS
					temp = temp/32;
#endif
#ifdef TRIMMED
					// remove adjacent repitition
					if (temp != prev) {
#endif
						prev = temp;
						uniq_count++;

						// do we have a matching element?
						if (temp != state_filter.at(filter_symbol_offset[j] + filter_index)) {
							// nope
							break;
						}
						
						++filter_index;
#ifdef TRIMMED
					}
#endif
				}
			}
		}

		// Went through whole filter without finding a matching collection
		if (!match_found) {
			
			filter_symbol_offset[i] = state_filter.size();
			st_t temp, prev = -1;
			unsigned int src_count = 0;
			for (int l = start; l < end; ++l) {
				if (src_table_[l] < 0) {
					//cout << "negative: " << src_table_[l] << endl;
				}
#ifdef GROUPS
				temp = src_table_[l]/32;
#else
				temp = src_table_[l];
#endif
						

#ifdef TRIMMED
				if (temp != prev) {
					helper_table.push_back(l - start);
					prev = temp;
					state_filter.push_back(temp);
					src_count++;
				}
#else
				helper_table.push_back(l - start);
				prev = temp;
				state_filter.push_back(temp);
				src_count++;
#endif
			}
			filter_symbol_counts[i] = src_count;
			
		}

		//cout << i << ": " << filter_symbol_offset[i] << ", " << filter_symbol_counts[i] << endl;
	}

	// Get number of filter elements per symbol.
	unsigned int total_count = 0;
	int symbol_count_real = 0;
	unsigned int max_count = 0;
	for (unsigned int j = 0; j < cfg.get_alphabet_size(); ++j) {
		// in case there is an empty "buffer" symbol
		if (filter_symbol_counts[j] > 0) {
			total_count += filter_symbol_counts[j];
			symbol_count_real++;
		}

		if (filter_symbol_counts[j] > max_count) {
			max_count = filter_symbol_counts[j];
		}
	}

	// // Determine if symbols with same max share source states
	// map<pair<int, int>, int> max_source_states;
	// for (int i = 0; i < cfg.get_alphabet_size(); i++) {
	// 	int curr_max = group_trans[i][num_groups - 1];
	// 	int source_states = filter_symbol_offset[i];
	// 	pair<int, int> max_states = make_pair(curr_max, source_states);
		
	// 	pair<map<pair<int, int>, int>::iterator, bool> ret;
	// 	ret = max_source_states.insert(make_pair(max_states, 1));
	// 	if (ret.second == false) {
	// 		//get existing element and increment count
	// 		max_source_states.at(max_states)++;
	// 	}	
	// }

	// // cout << endl <<  "Source states for symbols w/ same max (max, filter offset, count)" << endl;
	// // for (auto& s : max_source_states) {
	// // 	cout << s.first.first << ", " << s.first.second << ", " << s.second << endl;
	// // }
	// //cout << "Avg. Symbol Filter Element Count: " << total_count/symbol_count_real << endl;
	// //cout << "Max Symbol Filter Element Count: " << max_count << endl;

	// Place filter and metadata into appropiately sized arrays
	state_filter.shrink_to_fit();
	helper_table.push_back(end);
	filter_size_ = state_filter.size()*sizeof(st_t);
	filter_table_ = all_.alloc_host<st_t>(filter_size_);
	helper_table_ = all_.alloc_host<unsigned short>(helper_table.size() * sizeof(unsigned short));
	total_count = 0;
	for (int i = 0; i < state_filter.size(); i++) {
		filter_table_[i] = state_filter.at(i);
		helper_table_[i] = helper_table.at(i);
	}
	helper_table_[helper_table.size()-1] = helper_table.back();
	

	// Collect information about transitions per filter element
	unsigned int curr_count = 0;
	unsigned int min_count = -1;
	max_count = 0;
	unordered_map<int, int> transitions_counts;
	//cout << "" << endl;
	int bin1 = 0, bin2 = 0, bin3 = 0;
	for (int i = 0; i < state_filter.size(); i++) {
		//cout << helper_table_[i+1]- helper_table_[i] << endl;
		
		// This provides a good estimate, leaves out a few edge cases
		if (helper_table_[i+1] >  helper_table_[i] && i + 1 < state_filter.size()) {
			curr_count = helper_table_[i+1]- helper_table_[i];
			total_count += curr_count;
			if (curr_count > max_count) {
				max_count = curr_count;
				//cout << max_count << endl;
			}
			if (curr_count < min_count) {
				min_count = curr_count;
			}
			if (curr_count == 1) {
				bin1++;
			} else if (curr_count < 100) {
				bin2++;
				//transitions_counts.insert_or_assign(make_pair(10, bin2++));
			} else {
				bin3++;
				//transitions_counts.insert_or_assign(make_pair(11, bin3++));
			}
		}
	}
	
	// cout << "Average edges per src state/group: " << (total_count+0.0)/state_filter.size() << endl;
	// cout << "Min edges per src state/group: " << min_count << endl;
	// cout << "Max edges per src state/group: " << max_count << endl;
	// cout << "Filter count: " << state_filter.size() << ", " << helper_table.size() << endl;
	// cout << "Bins: " << bin1 << ", " << bin2 << ", " << bin3 <<endl;
	//clog << "NFA loading done.\n";
	
	return;
}
/*------------------------------------------------------------------------------------*/
void TransitionGraph::copy_to_device(){
	cudaError_t retval;
			
	d_nfa_table_ = all_.alloc_device<ST_BLOCK>(nfa_table_size_);
	retval = cudaMemcpy(d_nfa_table_, nfa_table_, nfa_table_size_,
			cudaMemcpyHostToDevice);
	CUDA_CHECK(retval, "Error while copying NFA table to device memory");

	d_src_table_ = all_.alloc_device<ST_BLOCK>(nfa_table_size_);
	retval = cudaMemcpy(d_src_table_, src_table_, nfa_table_size_,
			cudaMemcpyHostToDevice);
	CUDA_CHECK(retval, "Error while copying NFA src table to device memory");

	d_offset_table_ = all_.alloc_device<unsigned int>(offset_table_size_);
	retval = cudaMemcpy(d_offset_table_, offset_table_,
			offset_table_size_, cudaMemcpyHostToDevice);
	CUDA_CHECK(retval,
			"Error while copying character start offset table to device memory");
#ifdef GROUPS1
	d_filter_table_ = all_.alloc_device<unsigned short>(state_filter.size()*sizeof(unsigned short));
	retval = cudaMemcpy(d_filter_table_, filter_table_, state_filter.size()*sizeof(unsigned short),
			cudaMemcpyHostToDevice);
#else
	d_filter_table_ = all_.alloc_device<ST_BLOCK>(state_filter.size()*sizeof(st_t));
	retval = cudaMemcpy(d_filter_table_, filter_table_, state_filter.size()*sizeof(st_t),
			cudaMemcpyHostToDevice);
#endif
	CUDA_CHECK(retval, "Error while copying filter table to device memory");

#ifdef DEBUG
	cout << "Automata:" << endl;
	for (unsigned short i = 0; i < cfg.get_alphabet_size(); i++) {
		
		unsigned base = offset_table_[i];
		unsigned end = offset_table_[i+1];
		//cout << "Symbol " << i << ", base " << base << ", end " << end << endl;
		printf("Symbol: %d, base: %d, end: %d, number of transition (end-base): %d\n", i, base, end, end-base);
		/*for (unsigned j = base; j < end; ++j) {
			//cout << src_table_[j] << " -> " << nfa_table_[j] << endl;
			printf("%d -> %d\n",src_table_[j],nfa_table_[j]);
		}*/
	}

#endif
#ifdef DEBUG
	cout << "Automata by night" << endl;
	for (unsigned i = 0; i < nfa_table_size_/2; i++) {
		cout << src_table_[i] << " -> " << nfa_table_[i] << endl;
	}
#endif

	all_.dealloc_host(nfa_table_);
	all_.dealloc_host(src_table_);
	all_.dealloc_host(offset_table_);
	
	return;
}
/*------------------------------------------------------------------------------------*/
namespace boost {
	template<> struct range_const_iterator<Ranges> {
		typedef Ranges::const_iterator type;
	};
}
/*------------------------------------------------------------------------------------*/
void TransitionGraph::add(Graph &tnsn_graph, unsigned src, unsigned dst,
		const Ranges r)
{
	for (Ranges::const_iterator ii = r.begin(); ii != r.end(); ++ii) {
		if(!tnsn_graph.get(src, dst).includes(*ii))
			++transition_count_;
	}

	tnsn_graph.add(src, dst, r);
	return;
}
/*------------------------------------------------------------------------------------*/
void TransitionGraph::accepting_rules(const StateVector &final_vector, std::set<unsigned int> &results) const {
	
	vector<unsigned>accepted_rules;
	//cout << "size of final_vector " << final_vector.get_size() * bit_sizeof(unsigned char) << endl;
	for (unsigned i = 0; i < final_vector.get_size() * bit_sizeof(unsigned char); i++) {
		ST_BLOCK mask = 1;
		mask <<= i % bit_sizeof(ST_BLOCK);
		if (final_vector.get_host()[i/bit_sizeof(ST_BLOCK)] & mask) {
			map<unsigned, set<unsigned> >::const_iterator it = accepting_states_.find(i);
			if (it != accepting_states_.end()) {
				set<unsigned>::iterator iitt;
				for (iitt = it->second.begin();	iitt != it->second.end(); ++iitt)
					results.insert(*iitt);
			}
		}
	}
#ifdef DEBUG	
	//TEST ONLY
	for (unsigned i = 0; i < final_vector.get_size() * bit_sizeof(unsigned char); i++) {
		map<unsigned, set<unsigned> >::const_iterator it = accepting_states_.find(i);
		if (it != accepting_states_.end()) {
			set<unsigned>::iterator iitt;
			cout << "State: " << i << ", Rules: ";
			for (iitt = it->second.begin();	iitt != it->second.end(); ++iitt) cout << *iitt << " ";
			cout << endl;
		}		
	}	
	//END TEST
#endif
}
/*------------------------------------------------------------------------------------*/
//Added for matching operation
/*void TransitionGraph::mapping_states2rules(unsigned int *match_count, unsigned int *match_offset, unsigned int *match_states, 
                                           unsigned int match_vec_size, std::vector<unsigned long> cur_size_vec, std::ofstream &fp, int *rulestartvec, unsigned int gid) const {
	unsigned int total_matches=0;	
	for (int j = 0; j < cur_size_vec.size(); j++)	total_matches += match_count[j];
	fp   << "REPORTS: Total matches: " << total_matches << endl;
	
	for (int j = 0; j < cur_size_vec.size(); j++) {
		for (unsigned i = 0; i < match_count[j]; i++) {
			map<unsigned, set<unsigned> >::const_iterator it = accepting_states_.find(match_states[match_vec_size*j + i]);		
			if (j==0) fp   << match_offset[match_vec_size*j + i] << "::" << endl;
			else      fp   << match_offset[match_vec_size*j + i] + cur_size_vec[j-1] << "::" << endl;
			if (it != accepting_states_.end()) {
				set<unsigned>::iterator iitt;
				for (iitt = it->second.begin();	iitt != it->second.end(); ++iitt) {
					fp   << "    Rule: " << *iitt + rulestartvec[gid] << endl;
				}
			}
		}
	}
}*/
void TransitionGraph::mapping_states2rules(unsigned int *match_count, match_type *match_array, unsigned int match_vec_size, std::vector<unsigned long> cur_size_vec, std::vector<unsigned long> pad_size_vec, std::ofstream &fp
#ifdef DEBUG
                                                                         , int *rulestartvec, unsigned int gid
#endif
                                                                                                             ) const {//version 2: multi-byte fetching
	unsigned int total_matches=0;	
	for (int j = 0; j < cur_size_vec.size(); j++)	total_matches += match_count[j];
	fp   << "REPORTS: Total matches: " << total_matches << endl;
	
	for (int j = 0; j < cur_size_vec.size(); j++) {
		for (unsigned i = 0; i < match_count[j]; i++) {
			map<unsigned, set<unsigned> >::const_iterator it = accepting_states_.find(match_array[match_vec_size*j + i].stat);		
			if (j==0) fp   << match_array[match_vec_size*j + i].off  << "::" << endl;
			else      fp   << match_array[match_vec_size*j + i].off + cur_size_vec[j-1] - (pad_size_vec.empty() ? 0 : pad_size_vec[j-1]) << "::" << endl;
			if (it != accepting_states_.end()) {
				set<unsigned>::iterator iitt;
				for (iitt = it->second.begin();	iitt != it->second.end(); ++iitt) {
#ifdef DEBUG
					fp   << "    Rule: " << *iitt + rulestartvec[gid] << endl;
#else
                    fp   << "    Rule: " << *iitt << endl;
#endif
				}
			}
		}
	}
}
//END
/*------------------------------------------------------------------------------------*/
TransitionGraph *load_nfa_file(const char *pattern_name, unsigned int gid, unsigned int *trans_per_sym) {//Changed
	int trn_good = 0;
	vector<ifstream *> trn_file;
	ifstream file;
	
	// Setting for no multistride
	cfg.set_max_out(256);

	/* Load the translation tries from disk */
	if (pattern_name) {
		vector<string> prefixes;
		prefixes.push_back("/2");
		prefixes.push_back("/4");
		prefixes.push_back("/8");
		prefixes.push_back("/F");

		pair<string, string> p = utils::dir_base_name(pattern_name);
		
		//cout << "cfg.get_good_tries() = " << cfg.get_good_tries() << endl;
		
		string dump_filename;
		if (cfg.get_good_tries()) {
			dump_filename = p.first + prefixes[cfg.get_good_tries() - 1] + p.second 
				+ ".nfa";
		}
		else {
			dump_filename = p.first + "/" + p.second + ".nfa";//orig
			//dump_filename = p.first + "/" + p.second + ".dumpdfa";//testing
			//cout << "pattern_name = " << pattern_name << ", p.first = " << p.first << ", p.second = " << p.second << ", dump_filename = " << dump_filename << endl;
		}

		file.open(dump_filename.c_str());
		if (!file.good()) {
			cout << "Impossibile aprire il file " << dump_filename << endl;
			return NULL;
		}

		for(unsigned int i = 0; i < prefixes.size() &&	i < cfg.get_good_tries(); ++i) {
			string cur_name = p.first + prefixes[i] + p.second + ".translation";
			trn_file.push_back(new ifstream(cur_name.c_str()));
			cfg.get_mutable_max_outs()[i+1] = 0;
			if(trn_file[i]->good()) {
				cfg.add_trns(new HalfTrie<symbol_t, out_symbol_t>(*trn_file[i],
							cfg.get_mutable_max_outs()[i+1]));
				++trn_good;
			} else
				clog << "No good " << i << '\n';
		}	
		
		if (cfg.get_good_tries() > 0) {
#ifdef DEBUG
			cout << "Good tries: " << trn_good << endl;
#endif
			assert(trn_good);
			cfg.set_good_tries(trn_good);
			cfg.set_max_out(cfg.get_mutable_max_outs()[trn_good]);
		}
	} else {
		cout << "Ruleset name is invalid" << endl;
		return NULL;
	}
	
	/* Read the state count from the first transition graph line */
	{
		string line("#");
		
		unsigned int t;
		
		while (line[0] == '#') {
			if(!getline(file, line))
				error("reading NFA size", NFA_ERROR);

			istringstream iss(line);
			iss >> t;
		}
		cfg.set_state_count(t);
	}
		
	/* Read the transition graph from disk */
	TransitionGraph *tg = new TransitionGraph(file, cfg.get_allocator(), gid, trans_per_sym);
	
	/* Create the state vectors */
	tg->tmp_states_ = new StateVector(tg->get_mutable_initial_states());//this might cause memory overwritten, conflicts the resizing of memory footprint of group_offset_optim
	cfg.set_state_vector(tg->tmp_states_);
	
	return tg;
}
/*------------------------------------------------------------------------------------*/


ST_BLOCK *TransitionGraph::get_d_nfa_table() const {
	return d_nfa_table_;
}

ST_BLOCK *TransitionGraph::get_d_src_table() const {
	return d_src_table_;
}

unsigned int *TransitionGraph::get_d_offset_table() const {
	return d_offset_table_;
}

st_t *TransitionGraph::get_nfa_table() {
	return nfa_table_;
}

st_t *TransitionGraph::get_src_table() {
	return src_table_;
}

#ifdef GROUPS1
unsigned short *TransitionGraph::get_filter_table() {
	return filter_table_;
}
unsigned short *TransitionGraph::get_d_filter_table() const {
	return d_filter_table_;
}
#else
st_t *TransitionGraph::get_filter_table() {
	return filter_table_;
}
ST_BLOCK *TransitionGraph::get_d_filter_table() const {
	return d_filter_table_;
}
#endif

unsigned int *TransitionGraph::get_offset_table() {
	return offset_table_;
}

unsigned int *TransitionGraph::get_filter_symbol_offset() {
	return filter_symbol_offset;
}

unsigned int *TransitionGraph::get_filter_symbol_counts() {
	return filter_symbol_counts;
}

unsigned short *TransitionGraph::get_helper_table() {
	return helper_table_;
}

StateVector &TransitionGraph::get_mutable_persistent_states() {
	return persistent_states_;
}

StateVector &TransitionGraph::get_mutable_initial_states() {
	return initial_states_;
}

StateVector &TransitionGraph::get_accept_states() {
	return accept_states_;
}

std::map<unsigned, std::set<unsigned> > &TransitionGraph::get_accepting_states_() {
//Returning a reference means that the calling code can modify the value of your member variable after you return. 
	return accepting_states_;
}

size_t TransitionGraph::get_nfa_table_size() const {
	return nfa_table_size_;
}

size_t TransitionGraph::get_offset_table_size() const {
	return offset_table_size_;
}

size_t TransitionGraph::get_filter_size() const {
	return filter_size_;
}

unsigned int TransitionGraph::get_alphabet_size() const {
	return alphabet_size_;
}

void TransitionGraph::free_devmem(){
	all_.dealloc_device(d_nfa_table_);
	all_.dealloc_device(d_src_table_);
	all_.dealloc_device(d_offset_table_);	
}

unsigned int TransitionGraph::get_transition_count() const {
	return transition_count_;
}//For testing only

void TransitionGraph::set_nfa_table_size(size_t new_size){
	nfa_table_size_ = new_size;
}

void TransitionGraph::set_offset_table_size(size_t new_size){
	offset_table_size_ = new_size;	
}

unsigned int *TransitionGraph::get_group_offset() {
	return group_offset_;
}

st_t *TransitionGraph::get_nfa_table_optim() {
	return nfa_table_optim_;
}

st_t *TransitionGraph::get_src_table_optim() {
	return src_table_optim_;
}

void TransitionGraph::free_hostmem(){
	all_.dealloc_host(nfa_table_);
	all_.dealloc_host(src_table_);
	all_.dealloc_host(offset_table_);
	all_.dealloc_host(filter_table_);
}
