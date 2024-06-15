#!/bin/bash
VERDE="\e[0;32m"
ROJO="\e[0;31m"
AMARILLO="\e[0;33m"
RESET="\e[0m"

tests=(
    "tests/creacion_archivos.sh"
    "tests/borrado_archivos.sh"
    "tests/creacion_multiples_niveles.sh"
    "tests/escritura_archivos.sh"
    "tests/creacion_directorios.sh"
    "tests/borrado_directorios.sh"
    "tests/persistencia_stats.sh"
    "tests/acceso_cambia_stats.sh"
    "tests/edicion_cambia_stats.sh"
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

echo -e "\n${AMARILLO}Tests summary\n================$RESET"
echo -e "${VERDE} Passed: $passed $RESET"
echo -e "${ROJO} Failed: $failed $RESET"
