#!/bin/bash
VERDE="\e[0;32m"
ROJO="\e[0;31m"
RESET="\e[0m"

cd prueba

touch archivo4.txt

STAT_1=$(stat archivo4.txt)
RESULTADO=$?
STAT_2=$(stat archivo4.txt)
RESULTADO=$(( $RESULTADO + $? ))

if [ "$STAT_1" == "$STAT_2" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Persistencia de stats de archivos: $VERDE OK $RESET"
    rm archivo4.txt > /dev/null 2>&1
    exit 0
else
    echo -e "Persistencia de stats de archivos: $ROJO FAIL $RESET"
    rm archivo4.txt > /dev/null 2>&1
    exit 1
fi
