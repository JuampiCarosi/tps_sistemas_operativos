#!/bin/bash
VERDE="\e[0;32m"
ROJO="\e[0;31m"
RESET="\e[0m"

cd prueba

echo "Nuevo contenido" > archivo7.txt 2> /dev/null
echo "Contenido adicional" >> archivo7.txt 2> /dev/null

if grep -q "Nuevo contenido" archivo7.txt && grep -q "Contenido adicional" archivo7.txt; then
    echo -e "Escritura de archivos: $VERDE OK $RESET"
    rm archivo7.txt > /dev/null 2>&1
    exit 0
else
    echo -e "Escritura de archivos: $ROJO FAIL $RESET"
    rm archivo7.txt > /dev/null 2>&1
    exit 1
fi