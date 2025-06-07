#!/usr/bin/env python3
import csv
import sys
import matplotlib.pyplot as plt


def main():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <coords.csv> <solution.sol> <output.png>")
        sys.exit(1)

    csv_path, sol_path, out_path = sys.argv[1], sys.argv[2], sys.argv[3]

    # --- load coordinates ---
    coords = []
    with open(csv_path, newline='') as f:
        reader = csv.reader(f)
        next(reader)  # skip header
        for row in reader:
            coords.append((float(row[0]), float(row[1])))

    # --- load tour order ---
    # with open(sol_path) as f:
    #     lines = [l.strip() for l in f if l.strip()]
    #     tour = [int(i) for i in lines[1].split()]
    #     tour.append(tour[0])  # close loop

    # --- load tour order ---
    with open(sol_path) as f:
        # strip out empty lines
        lines = [l.strip() for l in f if l.strip()]

    # skip the header (lines[0]), then for each remaining line grab the first
    # field
    tour = [int(line.split()[0]) for line in lines[1:]]
    tour.append(tour[0])  # close loop

    # --- prepare data ---
    lats = [coords[i][0] for i in tour]
    lons = [coords[i][1] for i in tour]

    # --- plot setup ---
    fig, ax = plt.subplots(figsize=(10, 8), dpi=300)

    # draw arrows between each consecutive pair
    for i in range(len(tour) - 1):
        ax.annotate(
            "",
            xy=(lons[i+1], lats[i+1]),
            xytext=(lons[i],   lats[i]),
            arrowprops=dict(
                arrowstyle="->",
                lw=2,
                mutation_scale=20,  # bigger heads
                shrinkA=0, shrinkB=0
            )
        )

    # draw the points
    ax.scatter(lons, lats, s=30, zorder=3)

    # label each point with a small offset so labels never touch the edge
    for idx in tour[:-1]:
        lat, lon = coords[idx]
        ax.annotate(
            str(idx),
            xy=(lon, lat),
            xytext=(5, 5),                # offset in points
            textcoords="offset points",
            ha="right", va="bottom",
            fontsize=9,                   # fixed here
            clip_on=False
        )

    ax.set_xlabel("Longitude")
    ax.set_ylabel("Latitude")
    ax.set_title("TSP Tour Order")
    ax.grid(True)

    # add a little margin so even offset labels show up
    ax.margins(0.1)

    # save high-res with tight bounding box
    fig.savefig(out_path,
                bbox_inches="tight",
                pad_inches=0.1,
                dpi=300)
    plt.close(fig)
    print(f"Saved tour plot to {out_path}")


if __name__ == "__main__":
    main()
