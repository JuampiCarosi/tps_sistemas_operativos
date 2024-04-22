# shell

### Búsqueda en $PATH

**¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?**

Las funciones **execve(2)** y la familia de funciones **exec(3)** de la librería estándar de C son diferentes en cómo abordan la ejecución de nuevos programas, aunque ambas están relacionadas en su objetivo de iniciar un nuevo programa dentro de un proceso existente.

**execve(2):**

Es una syscall que ejecuta un programa especificado por un pathname.
Toma tres argumentos: el pathname del programa a ejecutar, un array de argumentos (argv[]) y un array de variables de entorno (envp[]).
La syscall ejecuta el programa indicado y reemplaza el proceso actual con el nuevo programa.
Si el pathname no está definido o no es válido, la syscall falla.

**Familia exec(3):**

Consiste en varias funciones de alto nivel (wrappers) que envuelven la syscall **execve(2)**.
Proporcionan diferentes formas de especificar el programa a ejecutar, los argumentos y el entorno.
Permiten especificar el programa usando solo el nombre del archivo (en lugar de un pathname completo), utilizando el **PATH** del entorno para buscarlo.

También ofrecen variaciones en la forma en que se pasan los argumentos, como una lista de argumentos (**execl, execlp, execle**), un array de punteros a strings terminados en NULL (**execv, execvp**), o permitiendo especificar un entorno personalizado para el nuevo programa (**execle, execvpe**).

Dependiendo de la función, puede manejar el entorno actual o uno personalizado, y manejar errores de manera diferente.

**¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?**

La llamada a **exec(3)** puede fallar por varias razones, como un error en el path al archivo, falta de permisos para ejecutar el archivo, espacio insuficiente para los argumentos o variables de entorno, o si el archivo ya está siendo utilizado por otro proceso.

Cuando **exec(3)** falla, devuelve -1 y establece la variable errno para indicar el error específico que ocurrió. En estos casos, la implementación de la shell suele imprimir el mensaje de error correspondiente según errno, interrumpir el proceso hijo actual y continuar esperando otro comando desde el usuario.

---

### Procesos en segundo plano

**Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.**

En runcmd.c, verificamos si tipo es **BACK**, lo que indica que el proceso debe ejecutarse en segundo plano. Si es el caso, el programa utiliza la función **_print_back_info_** para mostrar información sobre el proceso en segundo plano.

Luego utilizamos la función **_waitpid_** con los siguientes 3 argumentos:

- pid del proceso padre

- Una variable llamada status para almacenar el estado de salida del proceso hijo. Esto permite que el proceso principal espere a que el proceso en segundo plano termine antes de continuar.

- WNOHANG, evita que el proceso principal se bloquee mientras espera, permitiéndole seguir ejecutando otros procesos en primer o segundo plano.

Finalmente, para ejecutar el comando en segundo plano, se llama a **_exec_cmd_** con el comando apropiado.

---

### Flujo estándar

**Investigar el significado de 2>&1, explicar cómo funciona su forma general.**

La redirección **_2>&1_** es una técnica utilizada para dirigir la salida de error estándar (**stderr**) hacia el mismo destino que la salida estándar (**stdout**). Es esencialmente una forma de unificar ambas salidas para que vayan al mismo lugar.

**Mostrar qué sucede con la salida de cat out.txt en el ejemplo.**

![Ver imagen 1](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%201.png)

**Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, \***2>&1 >out.txt**\*).**

![Ver imagen 2](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%202.png)

**¿Cambió algo?**

No, se puede ver cómo se produce el mismo resultado.

**Compararlo con el comportamiento en bash(1).**

![Ver imagen 3](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%203.png)

**Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, \***2>&1 >out.txt**\*).**

![Ver imagen 4](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%204.png)

En este caso, inicialmente, el error se imprime en la consola porque el descriptor de archivo '1' aún está dirigido a la consola. Luego, se redirige la salida estándar al archivo **out.txt**, lo que resulta en la escritura del resultado del comando en dicho archivo.

---

### Tuberías múltiples

**Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe
¿Cambia en algo?
¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.**

En bash, para conocer el código de salida, se utiliza el comando **_echo $?_**, el cual mostrará el código de salida del último comando ejecutado. Si todos los comandos de un pipe se ejecutan sin errores, el código de salida será 0. Este comportamiento es consistente tanto en nuestra implementación de la shell como en bash estándar.

Aquí se ve como se devuelve 0. Y se ejecuta el tercer comando.

