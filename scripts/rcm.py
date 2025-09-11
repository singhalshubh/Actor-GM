#!/usr/bin/env python3
# Reads a weighted Matrix Market (.mtx), computes RCM on the structural pattern,
# applies the permutation to the ORIGINAL weighted matrix, and writes weighted .mtx
# with NO comment lines. Preserves (u,v,w) and (v,u,w) exactly.

import argparse
import numpy as np
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import reverse_cuthill_mckee
from scipy.io import mmread

def write_mtx_no_comment(A: csr_matrix, path: str) -> None:
    C = A.tocoo()
    with open(path, "w") as f:
        f.write("%%MatrixMarket matrix coordinate real general\n")
        f.write(f"{C.shape[0]} {C.shape[1]} {C.nnz}\n")
        r, c, x = C.row, C.col, C.data
        for i in range(C.nnz):
            # 1-based indices; weights preserved
            f.write(f"{r[i]+1} {c[i]+1} {float(x[i]):.16g}\n")

def main():
    ap = argparse.ArgumentParser(description="RCM on structure; preserve weights/directions; write weighted .mtx (no % line)")
    ap.add_argument("-i", "--input", required=True, help="Input weighted Matrix Market (.mtx)")
    ap.add_argument("-o", "--output", required=True, help="Output weighted Matrix Market (.mtx)")
    args = ap.parse_args()

    # Load weighted matrix as-is
    A: csr_matrix = mmread(args.input).tocsr()
    A.setdiag(0); A.eliminate_zeros()  # drop self-loops if any

    # Structural, undirected pattern for RCM only (weights NOT used)
    patt = A.copy()
    patt.data[:] = 1.0
    patt = ((patt + patt.T) > 0).astype(np.int8).tocsr()

    # RCM permutation (0-based indices)
    p = reverse_cuthill_mckee(patt, symmetric_mode=True)
    p = np.asarray(p, dtype=np.int64)

    # Apply permutation to ORIGINAL weighted matrix (preserves both directions & weights)
    A_perm = A[p][:, p]

    # Write Matrix Market without extra '%' comment line
    write_mtx_no_comment(A_perm, args.output)

if __name__ == "__main__":
    main()
