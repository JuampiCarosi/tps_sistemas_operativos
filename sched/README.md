# sched

Repositorio para el esqueleto del [TP sched](https://fisop.github.io/website/tps/sched) del curso Mendez-Fresia de **Sistemas Operativos (7508) - FIUBA**

## Respuestas teóricas

Utilizar el archivo `sched.md` provisto en el repositorio

## Compilar

Por _default_ se compilará el _scheduler_ en versión **round-robin**.

```bash
make
```

## Compilación condicional de _schedulers_

Para compilar y probar el kernel y poder probar ambos planificadores, se puede:

- **round-robin**:

```bash
make <target> USE_RR=1
```

- **priorities**:

```bash
make <target> USE_PR=1
```

## Pruebas

```bash
make grade
```

## Linter

```bash
$ make format
```

Para efectivamente subir los cambios producidos por el `format`, hay que `git add .` y `git commit`.
