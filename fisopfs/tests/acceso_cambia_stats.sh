#!/bin/bash
VERDE="\e[0;32m"
ROJO="\e[0;31m"
RESET="\e[0m"

cd prueba

echo contenido > archivo5.txt
cat archivo5.txt > /dev/null

STAT_1=$(stat archivo5.txt)
RESULTADO=$?
sleep 1

cat archivo5.txt > /dev/null
RESULTADO=$(( $RESULTADO + $? ))

STAT_2=$(stat archivo5.txt)
RESULTADO=$(( $RESULTADO + $? ))

if [ "$STAT_1" != "$STAT_2" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Cambio de stats de archivos al acceder: $VERDE OK $RESET"
    rm archivo5.txt /dev/null 2>&1
    exit 0
else
    echo -e "Cambio de stats de archivos al acceder: $ROJO FAIL $RESET"
    rm archivo5.txt /dev/null 2>&1
    exit 1
fi
