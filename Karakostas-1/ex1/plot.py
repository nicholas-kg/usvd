
import csv
import matplotlib.pyplot as plt
from collections import defaultdict

# Read benchmark results
data = []

with open("results.csv", "r") as file:
    reader = csv.DictReader(file)

    for row in reader:
        data.append({
            "N": int(row["N"]),
            "Power": int(row["Power"]),
            "Threads": int(row["Threads"]),
            "Sequential": float(row["Sequential"]),
            "Parallel": float(row["Parallel"])
        })

powers = sorted(set(row["Power"] for row in data))
thread_counts = sorted(set(row["Threads"] for row in data))

for power in powers:

    power_data = [row for row in data if row["Power"] == power]

    # Average sequential times across thread configurations
    sequential = defaultdict(list)
    parallel = defaultdict(list)

    for row in power_data:
        sequential[row["N"]].append(row["Sequential"])
        parallel[(row["N"], row["Threads"])].append(row["Parallel"])

    sizes = sorted(sequential.keys())

    seq_times = [
        sum(sequential[n]) / len(sequential[n])
        for n in sizes
    ]

    
    # -------------------------------
    # Graph 1: Execution times
    # -------------------------------
    fig, ax = plt.subplots(figsize=(11, 7))

    ax.plot(
        sizes,
        seq_times,
        marker="o",
        markersize=8,
        linewidth=2.5,
        linestyle="-",
        label="Sequential",
    )

    markers = ["s", "^", "D"]
    linestyles = ["--", "-.", ":"]

    for i, threads in enumerate(thread_counts):
        times = [
            sum(parallel[(n, threads)]) / len(parallel[(n, threads)])
            for n in sizes
        ]

        ax.plot(
            sizes,
            times,
            marker=markers[i],
            markersize=7,
            linewidth=2,
            linestyle=linestyles[i],
            label=f"Parallel ({threads} threads)",
        )

    ax.set_xlabel("Matrix size (N)", fontsize=12)
    ax.set_ylabel("Execution time (seconds)", fontsize=12)
    ax.set_title(
        f"Execution Time Comparison (Power = {power})",
        fontsize=15,
        pad=15,
    )

    ax.set_yscale("log")
    ax.set_xticks(sizes)
    ax.set_xticklabels(sizes)

    ax.grid(True, which="major", linestyle="-", alpha=0.4)
    ax.grid(True, which="minor", linestyle=":", alpha=0.2)

    ax.legend(fontsize=11)
    ax.tick_params(labelsize=10)

    fig.tight_layout()
    fig.savefig(f"execution_time_power_{power}.png", dpi=300)
    plt.close(fig)


    # -------------------------------
    # Graph 2: Speedup
    # -------------------------------
    plt.figure(figsize=(9, 6))

    for threads in thread_counts:
        speedups = []

        for n in sizes:
            par_time = (
                sum(parallel[(n, threads)]) /
                len(parallel[(n, threads)])
            )

            seq_time = (
                sum(sequential[n]) /
                len(sequential[n])
            )

            speedups.append(seq_time / par_time)

        plt.plot(
            sizes,
            speedups,
            marker="o",
            label=f"{threads} threads"
        )

    plt.axhline(y=1, linestyle="--", label="No speedup")

    plt.xlabel("Matrix size (N)")
    plt.ylabel("Speedup")
    plt.title(f"Parallel Speedup - Power {power}")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"speedup_power_{power}.png", dpi=300)
    plt.close()

print("Graphs generated successfully!")
