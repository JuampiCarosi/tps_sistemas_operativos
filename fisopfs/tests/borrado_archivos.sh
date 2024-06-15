#!/bin/bash

cd prueba

touch archivo3.txt
rm archivo3.txt

if [[ ! -f "archivo3.txt" ]]; then
    echo "Borrado de archivos: OK"
    exit 0
else
    echo "Borrado de archivos: FAIL"
    exit 1
fi