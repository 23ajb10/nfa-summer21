.PHONY: release real

CUDA_OBJ = cuda_wrappers gpu_functions host_functions nfa_engine burst
HOST_OBJ = cuda_allocator config_options globals half_trie sparse transition_graph state_vector utils
COMMON_HEADERS = common.h

NVCC=nvcc
#SM=sm_35
#SM=sm_52
SM=sm_61
CXX=g++
CC=gcc

######CUDA_INCLUDE=  -I/usr/local/cuda/include
CUDA_INCLUDE= -I/usr/local/cuda-11.0/include -I/home/cs-students/21att1/bin/boost_1_75_0 -I/home/cs-students/21att1/bin/libpcap-1.9.1 

CXXFLAGS+=$(CUDA_INCLUDE) -Wno-deprecated  -DGROUPS -DTRIMMED #-DCOUNTERS  -DSELECT  # -DSELECT #-DGROUPS ##-DCOUNTERS ##-DGROUPS  -DTRIMMED -dlcm=cg
NVCCFLAGS+=$(CUDA_INCLUDE) -Xptxas -dlcm=cg -v -arch ${SM} --compiler-options -Wno-deprecated -lineinfo  -DGROUPS -DTRIMMED #-DCOUNTERS -DSELECT  #-DGROUPS -DTRIMMED -DSELECT #-DGROUPS ##-DCOUNTERS -DGROUPS ## -DTEXTURE_MEM_USE -dlcm=cg -DTRIMMED

export DYN_LIB=  -L/home/cs-students/21att1/bin/libpcap-1.9.1 -lpcap -L/home/cs-students/21att1/bin/boost_1_75_0/stage/lib -lboost_regex -L/home/cs-students/21att1/bin/pcre-8.44/lib -lpcre

NVCCFLAGS_REL = $(NVCCFLAGS) -O4

CXXFLAGS_REL = $(CXXFLAGS) -O4

release:
	$(MAKE) -e real NVCCFLAGS="$(NVCCFLAGS_REL)" CXXFLAGS="$(CXXFLAGS_REL)"

real: nfa_engine

$(addsuffix .o, $(HOST_OBJ)) $(addsuffix .o, $(CUDA_OBJ)) : $(COMMON_HEADERS)

$(addsuffix .o, $(HOST_OBJ)) : $(addsuffix .cpp, $(basename $@)) $(addsuffix .h, $(basename $@))

$(addsuffix .o, $(CUDA_OBJ)) : $(addsuffix .cu, $(basename $@)) $(addsuffix .h, $(basename $@))
	${NVCC} $(NVCCFLAGS) -c -o $(addsuffix .o, $(basename $@)) $(addsuffix .cu, $(basename $@))
	
nfa_engine: $(addsuffix .o, $(HOST_OBJ)) $(addsuffix .o, $(CUDA_OBJ))
	${NVCC} $(NVCCFLAGS) -o nfa_engine $(addsuffix .o, $(HOST_OBJ)) $(addsuffix .o, $(CUDA_OBJ)) ${DYN_LIB}

clean:
	rm -f *.o nfa_engine
