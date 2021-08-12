#!/bin/bash
#echo "Collection Filter Cache Off --------------------------" >> data-collection/symbol-group-dist/results
# echo "BRILL --------------------------" > data-collection/symbol-group-dist/brill

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/brill -i ../nfas/AutomataZoo/inputs/brill.txt -T 1024 -g 1 -p 1 -N 5946 -O 0 >> data-collection/symbol-group-dist/brill

# # sudo ncu --csv --metrics=dram__bytes_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum bin/nfa_engine -a ../nfas/AutomataZoo/automata/brill -i ../nfas/AutomataZoo/inputs/brill.txt -T 1024 -g 1 -p 1 -N 5946 -O 0 >> data-collection/symbol-group-dist/results

# echo "PROTOMATA --------------------------" > data-collection/symbol-group-dist/protomata

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/Protomata -i ../AutomataZoo/Protomata/benchmarks/inputs/30k_prots.input -T 1024 -g 1 -p 1 -N 1309 -O 0 >> data-collection/symbol-group-dist/protomata

# # sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/Protomata -i ../AutomataZoo/Protomata/benchmarks/inputs/30k_prots.input -T 1024 -g 1 -p 1 -N 1309 -O 0 >> data-collection/symbol-group-dist/results

# echo "SEQMATCH --------------------------" > data-collection/symbol-group-dist/seqmatch

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/SeqMatch_w6p6 -i ../nfas/AutomataZoo/inputs/seq.bin -T 1024 -g 1 -p 1 -N 1719 -O 0 >> data-collection/symbol-group-dist/seqmatch
# # sudo ncu --csv --metrics=dram__bytes_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum bin/nfa_engine -a ../nfas/AutomataZoo/automata/SeqMatch_w6p6 -i ../nfas/AutomataZoo/inputs/seq.bin -T 1024 -g 1 -p 1 -N 1719 -O 0 >> data-collection/symbol-group-dist/results

# # sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/SeqMatch_w6p6 -i ../nfas/AutomataZoo/inputs/seq.bin -T 1024 -g 1 -p 1 -N 1719 -O 0 >> data-collection/symbol-group-dist/results

# # echo "SEL" >> data-collection/symbol-group-dist/results

# echo "CRISPR --------------------------" > data-collection/symbol-group-dist/crispr

# #sudo ncu --csv --metrics=dram__bytes_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum bin/nfa_engine -a ../nfas/AutomataZoo/automata/CRISPR_CasOFFinder -i ../nfas/AutomataZoo/inputs/CRISPR_CasOFFinder.txt -T 1024 -g 1 -p 1 -N 2000 -O 0 >> data-collection/symbol-group-dist/crispr
# bin/nfa_engine -a ../nfas/AutomataZoo/automata/CRISPR_CasOFFinder -i ../nfas/AutomataZoo/inputs/CRISPR_CasOFFinder.txt -T 1024 -g 1 -p 1 -N 2000 -O 0 >> data-collection/symbol-group-dist/crispr


# sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_#ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/CRISPR_CasOFFinder -i ../nfas/AutomataZoo/inputs/CRISPR_CasOFFinder.txt -T 1024 -g 1 -p 1 -N 2000 -O 0 >> data-collection/symbol-group-dist/results



# echo "APPRNG --------------------------" >> results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/APPRNG_4sided -i ../AutomataZoo/APPRNG/benchmarks/4_sided/inputs/10MB_A.prng -T 1024 -g 1 -p 1 -N 1000 -O 0 >> results

# # sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/APPRNG_4sided -i ../AutomataZoo/APPRNG/benchmarks/4_sided/inputs/10MB_A.prng -T 1024 -g 1 -p 1 -N 1000 -O 0 >> results

# echo "ENTITYRESOLUTION --------------------------" >> results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/ER_10000names -i ../nfas/AutomataZoo/inputs/ER_1m_names.input -T 1024 -g 1 -p 1 -N 10000 -O 0 >> results

# # sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/ER_10000names -i ../nfas/AutomataZoo/inputs/ER_1m_names.input -T 1024 -g 1 -p 1 -N 10000 -O 0 >> results

# echo "FILECARVER --------------------------" >> results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/File_Carver -i ../AutomataZoo/FileCarving/benchmarks/inputs/fat32_files.input -T 1024 -g 1 -p 1 -N 9 -O 0 >> results

# # sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/File_Carver -i ../AutomataZoo/FileCarving/benchmarks/inputs/fat32_files.input -T 1024 -g 1 -p 1 -N 9 -O 0 >> results

# echo "HAMMING --------------------------" >> results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/Hamming_l18d3 -i ../AutomataZoo/Hamming/benchmarks/N1000_l18_d3/inputs/10MB_D.DNA -T 1024 -g 1 -p 1 -N 1000 -O 0 >> results

# # sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/Hamming_l18d3 -i ../AutomataZoo/Hamming/benchmarks/N1000_l18_d3/inputs/10MB_D.DNA -T 1024 -g 1 -p 1 -N 1000 -O 0 >> results