![Ver imagen 5](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%205.png)

En este otro ejemplo, se ve como devuelve 0. Y se ejecuta el tercer comando.

![Ver imagen 6](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%206.png)

En este otro ejemplo, Se ve como devuelve 127, ya que no se pudo ejecutar el tercer comando.

![Ver imagen 7](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%207.png)

Con nuestra implementación:

Si el primer comando falla, se muestra el 0 del último comando ejecutado.

![Ver imagen 8](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%208.png)

Si el segundo comando falla, se muestra el 0 del ultimo comando ejecutado.

![Ver imagen 9](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%209.png)

Y si el tercer comando falla tambien muestra 0 ya que al esperar a sus hijos izquierdo y derecho, el proceso padre ejecuta **_exit(0)_**. Dado que es el último proceso en ejecutarse, **echo $?** siempre devuelve 0.

---

### Variables de entorno temporarias

**¿Por qué es necesario hacerlo luego de la llamada a fork(2)?**

Es necesario hacerlo luego de la llamada **fork(2)** porque las variables de entorno temporales son parte del espacio de memoria del proceso.
Cuando se llama a **fork(2)** se crea un nuevo proceso hijo que es una copia exacta del proceso padre, incluyendo las variables de entorno temporales. Si se modifican las variables de entorno antes de llamar a **fork(2)**, estas modificaciones se verán reflejadas en ambos procesos, lo cual no es deseable ya que bajo este caso no serían temporales debido a que existen dentro del proceso de la shell.

Por lo tanto, para que sean temporales, solo se deben guardar en el proceso hijo ya que se definieron durante la ejecución de un proceso particular y se eliminan automáticamente al finalizar su ejecución, dejando el proceso padre, osea la shell, intacta.

**En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).
¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.**

En el caso de la familia de funciones exec que terminan con la letra e, se les puede pasar un arreglo de strings con las variables de entorno temporales para la ejecución de ese proceso. En el otro caso, por ejemplo utilizando un execv(3), el nuevo proceso toma las variables de entorno del proceso padre. Esta información se puede observar en el siguiente fragmento del manual de linux (man 3 exec):

```c
e - execle(), execvpe()
    The environment of the caller is specified via the argument envp.  The envp argument is an array  of  pointers
    to null-terminated strings and must be terminated by a null pointer.

    All  other  exec() functions (which do not include 'e' in the suffix) take the environment for the new process
    image from the external variable environ in the calling process.
```

La función setenv(3) por un lado modifica las variables de entorno del proceso actual, permitiendo que estas variables sean accesibles por cualquier proceso hijo que se cree. Por otro lado, si se pasan las variables de entorno como un arreglo de strings en el tercer argumento de una de las funciones de exec(3) terminadas en e, estas variables de entorno solo estas serán accesibles por el proceso donde se ejecuta la función exec(3) ignorando las variables de entorno del proceso padre. Por lo tanto, el comportamiento resultante no es el mismo en ambos casos.

Para poder replicar este comportamiento, crear un vector de strings con las variables de entorno temporales a agregar más las variables de entorno del proceso padre, y luego pasar este vector como argumento en el tercer argumento de la función exec(3) que se utilice. Los strings que representan las variables de entorno irían de la siguiente manera "variable=valor" y este mismo vector terminaría con un puntero nulo. Finalmente, ejecutamos la función exec(3) con estos parámetros.

---

### Pseudo-variables

**Investigar al menos otras tres variables mágicas estándar, y describir su propósito.
Incluir un ejemplo de su uso en bash (u otra terminal similar).**

**$0** - obtener el nombre del script actual: A pesar de que esta variable mágica sea más útil en bash scripts, en el contexto de una shell puede ser útil para saber que shell se está ejecutando ya que no es lo mismo si estamos usando bash, zsh o una custom. Un ejemplo útil podría ser un instalador de plugins de zsh, que confirme que estemos corriendo en un entorno de zsh.

![Ver imagen 10](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%2010.png)

![Ver imagen 11](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%2011.png)

**$!** - obtener el PID del último proceso ejecutando en background: Muy útil cuando queremos identificar el último proceso en background (y no queremos subir a donde lo ejecutamos) para diferentes tareas como monitorear el proceso en el administrador de tareas o matar el proceso.

![Ver imagen 12](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%2012.png)

**$\_**- listar el último argumento del comando anterior: Puede ser muy útil en casos donde el generar dinámicamente una cadena de texto para un comando y queremos utilizarla posteriormente, por ejemplo.

