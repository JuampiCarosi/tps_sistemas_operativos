#!/bin/bash
VERDE="\e[0;32m"
ROJO="\e[0;31m"
RESET="\e[0m"

cd prueba

echo prueba > archivo6.txt

STAT_1=$(stat archivo6.txt)
RESULTADO=$?

echo cambios > archivo6.txt
RESULTADO=$(( $RESULTADO + $? ))

STAT_2=$(stat archivo6.txt)
RESULTADO=$(( $RESULTADO + $? ))

if [ "$STAT_1" != "$STAT_2" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Cambio de stats de archivos al editar: $VERDE OK $RESET"
    rm archivo6.txt > /dev/null 2>&1
    exit 0
else
    echo -e "Cambio de stats de archivos al editar: $ROJO FAIL $RESET"
    rm archivo6.txt > /dev/null 2>&1
    exit 1
fi