# echo "RANDOMFOREST --------------------------" >> rf-back-to-start

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/RemoteForest -i ../AutomataZoo/RandomForest/benchmarks/inputs/20_400_200_inputs/input_features_large.bin -T 1024 -g 1 -p 1 -N 8000 -O 0 >> rf-back-to-start

# # sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/RemoteForest -i ../AutomataZoo/RandomForest/benchmarks/inputs/20_400_200_inputs/input_features_large.bin -T 1024 -g 1 -p 1 -N 8000 -O 0 >> results

# echo "SNORT --------------------------" >> results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/Snort -i ../AutomataZoo/Snort/benchmarks/inputs/wrccdc2012.pcap -T 1024 -g 1 -p 1 -N 2486 -O 0 >> results

# # sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/Snort -i ../AutomataZoo/Snort/benchmarks/inputs/wrccdc2012.pcap -T 1024 -g 1 -p 1 -N 2486 -O 0 >> results

# echo "YARA --------------------------" >> results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/YARA -i ../AutomataZoo/YARA/benchmarks/YARA/inputs/malware_malz2.input -T 1024 -g 1 -p 1 -N 23530 -O 0 >> results

# # sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/YARA -i ../AutomataZoo/YARA/benchmarks/YARA/inputs/malware_malz2.input -T 1024 -g 1 -p 1 -N 23530 -O 0 >> results

# echo "CLAMAV --------------------------" >> results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/clamav -i ../AutomataZoo/ClamAv/benchmarks/inputs/clamav.input -T 1024 -g 1 -p 1 -N 33171 -O 0 >> results

# sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/clamav -i ../AutomataZoo/ClamAv/benchmarks/inputs/clamav.input -T 1024 -g 1 -p 1 -N 33171 -O 0 >> results


# echo "LEVENSHTEIN --------------------------" >> results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/Levenshtein_d3 -i ../AutomataZoo/Levenshtein/benchmarks/N1000_l19_d3/inputs/10MB_A.dna -T 1024 -g 1 -p 1 -N 1000 -O 0 >> results

# sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/Levenshtein_d3 -i ../AutomataZoo/Levenshtein/benchmarks/N1000_l19_d3/inputs/10MB_A.dna -T 1024 -g 1 -p 1 -N 1000 -O 0 >> results

# make clean && make

# echo "GRP HISTO ----------------------------------\n\n" >> data-collection/trimmed-groups/results

# echo "BRILL --------------------------" >> data-collection/trimmed-groups/results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/brill -i ../nfas/AutomataZoo/inputs/brill.txt -T 1024 -g 1 -p 1 -N 5946 -O 0 >> data-collection/trimmed-groups/results

# #sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/brill -i ../nfas/AutomataZoo/inputs/brill.txt -T 1024 -g 1 -p 1 -N 5946 -O 0 >> data-collection/trimmed-groups/results

# echo "PROTOMATA --------------------------" >> data-collection/trimmed-groups/results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/Protomata -i ../AutomataZoo/Protomata/benchmarks/inputs/30k_prots.input -T 1024 -g 1 -p 1 -N 1309 -O 0 >> data-collection/trimmed-groups/results

# #sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/Protomata -i ../AutomataZoo/Protomata/benchmarks/inputs/30k_prots.input -T 1024 -g 1 -p 1 -N 1309 -O 0 >> results

# echo "SEQMATCH --------------------------" >> results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/SeqMatch_w6p6 -i ../nfas/AutomataZoo/inputs/seq.bin -T 1024 -g 1 -p 1 -N 1719 -O 0 >> results

# #sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/SeqMatch_w6p6 -i ../nfas/AutomataZoo/inputs/seq.bin -T 1024 -g 1 -p 1 -N 1719 -O 0 >> results

# echo "CRISPR --------------------------" >> results

# bin/nfa_engine -a ../nfas/AutomataZoo/automata/CRISPR_CasOFFinder -i ../nfas/AutomataZoo/inputs/CRISPR_CasOFFinder.txt -T 1024 -g 1 -p 1 -N 2000 -O 0 >> results

# #sudo ncu --csv --metrics=dram__bytes_read.sum,dram__sectors_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld.sum,l1tex__t_requests_pipe_lsu_mem_global_op_ld.sum,smsp__inst_executed.sum,smsp__inst_executed_op_global_ld.sum,smsp__inst_executed_op_local_ld.sum,smsp__inst_executed_op_shared_ld.sum,sm__warps_active.avg.pct_of_peak_sustained_active,lts__t_sectors_op_read.sum,lts__t_sectors_op_atom.sum,lts__t_sectors_op_red.sum,lts__t_sectors_srcunit_tex_op_read.sum,l1tex__t_sectors_pipe_lsu_mem_global_op_ld_lookup_hit.sum,lts__t_sector_hit_rate.pct bin/nfa_engine -a ../nfas/AutomataZoo/automata/CRISPR_CasOFFinder -i ../nfas/AutomataZoo/inputs/CRISPR_CasOFFinder.txt -T 1024 -g 1 -p 1 -N 2000 -O 0 >> results