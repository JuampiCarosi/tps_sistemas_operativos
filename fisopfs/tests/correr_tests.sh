#!/bin/bash

tests=(
    "tests/creacion_archivos.sh"
    "tests/creacion_directorios.sh"
    "tests/creacion_multiples_niveles.sh"
    "tests/escritura_archivos.sh"
    "tests/borrado_archivos.sh"
    "tests/borrado_directorios.sh"
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

echo "Tests passed: $passed"
echo "Tests failed: $failed"