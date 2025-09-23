datasets=("cage15.bin" "HV15R.bin" "graph500-scale21-ef16_adj.bin" "graph500-scale22-ef16_adj.bin" "graph500-scale23-ef16_adj.bin" "graph500-scale24-ef16_adj.bin" "kmer_P1a.bin" "kmer_U1a.bin" "kmer_V1r.bin" "kmer_V2a.bin" "static_highOverlap_lowBlockSizeVar_1000000_nodes.bin" "static_highOverlap_lowBlockSizeVar_5000000_nodes.bin" "static_highOverlap_lowBlockSizeVar_20000000_nodes.bin" "com-Orkut.bin" "com-Friendster.bin" "rgg_small.bin" "rgg_medium.bin" "rgg_big.bin")
cores=(4 8 16 32 64 128)

d_path=$HOME/scratch/mel-dataset/converted

for ds in "${datasets[@]}"; do
  for c in "${cores[@]}"; do
    echo "== $ds | $c nodes =="

    # run each job with a 10-minute (600s) timeout
    if ! timeout 600 srun -N "$c" -n $((16*c)) ./match -f "$d_path/$ds"; then
      echo "$ds on $c cores did not finish within 5 minutes, skipping..."
    fi

    echo -e "\n"
  done
done
