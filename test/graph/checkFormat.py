import sys

def is_symmetric_mtx(filename):
    edges = set()
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('%') or line.startswith('%%'):
                continue
            parts = line.split()
            if len(parts) < 3:
                continue  # skip malformed
            u, v, w = int(parts[0]), int(parts[1]), float(parts[2])
            edges.add((u, v, w))

    for u, v, w in edges:
        if (v, u, w) not in edges:
            print(f"Missing symmetric edge for ({u}, {v}, {w})")
            return False

    return True


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python check_symmetric.py <graph.mtx>")
        sys.exit(1)

    filename = sys.argv[1]
    if is_symmetric_mtx(filename):
        print("Graph is symmetric")
    else:
        print("Graph is NOT symmetric")
