#!/usr/bin/env bash
set -e

EXEC="./bank"
OUT="results.csv"

if [ ! -x "$EXEC" ]; then
    echo "Executable $EXEC not found. Please run 'make' first."
    exit 1
fi

echo "experiment,accounts,transactions,inquiry_pct,lock_mode,lock_name,threads,time_sec,checksum,total_inquiry_sum" > "$OUT"

declare -A MODE_NAMES
MODE_NAMES[0]="Coarse_Mutex"
MODE_NAMES[1]="Coarse_RWLock"
MODE_NAMES[2]="Fine_Mutex"
MODE_NAMES[3]="Fine_RWLock"

THREADS=4

echo "Starting benchmarks (Threads = $THREADS)..."

# 1. Varying number of accounts (fixed: 50000 tx/thread, 50% inquiries, 4 threads)
for acc in 16 128 1024 8192; do
    for mode in 0 1 2 3; do
        output=$($EXEC $acc 50000 50 $mode $THREADS)
        time_sec=$(echo "$output" | awk -F'Time elapsed: ' '{print $2}' | awk '{print $1}')
        checksum=$(echo "$output" | awk -F'Checksum: ' '{print $2}' | awk '{print $1}')
        inq_sum=$(echo "$output" | awk -F'Total inquiry sum: ' '{print $2}' | awk '{print $1}')
        echo "accounts,$acc,50000,50,$mode,${MODE_NAMES[$mode]},$THREADS,$time_sec,$checksum,$inq_sum" >> "$OUT"
        printf "[Accounts=%-4s] Mode: %-13s | Time: %s s\n" "$acc" "${MODE_NAMES[$mode]}" "$time_sec"
    done
done

# 2. Varying transactions per thread (fixed: 1024 accounts, 50% inquiries, 4 threads)
for tx in 10000 50000 100000; do
    for mode in 0 1 2 3; do
        output=$($EXEC 1024 $tx 50 $mode $THREADS)
        time_sec=$(echo "$output" | awk -F'Time elapsed: ' '{print $2}' | awk '{print $1}')
        checksum=$(echo "$output" | awk -F'Checksum: ' '{print $2}' | awk '{print $1}')
        inq_sum=$(echo "$output" | awk -F'Total inquiry sum: ' '{print $2}' | awk '{print $1}')
        echo "transactions,1024,$tx,50,$mode,${MODE_NAMES[$mode]},$THREADS,$time_sec,$checksum,$inq_sum" >> "$OUT"
        printf "[Tx=%-6s]      Mode: %-13s | Time: %s s\n" "$tx" "${MODE_NAMES[$mode]}" "$time_sec"
    done
done

# 3. Varying inquiry percentage (fixed: 1024 accounts, 50000 tx/thread, 4 threads)
for pct in 0 25 50 75 95; do
    for mode in 0 1 2 3; do
        output=$($EXEC 1024 50000 $pct $mode $THREADS)
        time_sec=$(echo "$output" | awk -F'Time elapsed: ' '{print $2}' | awk '{print $1}')
        checksum=$(echo "$output" | awk -F'Checksum: ' '{print $2}' | awk '{print $1}')
        inq_sum=$(echo "$output" | awk -F'Total inquiry sum: ' '{print $2}' | awk '{print $1}')
        echo "inquiry_pct,1024,50000,$pct,$mode,${MODE_NAMES[$mode]},$THREADS,$time_sec,$checksum,$inq_sum" >> "$OUT"
        printf "[Inquiry=%-3s%%]  Mode: %-13s | Time: %s s\n" "$pct" "${MODE_NAMES[$mode]}" "$time_sec"
    done
done

echo "Completed! Results saved to $OUT."