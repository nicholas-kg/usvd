
#!/bin/bash

PROGRAM="./array_stats"
REPETITIONS=5

SIZES=(1000 10000 100000 1000000 10000000)

echo "Starting benchmarks..."

for N in "${SIZES[@]}"; do

    SEQ_TOTAL=0
    PAR_TOTAL=0

    for ((i=1; i<=REPETITIONS; i++)); do

        OUTPUT=$("$PROGRAM" "$N")
        STATUS=$?

        if [ $STATUS -ne 0 ]; then
            echo "Program failed! Size=$N"
            exit 1
        fi

        SEQ_TIME=$(echo "$OUTPUT" | grep "Sequential implementation lasted:" | awk '{print $(NF-1)}')
        PAR_TIME=$(echo "$OUTPUT" | grep "Parallel implementation lasted:" | awk '{print $(NF-1)}')

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
    echo "Size=$N"
    echo "Average sequential time: $SEQ_AVG seconds"
    echo "Average parallel time:   $PAR_AVG seconds"
    echo "======================================"
    echo

    
    
done

echo "Benchmarks completed!"
