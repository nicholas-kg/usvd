
#!/bin/bash

PROGRAM="./matrix_pow"
REPETITIONS=5

SIZES=(50 128 256 512 1024)
POWERS=(64 96 100)
THREADS=(4 8 16)

echo "Starting benchmarks..."

for N in "${SIZES[@]}"; do
    for P in "${POWERS[@]}"; do
        for T in "${THREADS[@]}"; do

            SEQ_TOTAL=0
            PAR_TOTAL=0

            for ((i=1; i<=REPETITIONS; i++)); do

                OUTPUT=$("$PROGRAM" "$N" "$P" "$T")
                STATUS=$?

                if [ $STATUS -ne 0 ]; then
                    echo "Program failed! N=$N P=$P T=$T"
                    exit 1
                fi

                SEQ_TIME=$(echo "$OUTPUT" | grep "Time elapsed for the non parallel implementation:" | awk '{print $(NF-1)}')
                PAR_TIME=$(echo "$OUTPUT" | grep "Time elapsed for the prallel implementation:" | awk '{print $(NF-1)}')

                if [[ -z "$SEQ_TIME" || -z "$PAR_TIME" ]]; then
                    echo "Could not extract execution times!"
                    echo "$OUTPUT"
                    exit 1
                fi

                SEQ_TOTAL=$(awk -v a="$SEQ_TOTAL" -v b="$SEQ_TIME" 'BEGIN {print a+b}')
                PAR_TOTAL=$(awk -v a="$PAR_TOTAL" -v b="$PAR_TIME" 'BEGIN {print a+b}')

            done

            SEQ_AVG=$(awk -v total="$SEQ_TOTAL" -v n="$REPETITIONS" 'BEGIN {printf "%.6f", total/n}')
            PAR_AVG=$(awk -v total="$PAR_TOTAL" -v n="$REPETITIONS" 'BEGIN {printf "%.6f", total/n}')

            echo "======================================"
            echo "N=$N | Power=$P | Threads=$T"
            echo "Average sequential time: $SEQ_AVG seconds"
            echo "Average parallel time:   $PAR_AVG seconds"
            echo "======================================"
            echo

        done
    done
done

echo "Benchmarks completed!"
