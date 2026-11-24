import csv
from collections import defaultdict
import matplotlib.pyplot as plt


def main():
    # data[experiment][lock_name] = list of (x_val, time_sec)
    data = defaultdict(lambda: defaultdict(list))

    with open("results.csv", "r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            exp = row["experiment"]
            lock_name = row["lock_name"]
            time_sec = float(row["time_sec"])

            if exp == "accounts":
                x = int(row["accounts"])
            elif exp == "transactions":
                x = int(row["transactions"])
            else:
                x = int(row["inquiry_pct"])

            data[exp][lock_name].append((x, time_sec))

    fig, axes = plt.subplots(1, 3, figsize=(16, 5.0))
    configs = [
        ("accounts", "Number of Accounts (log scale)", "Execution Time vs Accounts", True),
        ("transactions", "Transactions per Thread", "Execution Time vs Transactions", False),
        ("inquiry_pct", "Inquiry Percentage (%)", "Execution Time vs Inquiry Ratio", False),
    ]
    markers = {
        "Coarse_Mutex": "o",
        "Coarse_RWLock": "s",
        "Fine_Mutex": "^",
        "Fine_RWLock": "D",
    }

    for ax, (exp, xlabel, title, use_log) in zip(axes, configs):
        for lock_name in ["Coarse_Mutex", "Coarse_RWLock", "Fine_Mutex", "Fine_RWLock"]:
            pts = sorted(data[exp][lock_name], key=lambda p: p[0])
            if not pts:
                continue
            xs = [p[0] for p in pts]
            ys = [p[1] for p in pts]
            ax.plot(xs, ys, marker=markers[lock_name], linewidth=2, label=lock_name)

        if use_log:
            try:
                ax.set_xscale("log", base=2)
            except ValueError:
                ax.set_xscale("log", basex=2)

        ax.set_title(title, pad=10)
        ax.set_xlabel(xlabel)
        ax.set_ylabel("Execution Time (sec)")
        ax.grid(True, linestyle="--", alpha=0.6)
        ax.legend()

    plt.suptitle(
        "Bank Simulation Synchronization Comparison (4 Threads)",
        fontsize=13,
        fontweight="bold",
        y=0.98,
    )
    plt.tight_layout(rect=[0, 0, 1, 0.92])
    plt.savefig("bank_benchmark.png", dpi=300)
    print("Plot saved to bank_benchmark.png")


if __name__ == "__main__":
    main()