# sched

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