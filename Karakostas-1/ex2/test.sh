#!/usr/bin/env bash
set -e

EXEC="./counter"
OUT="results.csv"

if [ ! -x "$EXEC" ]; then
    echo "Executable $EXEC not found. Please run 'make' first."
    exit 1
fi

echo "mode,mode_name,threads,iterations,final_val,expected,time_sec" > "$OUT"

THREADS_LIST=(1 2 4 8)
ITERS_LIST=(100000 1000000 5000000)

declare -A MODE_NAMES
MODE_NAMES[1]="Mutex"
MODE_NAMES[2]="RWLock"
MODE_NAMES[3]="Atomic"

echo "Starting benchmarks..."

for iters in "${ITERS_LIST[@]}"; do
    for threads in "${THREADS_LIST[@]}"; do
        for mode in 1 2 3; do
            res=$($EXEC $mode $threads $iters)
            IFS=',' read -r m t it fval exp tm <<< "$res"
            echo "$m,${MODE_NAMES[$m]},$t,$it,$fval,$exp,$tm" >> "$OUT"
            printf "Mode: %-7s | Threads: %-2s | Iters: %-8s | Val: %-10s | Time: %s s\n" \
                   "${MODE_NAMES[$m]}" "$t" "$it" "$fval" "$tm"
        done
    done
done

echo "Completed! Results saved to $OUT."