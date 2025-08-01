import numpy as np
import sys

def mel_bin_to_mtx(bin_file, mtx_file):
    with open(bin_file, "rb") as f:
        # 1. Read header: (M_=vertices, N_=edges)
        header = np.fromfile(f, dtype=np.uint64, count=2)
        M = int(header[0])  # number of vertices
        N = int(header[1])  # number of edges

        # 2. Read prefix array (CSR offsets)
        offsets = np.fromfile(f, dtype=np.uint64, count=M+1)

        # 3. Read edge list: neighbor:int32, weight:float32
        edges = np.fromfile(f, dtype=[("nbr","<i8"),("weight","<f8")], count=N)

    # 4. Write MTX file directly (streaming to avoid memory blowup)
    with open(mtx_file, "w") as out:
        out.write("%%MatrixMarket matrix coordinate real symmetric\n")

        # Each edge generates two entries for undirected graph
        out.write(f"{M} {M} {N}\n")

        for u in range(M):
            start = offsets[u]
            end = offsets[u+1]
            for idx in range(start, end):
                v = int(edges[idx]["nbr"])
                w = float(edges[idx]["weight"])

                # 1-based indices for .mtx
                out.write(f"{u+1} {v+1} {w}\n")

    print(f"✅ Conversion complete: {bin_file} → {mtx_file}")
    print(f"Vertices: {M}, Edges: {N} (wrote {N} entries for undirected)")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 convert-to-mtx.py input.bin output.mtx")
        sys.exit(1)

    mel_bin_to_mtx(sys.argv[1], sys.argv[2])
