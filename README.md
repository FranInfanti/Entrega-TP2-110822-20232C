<div align="right">
<img width="32px" src="img/algo2.svg">
</div>

# TP2

## Repositorio de Francisco Infanti - 110822 - finfanti@fi.uba.ar

- Para compilar:

```bash
make main
```

- Para ejecutar:

```bash
./main
```

- Para ejecutar con valgrind:
```bash
make
```
---
##  Funcionamiento
El programa consiste en un juego de pokemones. Para poder crear este juego se implementaron dos **TDAs** principales y se utilizaron otros secundarios para facilitar la implementacion de los dos **TDAs** principales.

- Uno de los **TDA** que se implemento es `juego.h`. Este lo que hace es llevar toda la logica del juego. Es decir, se encarga de almacenar toda la informacion necesaria para poder jugar.  
- Y el otro **TDA** es `adversario.h`. Como este juego es para dos jugadores, se tuvo que implementar la logica de un 2do jugador. Esta `ia` lo que hace es llevar a cabo las operaciones necesarias para que pueda llevarse a cabo el juego. Por ejemplo, esta `ia` realiza jugadas y tambien selecciona pokemones.

Ahora pasare a explicar con detalle como fueron implementados ambos **TDAs**.

#### Juego.h
Este **TDA** provee funciones las cuales permiten poder llevar a cabo el juego. 

Para poder implementar la logica del juego, se definieron dos estructuras privadas y se usaron diferentes **TDAs** que ire mencionado. 
- `struct juego` es la estructura principal. En esta se almacena la ronda en la que se encuentran los jugadores, la informacion de un archivo de pokemones y la informacion de los dos jugadores que esta almacenada en la siguiente estructura creada.
- `struct jugador`, esta estructura almacena los puntos del jugador, los pokemones que este tiene y los ataques que tiene disponible.

Como sabemos, el usuario para poder empezar el juego necesita proporcionar un archivo con pokemones y tres ataques. El juego necesita conocer esta informacion, por lo tanto por eso se hace uso del **TDA** `pokemon.h`. Este **TDA** nos permite leer dicho archivo y guardar la informacion que este contiene en una estructura conveniente y nos devuelve un puntero a esta informacion, `informacion_pokemon_t`. 
Ahora, como este **TDA** no provee funciones las cuales me permiten tener un acceso rapido y comodo a la informacion, entonces lo que hago es hacer uso del **TDA** `lista.h`, donde voy a guardarme toda la informacion de los pokemones, con sus respectivos ataques, en una `lista`.

Toda esta logica se lleva acabo en la funcion `juego_cargar_pokemon`. Si analizamos la complejidad de esta operacion, podemos ver que es $O(n^2)$. Pues la funcion que se utiliza para poder leer el archivo tiene una complejidad $O(n^2)$, porque ademas de leer el archivo lo ordena utilizando bubble sort. Tambien vamos a tener que recorrer cada uno de los pokemones e ir insertandolos al final de una `lista`, entonces si tenemos $n$ pokemones y con cada uno lo insertamos al final de una `lista`, que tiene complejidad $O(1)$, todo esto termina teniendo complejidad lineal $O(n)$. Luego $T(n) = O(n²)+O(n)$ y para **Big-O** es $O(n²)$.

---
<div align="center">
<img width="70%" src="">
<div>Representacion de como se veria en memoria luego de haber cargado la informacion</div>
</div>

---
Una vez se haya cargado la informacion de los pokemones, los jugadores ingresan el nombre de tres pokemones de la `lista`. Primero se verifica que alguno de los tres pokemones ingresados no se repitan entre si y por ultimo se verifica que realmente existan esos pokemones. 

En caso de que todo sea valido se procede buscando los pokemones en la `lista` e insertandolos en el `hash` correspondiente al jugador utilizando como clave el nombre del pokemon. A su vez se insertan los tres ataques de dicho pokemon en un `abb` usando como comparador los nombres de los ataques.
Ahora como en este juego los dos primeros pokemones son para el que los ingresa y el tercero es para tu adversario, entonces al estar insertando los pokemones en el `hash` y los ataques en el `abb`, se tiene esto en cuenta y se inserta todo en su correspondiente lugar.

La explicacion de por que utilizo un `hash` y un `abb` tendra mas sentido cuando este explicando el proceso de las jugadas.

Toda esta logica mencionada anteriormente se lleva a cabo en la funcion `juego_seleccionar_pokemon`. La complejidad que tiene toda esta operacion, desde validar los nombres hasta insertarlos en el `hash` y el `abb`, es $O(n)$. Porque, para validar que los nombres no sean repetidos se compara los tres nombres entre si, en este caso al ser siempre tres pokemones, la complejidad es consante $O(1)$. Ahora, para validar que esos pokemones existan, se tiene que recorrer una lista lo cual en el peor de los casos la complejidad es $O(n)$. Insertar en un `hash` tiene complejidad $O(1)$ y como estamos insertando dos pokemones en un `hash` y uno en otro `hash`, al final estamos haciendo tres operaciones de insercion lo cual sigue siendo constante. Por ultimo, insertar en un abb, suponiendo que se mantiene balanceado, tiene complejidad $O(log(n))$. En este caso vamos a insertar seis ataques en un `abb` y tres en el otro, pero al fin y al cabo termina teniendo una complejidad logartimica, pues al insertar una cantidad constante, no aporta el tamaño del problema.

Por lo tanto luego de todo esto podemos concluir que $T(n) = O(1) + O(n) + O(1) + O(log(n))$ y esto para **Big-O** es $O(n)$.

---
<div align="center">
<img width="70%" src="">
<div>Representacion de como se veria insertado un pokemon con sus ataques en el hash y el abb. No estan dibujados los punteros NULL</div>
</div>

---
Una vez seleccionados los pokemones se procede planteando una jugada, seleccionando un pokemon junto a su ataque. Lo primero que se debe hacer es validar que la jugada. Entonces lo que se hace es verificar que el pokemon seleccionado sea uno de los que tenga el jugador almacenado en su `hash` y que el ataque exista en el `abb`. Si se cumplen ambas condiciones entonces la jugada es valida y se procede a determinar la efecitvidad del ataque contra el pokemon del adversario, el puntaje final del ataque y por ultimo se elimina el ataque del `abb` para hacer referencia a que no se puede volver a usar. Todo esto se repite dos veces, pues se reciben dos jugadas de dos jugadors.

Esta logica se lleva a cabo en la funcion `juego_jugar_turno`. Veamos que la complejidad de esta operacion es $O(log(n))$ y esto se debe gracias a la eleccion del `hash` y el `abb`. Pues veamos que para validar la jugada tenemos que verificar si en el `hash` existe un elemento con la clave del nombre del pokemon seleccionado y esto es una operacion $O(1)$. Luego debemos verificar si el ataque esta en el `abb`, suponiendo que esta balanceado, podemos ver que la complejidad es $O(log(n))$, pues lo que hariamos seria una busqueda en un `abb`. Entonces todo el proceso de validacion tiene complejidad logaritmica y despues determinar la efectividad del ataque y los puntos tienen complejidad constante $O(1)$, en el caso de la efectividad aunque estemos recorrendo con un for, siempre recorremos la misma cantidad de elementos. Y por ultimo lo que debemos hacer es eliminar un elemento de un `abb` y eso tiene complejidad $O(log(n))$. 

Por lo tanto $T(n) = O(1) + O(log(n)) + O(1) + O(1) + O(log(n))$ y esto para **Big-O** es O(log(n)).