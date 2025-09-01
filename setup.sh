
wget https://suitesparse-collection-website.herokuapp.com/MM/SNAP/com-Orkut.tar.gz
tar -xvf com-Orkut.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/SNAP/com-Friendster.tar.gz
tar -xvf com-Friendster.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/vanHeukelum/cage15.tar.gz
tar -xvf cage15.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/Fluorem/HV15R.tar.gz
tar -xvf HV15R.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/GenBank/kmer_A2a.tar.gz
tar -xvf kmer_A2a.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/GenBank/kmer_P1a.tar.gz
tar -xvf kmer_P1a.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/GenBank/kmer_U1a.tar.gz
tar -xvf kmer_U1a.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/GenBank/kmer_V1r.tar.gz
tar -xvf kmer_V1r.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/GenBank/kmer_V2a.tar.gz
tar -xvf kmer_V2a.tar.gz

wget https://graphchallenge.s3.amazonaws.com/synthetic/graph500-scale21-ef16/graph500-scale21-ef16_adj.mmio.gz
gzip -df graph500-scale21-ef16_adj.mmio.gz

wget https://graphchallenge.s3.amazonaws.com/synthetic/graph500-scale22-ef16/graph500-scale22-ef16_adj.mmio.gz
gzip -df graph500-scale22-ef16_adj.mmio.gz

wget https://graphchallenge.s3.amazonaws.com/synthetic/graph500-scale23-ef16/graph500-scale23-ef16_adj.mmio.gz
gzip -df graph500-scale23-ef16_adj.mmio.gz

wget https://graphchallenge.s3.amazonaws.com/synthetic/graph500-scale24-ef16/graph500-scale24-ef16_adj.mmio.gz
gzip -df graph500-scale24-ef16_adj.mmio.gz

wget https://graphchallenge.s3.amazonaws.com/synthetic/partitionchallenge_2018/simulated_blockmodel_graph_highOverlap_lowBlockSizeVar.tar.gz
tar -xvf simulated_blockmodel_graph_highOverlap_lowBlockSizeVar.tar.gz
tar -xvf simulated_blockmodel_graph_1000000_nodes_static_highOverlap_lowBlockSizeVar.tar.gz
tar -xvf simulated_blockmodel_graph_5000000_nodes_static_highOverlap_lowBlockSizeVar.tar.gz
tar -xvf simulated_blockmodel_graph_20000000_nodes_static_highOverlap_lowBlockSizeVar.tar.gz

git clone https://github.com/ECP-ExaGraph/vite
cd vite 
make

python3 convert-to-symmetric.py $DATA_PATH/cage15/cage15.mtx $DATA_PATH/cage15/cage15-adjusted.mtx
python3 convert-to-symmetric.py $DATA_PATH/HV15R/HV15R.mtx $DATA_PATH/HV15R/HV15R-adjusted.mtx

