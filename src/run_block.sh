datasets=("cage15.mtx" "HV15R.mtx" "graph500-scale21-ef16_adj.mtx" "graph500-scale22-ef16_adj.mtx" "graph500-scale23-ef16_adj.mtx" "graph500-scale24-ef16_adj.mtx" "kmer_P1a.mtx" "kmer_U1a.mtx" "kmer_V1r.mtx" "kmer_V2a.mtx" "static_highOverlap_lowBlockSizeVar_1000000_nodes.mtx" "static_highOverlap_lowBlockSizeVar_5000000_nodes.mtx" "static_highOverlap_lowBlockSizeVar_20000000_nodes.mtx" "com-Orkut.mtx" "com-Friendster.mtx")
cores=(4 8 16 32 64 128)

d_path=/storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/converted

for ds in "${datasets[@]}"; do
  for c in "${cores[@]}"; do
    echo "== $ds | $c cores =="

    # run each job with a 10-minute (600s) timeout
    if ! timeout 300 srun -N $c -n $((16*c)) ./block -f $d_path/$ds; then
      echo "$ds on $c cores did not finish within 5 minutes, skipping..."
    fi

    echo -e "\n"
  done
done