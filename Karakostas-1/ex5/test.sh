#!/usr/bin/env bash
set -e

OUT="results_barriers.csv"

for exe in ./barrier ./barrier_cond ./barrier_sense ./barrier_tree; do
    if [ ! -x "$exe" ]; then
        echo "Executable $exe not found. Please run 'make' first."
        exit 1
    fi
done

echo "mode,mode_name,threads,iterations,time_sec" > "$OUT"

THREADS_LIST=(4 8)
# Note: barrier_cond takes exponent n (10^n), while the others take exact iterations
EXPONENTS=(4 5)

echo "Starting barrier benchmarks..."

for exp in "${EXPONENTS[@]}"; do
    iters=$((10 ** exp))
    for threads in "${THREADS_LIST[@]}"; do
        # 1. POSIX pthread_barrier (barrier.c)
        out1=$(./barrier $threads $iters)
        t1=$(echo "$out1" | grep -oE '[0-9]+\.[0-9]+' | tail -n 1)
        echo "1,Pthread_Barrier,$threads,$iters,$t1" >> "$OUT"
        printf "Mode: %-17s | Threads: %-2s | Iters: %-8s | Time: %s s\n" "Pthread_Barrier" "$threads" "$iters" "$t1"

        # 2. Condition Variable barrier (barrier_cond.c - expects exponent n for 10^n)
        out2=$(./barrier_cond $threads $exp)
        t2=$(echo "$out2" | grep -oE '[0-9]+\.[0-9]+' | tail -n 1)
        echo "2,Cond_Var_Barrier,$threads,$iters,$t2" >> "$OUT"
        printf "Mode: %-17s | Threads: %-2s | Iters: %-8s | Time: %s s\n" "Cond_Var_Barrier" "$threads" "$iters" "$t2"

        # 3. Sense-Reversal Centralized barrier (barrier_sense.c)
        out3=$(./barrier_sense $threads $iters)
        t3=$(echo "$out3" | grep -oE '[0-9]+\.[0-9]+' | tail -n 1)
        echo "3,Sense_Reversal,$threads,$iters,$t3" >> "$OUT"
        printf "Mode: %-17s | Threads: %-2s | Iters: %-8s | Time: %s s\n" "Sense_Reversal" "$threads" "$iters" "$t3"

        # 4. Combining Tree barrier (barrier_tree.c)
        out4=$(./barrier_tree $threads $iters)
        t4=$(echo "$out4" | grep -oE '[0-9]+\.[0-9]+' | tail -n 1)
        echo "4,Combining_Tree,$threads,$iters,$t4" >> "$OUT"
        printf "Mode: %-17s | Threads: %-2s | Iters: %-8s | Time: %s s\n" "Combining_Tree" "$threads" "$iters" "$t4"
    done
done

echo "Completed! Results saved to $OUT."