#!/bin/bash

cd prueba

touch archivo1.txt
echo "Hello, World!" > archivo2.txt

if [[ -f "archivo1.txt" && -f "archivo2.txt" ]]; then
    echo "Creación de archivos: OK"
    exit 0
else
    echo "Creación de archivos: FAIL"
    exit 1
fi