![Ver imagen 13](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%2013.png)

También es muy util para agilizar el uso de la terminal, simplemente creando un directorio con el nombre que queremos y luego accediendo al mismo.

![Ver imagen 14](https://github.com/fiubatps/sisop_2024a_g25/blob/md/shell/images/Imagen%2014.png)

---

### Comandos built-in

**¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)**

Para responder estas preguntas primero comenzaremos explicando que es un comando built-in. Un comando built-in es un comando que está integrado en el shell, es decir, no es un programa externo que se ejecuta en un proceso hijo separado, sino que es parte del proceso de la shell. Al ser built-in en la shell, puede ejecutarse rápidamente y de manera eficiente sin necesidad de invocar un programa externo facilitando su uso. Tenerlo como un comando integrado también significa que puede acceder directamente al estado interno de la shell, en lugar de necesitar comunicarse con un proceso externo, lo que sería más lento y requeriría más recursos. Estos son algunos de los beneficios de tener un comando como built-in en la shell.

Una vez aclarado esto podemos responder a la primera pregunta. El comando cd no se podría implementar sin ser built-in, ya que este comando cambia el directorio actual del shell, por lo tanto, es necesario que se ejecute dentro del proceso de la shell para poder ver los resultados correctamente. Caso contrario, si se ejecutase como un comando normal, dentro de otro proceso tal como explicamos anteriormente, el cambio de directorio se realizaría en ese proceso y no en el de la shell, lo que significa que el resultado no se vería reflejado en la shell.

Por otro lado, el comando pwd si se podría implementar sin ser built-in, ya que este comando simplemente imprime el directorio actual, por lo tanto, puede que se utilice este comando en situaciones por fuera de la shell. También, en este caso tampoco ocurre el problema mencionado anteriormente donde el resultado solo se vería correctamente en el proceso hijo. Además, cabe recalcar que dentro de ciertos sistemas de Unix existen implementaciones del comando externo de pwd dentro de /bin/pwd. Este caso nos sería útil si quisiéramos ejecutar pwd desde un script de bash o algún programa para mostrar el directorio actual de trabajo sin necesidad de tener una shell corriendo para poder ejecutar el comando built-in.

---

### Segundo plano avanzado

**¿Por qué es necesario el uso de señales?**

El uso de las señales es vital desde dos puntos de vista diferentes. El primero es para manejar ciertas acciones que suceden desde el sistema operativo como **SIGCHILD**, ya que no hay otra forma de manejar cuando un proceso hijo es detenido y es una acción vital para el manejo de programas con hijos.

Otro punto de vista es para interceptar acciones de usuario como **SIGKILL** (a pesar que también puede ser enviada por el sistema operativo) ya que si no tenemos este tipo de señales no hay forma que nuestro programa cierre gracefully a la hora de hacer un ctrl^c o mandar la señal de apagar un proceso.

En definitiva, es necesario que nuestro programa pueda manejar estas diferentes señales que interactúan con otros procesos o con el sistema operativo.

**Como lo implementamos?**
Para llevar a cabo el proceso avanzado en segundo plano nos aseguramos que solo los comandos que son en background tengan en pgid igual al de la shell, para lograr esto en el run_cmd luego de realizar el fork configuramos el pgid a todos los procesos que no sean back al mismo que el del pid, de esta manera solo los comandos background tienen el pgid igual al de la shell. Lo realizamos ahi ya que en la funcion exec_cmd realizamos llamados recursivos y podrian resetearse pgids en lugares donde no queremos. Luego en init_shell corremos la funcion setup_sigchild que en definitiva crea un stack alternativo en el heap y configura el signal handler, el cual simplemente lee el pid del proceso hijo con mismo pgid (con wait y WNOHANG) y si el mismo finalizo muestra el mensaje de finalizacion de forma async-signal safe.

Finalmente en run shell cuando se llama a EXIT_SHELL se corre una funcion que se encarga de liberar el stack alternativo, la misma consiste en leer el signal stack actual, luego leer el sigaction actual, resetear el sigaction a su comportamiento default y si habia un stack pointer el mismo se libera del heap. Esta funcion la corremos tambien al hacer fork(), esto lo hacemos por el caso especifico donde haya pipes, ya que en cualquier otro caso simplemente se realiza un execvp() y se limpian las signals, pero con los pipes no se realiza execvp() en todos los hijos y estos terminan sin enviar el EXIT_SHELL, leakeando el stack alternativo creado en el heap

---
