#!/bin/bash

cd prueba

mkdir directorio2
rmdir directorio2

if [[ ! -d "directorio2" ]]; then
    echo "Borrado de directorios: OK"
    exit 0
else
    echo "Borrado de directorios: FAIL"
    exit 1
fi