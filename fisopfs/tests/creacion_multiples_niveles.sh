#!/bin/bash
VERDE="\e[0;32m"
ROJO="\e[0;31m"
RESET="\e[0m"

cd prueba

CREATED_AT_DIR=$PWD

mkdir -p directorio1/directorio2/directorio3 2> /dev/null
cd directorio1/directorio2/directorio3 2> /dev/null

touch archivo4.txt 2> /dev/null
RESULTADO=$?

if [ -f "archivo4.txt" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Multiples niveles: $VERDE OK $RESET"
    cd $CREATED_AT_DIR
    rm -rf directorio1 > /dev/null 2>&1
    exit 0
else
    echo -e "Multiples niveles: $ROJO FAIL $RESET"
    cd $CREATED_AT_DIR
    rm -rf directorio1 > /dev/null 2>&1
    exit 1
fi
