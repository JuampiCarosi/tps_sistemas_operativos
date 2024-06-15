#!/bin/bash

cd prueba

mkdir directorio1

if [[ -d "directorio1" ]]; then
    echo "Creación de directorios: OK"
    exit 0
else
    echo "Creación de directorios: FAIL"
    exit 1
fi