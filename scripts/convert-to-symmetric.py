#!/usr/bin/env python3
import sys

def convert_general_to_symmetric(input_file, output_file):
    edges = set()
    weights = {}
    header_lines = []
    data_lines = []

    with open(input_file, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('%'):
                header_lines.append(line)
                continue
            parts = line.split()
            if len(parts) < 2:
                continue  # Skip malformed lines
            u, v = int(parts[0]), int(parts[1])
            w = float(parts[2]) if len(parts) >= 3 else None

            # Undirected edge normalization
            key = (u, v) if u < v else (v, u)
            if key not in edges:
                edges.add(key)
                weights[key] = w

    # Prepare new header: convert 'general' to 'symmetric'
    if header_lines:
        header_lines[0] = header_lines[0].replace('general', 'symmetric')

    # Write output
    with open(output_file, 'w') as f:
        for line in header_lines:
            f.write(line + "\n")
        f.write(f"{len(edges)} {len(edges)} {len(edges)}\n")  # symmetric square matrix

        for (u, v), w in sorted(weights.items()):
            if w is None:
                f.write(f"{u} {v}\n")
            else:
                f.write(f"{u} {v} {w}\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input.mtx output.mtx")
        sys.exit(1)

    convert_general_to_symmetric(sys.argv[1], sys.argv[2])
