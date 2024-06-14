#!/bin/bash

rm fs.fisopfs
mkdir -p prueba

./fisopfs -f prueba/ &
FISOPFS_PID=$!

sleep 1

tests=(
    "tests/creacion_archivos.sh"
)

passed=0
failed=0

for test in "${tests[@]}"; do
    ./$test
    if [[ $? -eq 0 ]]; then
        passed=$((passed + 1))
    else
        failed=$((failed + 1))
    fi
done

kill $FISOPFS_PID

echo "Tests passed: $passed"
echo "Tests failed: $failed"