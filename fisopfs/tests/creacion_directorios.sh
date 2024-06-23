#!/bin/bash
VERDE="\e[0;32m"
ROJO="\e[0;31m"
RESET="\e[0m"

cd prueba

mkdir directorio 2> /dev/null
RESULTADO=$?

if [ $RESULTADO -eq 0 ] && [ -d "directorio" ]; then
    echo -e "Creacion de directorios: $VERDE OK $RESET"
    exit 0
else
    echo -e "Creacion de directorios: $ROJO FAIL $RESET"
    exit 1
fi
