#!/usr/bin/env python3
import csv
import sys
import matplotlib.pyplot as plt


def main():
    if len(sys.argv) != 4:
        print(
            f"Usage: {sys.argv[0]} <path/to/coords.csv> <path/to/solution.sol> <path/to/output.png>")
        sys.exit(1)

    csv_path = sys.argv[1]
    sol_path = sys.argv[2]
    out_path = sys.argv[3]

    # Read coordinates (expects header "latitude,longitude")
    coords = []
    with open(csv_path, newline='') as f:
        reader = csv.reader(f)
        next(reader)
        for row in reader:
            lat = float(row[0])
            lon = float(row[1])
            coords.append((lat, lon))

    # Read Concorde solution (first line = number of nodes, second line = tour
    # order)
    with open(sol_path) as f:
        lines = [line.strip() for line in f if line.strip()]
        tour_indices = [int(idx) for idx in lines[1].split()]

    # Close the tour loop
    tour_indices.append(tour_indices[0])

    # Collect lat/lon in tour order
    tour_lats = [coords[i][0] for i in tour_indices]
    tour_lons = [coords[i][1] for i in tour_indices]

    # Plot
    plt.figure()
    plt.plot(tour_lons, tour_lats, marker='o', linestyle='-')
    plt.title("TSP Tour Order")
    plt.xlabel("Longitude")
    plt.ylabel("Latitude")
    plt.grid(True)

    # Annotate each point (skip duplicate at end)
    for i in tour_indices[:-1]:
        lat, lon = coords[i]
        plt.text(lon, lat, str(i), fontsize=9, ha='right', va='bottom')

    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()
    print(f"Tour plot saved to {out_path}")


if __name__ == "__main__":
    main()
