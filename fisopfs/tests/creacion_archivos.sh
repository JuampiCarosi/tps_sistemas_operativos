#!/bin/bash

VERDE="\e[0;32m"
ROJO="\e[0;31m"
RESET="\e[0m"

cd prueba

touch archivo1.txt 2> /dev/null
RESULTADO=$?
echo "hola mundo" > archivo2.txt 2> /dev/null
RESULTADO=$(( $RESULTADO + $? ))

if [ -f "archivo1.txt" ] && [ -f "archivo2.txt" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Creación de archivos: $VERDE OK $RESET"
    exit 0
else
    echo -e "Creación de archivos: $ROJO FAIL $RESET"
    exit 1
fi