#!/bin/bash

cd prueba

mkdir -p directorio1/directorio2/directorio3
cd directorio1/directorio2/directorio3

touch archivo4.txt

if [[ -f "archivo4.txt" ]]; then
    echo "Multiples niveles: OK"
    exit 0
else
    echo "Multiples niveles: FAIL"
    exit 1
fi