# fisopfs

Repositorio para el esqueleto del [TP: filesystem](https://fisop.github.io/website/tps/filesystem) del curso Mendez-Fresia de **Sistemas Operativos (7508) - FIUBA**

> Sistema de archivos tipo FUSE.

## Respuestas teóricas

Utilizar el archivo `fisopfs.md` provisto en el repositorio

## Compilar

```bash
$ make
```

## Ejecutar

### Setup

Primero hay que crear un directorio de prueba:

```bash
$ mkdir prueba
```

### Iniciar el servidor FUSE

En el mismo directorio que se utilizó para compilar la solución, ejectuar:

```bash
$ ./fisopfs prueba/
```

### Verificar directorio

```bash
$ mount | grep fisopfs
```

### Utilizar el directorio de "pruebas"

En otra terminal, ejecutar:

```bash
$ cd prueba
$ ls -al
```

### Limpieza

```bash
$ sudo umount prueba
```

## Linter

```bash
$ make format
```

Para efectivamente subir los cambios producidos por el `format`, hay que `git add .` y `git commit`.
