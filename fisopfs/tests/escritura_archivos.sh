#!/bin/bash

echo "Nuevo contenido" > archivo2.txt
echo "Contenido adicional" >> archivo2.txt

if grep -q "Nuevo contenido" archivo2.txt && grep -q "Contenido adicional" archivo2.txt; then
    echo "Escritura de archivos: OK"
    exit 0
else
    echo "Escritura de archivos: FAIL"
    exit 1
fi