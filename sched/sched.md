# sched

## Context switch

### De modo kernel a modo usuario

Primero pondremos un breakpoint en context_switch.
![Ver imagen 1](https://github.com/fiubatps/sisop_2024a_g25/blob/unoptimized_search/sched/images/parte_1_imagen_1.png)

Y avanzamos la ejecución hasta ese punto.
![Ver imagen 2](https://github.com/fiubatps/sisop_2024a_g25/blob/unoptimized_search/sched/images/parte_1_imagen_2.png)

Vemos el estado del stack.
![Ver imagen 3](https://github.com/fiubatps/sisop_2024a_g25/blob/unoptimized_search/sched/images/parte_1_imagen_3.png)

Avanzamos un paso y volvemos a ver el estado del stack.
![Ver imagen 4](https://github.com/fiubatps/sisop_2024a_g25/blob/unoptimized_search/sched/images/parte_1_imagen_4.png)

Avanzamos un paso y volvemos a ver el estado del stack.
![Ver imagen 5](https://github.com/fiubatps/sisop_2024a_g25/blob/unoptimized_search/sched/images/parte_1_imagen_5.png)

Avanzamos un paso y volvemos a ver el estado del stack.
![Ver imagen 6](https://github.com/fiubatps/sisop_2024a_g25/blob/unoptimized_search/sched/images/parte_1_imagen_6.png)

Avanzamos un paso y volvemos a ver el estado del stack.
![Ver imagen 7](https://github.com/fiubatps/sisop_2024a_g25/blob/unoptimized_search/sched/images/parte_1_imagen_7.png)

Avanzamos un paso y volvemos a ver el estado del stack.
![Ver imagen 8](https://github.com/fiubatps/sisop_2024a_g25/blob/unoptimized_search/sched/images/parte_1_imagen_8.png)

Visualizamos los registros antes de la llamada a iret
![Ver imagen 9](https://github.com/fiubatps/sisop_2024a_g25/blob/unoptimized_search/sched/images/parte_1_imagen_9.png)

Y finalmente podemos visualizar como se cambian los registros luego de ejecutar iret
![Ver imagen 10](https://github.com/fiubatps/sisop_2024a_g25/blob/unoptimized_search/sched/images/parte_1_imagen_10.png)

## Scheduler con prioridades

Para el caso de un scheduler con prioridades decidimos implementar una version de MLFQ. Para ello se creo una estructura con 4 colas de prioridades, donde guardamos el principio y fin de cada cola para poder realizar operaciones de encolar y desencolar de manera eficiente

```c
struct MLFQ_queue {
	envid_t envs[NENV];
	int last;
	int beginning;
};
struct MLFQ_sched {
	struct MLFQ_queue q0;
	struct MLFQ_queue q1;
	struct MLFQ_queue q2;
	struct MLFQ_queue q3;
	int total_executions;
};
```

De esta manera deseconlar el primer proceso es simplemente sumar 2 al beginning y si se da el caso que que queremos usar un proceso en el medio, se intercambia por el primer proceso y se suma 1 a beginning.

Para evitar el gaming de procesos es siempre penalizarlos en cada ejecución, de esta manera si un proceso se ejecuta muchas veces en la cola de mayor prioridad, eventualmente sera movido a la cola de menor prioridad. Luego de una cantidad de ejecuciones (determinada por `MAX_MLFQ_EXECUTIONS`) se realiza un boosting de todos los procesos nuevamente a la cola de mayor prioridad.

Volviendo a la implementacion nuestro scheduler, cuando el mismo es llamado busca la cola de prioridades menor que tenga procesos en estado `RUNNABLE`, si no hay ninguna intenta seguir con el proceso actual o se llama a sched_halt. De caso contrario se busca en la cola de prioridades el primer proceso en estado `RUNNABLE`, el proceso es removido de la cola actual e insertado en la cola de proxima prioridad. Si el proceso ya estaba en la cola de mayor prioridad, se lo deja en la misma, luego se llama a env_run para ejecutar el proceso.