export DATA_PATH=/storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/cage15/cage15-adjusted.mtx -o $DATA_PATH/converted/cage15.bin
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/HV15R/HV15R-adjusted.mtx -o $DATA_PATH/converted/HV15R.bin
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/com-Orkut/com-Orkut.mtx -o $DATA_PATH/converted/com-Orkut.bin
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/com-Friendster/com-Friendster.mtx -o $DATA_PATH/converted/com-Friendster.bin
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/kmer_P1a/kmer_P1a.mtx -o $DATA_PATH/converted/kmer_P1a.bin
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/kmer_U1a/kmer_U1a.mtx -o $DATA_PATH/converted/kmer_U1a.bin
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/kmer_V1r/kmer_V1r.mtx -o $DATA_PATH/converted/kmer_V1r.bin
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/kmer_V2a/kmer_V2a.mtx -o $DATA_PATH/converted/kmer_V2a.bin
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/graph500-scale21-ef16_adj.mmio -o $DATA_PATH/converted/graph500-scale21-ef16_adj.bin
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/graph500-scale22-ef16_adj.mmio -o $DATA_PATH/converted/graph500-scale22-ef16_adj.bin
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/graph500-scale23-ef16_adj.mmio -o $DATA_PATH/converted/graph500-scale23-ef16_adj.bin
srun -n 1 ./fileConvert -m -r -f $DATA_PATH/graph500-scale24-ef16_adj.mmio -o $DATA_PATH/converted/graph500-scale24-ef16_adj.bin
srun -n 1 ./fileConvert -u -r -f $DATA_PATH/static/highOverlap_lowBlockSizeVar/static_highOverlap_lowBlockSizeVar_1000000_nodes.tsv -o $DATA_PATH/converted/static_highOverlap_lowBlockSizeVar_1000000_nodes.bin
srun -n 1 ./fileConvert -u -r -f $DATA_PATH/static/highOverlap_lowBlockSizeVar/static_highOverlap_lowBlockSizeVar_5000000_nodes.tsv -o $DATA_PATH/converted/static_highOverlap_lowBlockSizeVar_5000000_nodes.bin
srun -n 1 ./fileConvert -u -r -f $DATA_PATH/static/highOverlap_lowBlockSizeVar/static_highOverlap_lowBlockSizeVar_20000000_nodes.tsv -o $DATA_PATH/converted/static_highOverlap_lowBlockSizeVar_20000000_nodes.bin
 
export DATA_PATH=/storage/home/hcoda1/8/ssinghal74/scratch/mel-dataset/
python3 convert-to-mtx.py $DATA_PATH/converted/cage15.bin $DATA_PATH/converted/cage15.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/HV15R.bin $DATA_PATH/converted/HV15R.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/com-Orkut.bin $DATA_PATH/converted/com-Orkut.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/com-Friendster.bin $DATA_PATH/converted/com-Friendster.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/kmer_V2a.bin $DATA_PATH/converted/kmer_V2a.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/kmer_P1a.bin $DATA_PATH/converted/kmer_P1a.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/kmer_U1a.bin $DATA_PATH/converted/kmer_U1a.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/kmer_V1r.bin $DATA_PATH/converted/kmer_V1r.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/graph500-scale21-ef16_adj.bin $DATA_PATH/converted/graph500-scale21-ef16_adj.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/graph500-scale22-ef16_adj.bin $DATA_PATH/converted/graph500-scale22-ef16_adj.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/graph500-scale23-ef16_adj.bin $DATA_PATH/converted/graph500-scale23-ef16_adj.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/graph500-scale24-ef16_adj.bin $DATA_PATH/converted/graph500-scale24-ef16_adj.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/static_highOverlap_lowBlockSizeVar_1000000_nodes.bin $DATA_PATH/converted/static_highOverlap_lowBlockSizeVar_1000000_nodes.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/static_highOverlap_lowBlockSizeVar_5000000_nodes.bin $DATA_PATH/converted/static_highOverlap_lowBlockSizeVar_5000000_nodes.mtx
python3 convert-to-mtx.py $DATA_PATH/converted/static_highOverlap_lowBlockSizeVar_20000000_nodes.bin $DATA_PATH/converted/static_highOverlap_lowBlockSizeVar_20000000_nodes.mtx

srun -N 1 -n 24 ./match -f $DATA_PATH/converted/graph500-scale22-ef16_adj.bin -r 24

export OMP_NUM_THREADS=24
rm -rf matching.net 
srun -n 1 -c 24 ./bMatching -f $DATA_PATH/converted/graph500-scale22-ef16_adj.mtx -b 1
wc -l matching.net

export OMP_NUM_THREADS=24
rm -rf matching.net 
srun -n 1 -c 24 ./bMatching -f $DATA_PATH/converted/HV15R.mtx -b 1
wc -l matching.net

export OMP_NUM_THREADS=24
rm -rf matching.net 
srun -n 1 -c 24 ./bMatching -f $DATA_PATH/converted/com-Friendster.mtx -b 1
wc -l matching.net