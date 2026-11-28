import csv
from collections import defaultdict
import matplotlib.pyplot as plt


def main():
    groups = defaultdict(list)
    iters_set = set()
    threads_set = set()

    with open("results_barriers.csv", "r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            mode_name = row["mode_name"]
            threads = int(row["threads"])
            iterations = int(row["iterations"])
            time_sec = float(row["time_sec"])

            groups[(iterations, mode_name)].append((threads, time_sec))
            iters_set.add(iterations)
            threads_set.add(threads)

    iters_list = sorted(iters_set)
    threads_list = sorted(threads_set)

    fig, axes = plt.subplots(1, len(iters_list), figsize=(5.5 * len(iters_list), 5.0))
    if len(iters_list) == 1:
        axes = [axes]

    markers = {
        "Pthread_Barrier": "o",
        "Cond_Var_Barrier": "s",
        "Sense_Reversal": "^",
        "Combining_Tree": "D",
    }

    for ax, iters in zip(axes, iters_list):
        for mode_name in ["Pthread_Barrier", "Cond_Var_Barrier", "Sense_Reversal", "Combining_Tree"]:
            pts = sorted(groups.get((iters, mode_name), []), key=lambda x: x[0])
            if not pts:
                continue
            xs = [p[0] for p in pts]
            ys = [p[1] for p in pts]
            ax.plot(xs, ys, marker=markers[mode_name], linewidth=2, label=mode_name)

        ax.set_title(f"Iterations N = {iters:,}", pad=10)
        ax.set_xlabel("Number of Threads")
        ax.set_ylabel("Execution Time (sec)")
        ax.set_xticks(threads_list)
        ax.grid(True, linestyle="--", alpha=0.6)
        ax.legend()

    plt.suptitle(
        "Barrier Implementations Performance Comparison",
        fontsize=13,
        fontweight="bold",
        y=0.98,
    )
    plt.tight_layout(rect=[0, 0, 1, 0.92])
    plt.savefig("barriers_comparison.png", dpi=300)
    print("Plot saved to barriers_comparison.png")


if __name__ == "__main__":
    main()