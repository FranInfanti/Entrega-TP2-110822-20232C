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

#### Juego.h
Este **TDA** provee funciones las cuales permiten poder llevar a cabo el juego. 

Para poder implementar la logica del juego, se definieron dos estructuras y se usaron diferentes **TDAs** que ire mencionado. 
- `struct juego` es la estructura principal. En esta se almacena la ronda en la que se encuentran los jugadores, la informacion de un archivo de pokemones y la informacion de los dos jugadores que esta almacenada en la siguiente estructura creada.
- `struct jugador`, esta estructura almacena los puntos del jugador, los pokemones que este tiene y los ataques que tiene disponible.


Como sabemos, el usuario para poder empezar el juego necesita proporcionar un archivo con pokemones y tres ataques, todo esto esta escrito segun un formato. Por lo tanto es por eso que hago uso del **TDA** `pokemon.h`, este proporciona una funcion la cual lee de un archivo segun un formato y de lo que lee lo guarda en memoria. Una vez termina de leer devuelve un puntero a esa estructura, `informacion_pokemon_t`, que contiene toda la informacion de los pokemones. La complejidad que tiene leer el archivo y guardarlo en memoria es $O(n²)$, pues se deben leer $n$ pokemones y luego se los ordena usando `Bubble Sort` que tiene complejidad cuadratica. Entonces termina siendo una suma de algo que tiene complejidad lineal y algo que tiene cuadratica, y para **Big-O** el termino cuadratico pesa mas.

El problema es que este **TDA** no tiene funciones comodas para poder acceder los datos de todos los pokemones. Entonces por este problema es que hago uso del **TDA** `lista.h`. Lo que hice fue insertar cada pokemon, con su respectia informacion, en una `lista` y de esta manera tengo un acceso mas comodo a los datos de los pokemones. La complejidad que tiene insertar los $n$ pokemones al final de la `lista` es $O(n)$, pues debemos recorrer los $n$ pokemones e insertarlos al final de la lista lo cual tiene una complejidad constante. 

Toda esta logica se lleva acabo en la funcion `juego_cargar_pokemon`. Si analizamos la complejidad total de esta operacion podemos ver que: 

$$T(n) = O(n²) + O(n)$$ 

Y como **Big-O** toma el peor caso, entonces $T(n) = O(n²)$.

---
<div align="center">
<img width="70%" src="">
<div>Representacion de como se veria en memoria luego de haber cargado la informacion</div>
</div>

---
Para poder guardar los pokemones seleccionados por el `jugador1` y el `jugador2` hago uso de un `hash` usando el nombre del pokemon como clave. Pues a mi lo unico que me interesa es saber que pokemones tienen ambos jugadores para de esta manera poder validar sus jugadas, y como el acceso a un elemento de un `hash` es casi instantaneo la complejidad seria la menor posible. Tambien otro dato seria que en el caso de que hubiese pokemones repetidos, no tengo que insertarlos dos veces, entonces consumo menos memoria. 

Pero bueno, al momento de seleccionar los pokemones, se recibe el nombre de tres pokemones. Dos para el jugador que los ingreso y el tercero para su adversario. Entonces lo que se hace es insertar los pokemones en el `hash` correspondiente. Veamos que la complejidad de esto es $O(n)$, pues insertar elementos en un `hash` tiene complejidad lineal.

A su vez, ademas de guardarme los pokemones en un `hash`, tambien me guardo todos los ataques de este en un `abb`. Esto lo hago porque cuando se ingrese un pokemon junto a su ataque (para hacer una jugada), voy a necesitar saber si ese ataque fue usado o no. Entonces si el ataque esta en el `abb` es porque no fue usado y puede ser usado. Ahora si no lo esta, es porque ya fue utilizado y debe elegir otro. Entonces en el caso de que hubiera usado un `hash` no se hubiera insertado ese ataque repetido y seria mas dificil determinar si ya fue usado o no. Ademas entre un `abb` y una `lista`, si el `abb` se mantiene balanceado, entonces tiene una mejor complejidad.

Entonces hay que agregar que a la hora se seleccionar los pokemones, debo insertar los ataques en el correspondiente `abb`. Por lo tanto la complejidad de esto, suponiendo que el arbol continua balanceado a medida que se insertan, seria $O(log(n))$.

Esta logica de seleccionar pokemones se lleva a cabo en la funcion `juego_seleccionar_pokemon`. Si analizamos la complejidad total de esta operacion tengo que añadir que antes de insertar en el `hash` y el `abb`, se valida que los nombres de los pokemones no se repitan entre si. Para lograr esto comparo los tres nombres entre si, lo cual tiene complejidad constante. Y ademas debo verificar que los tres pokemones esten en la `lista`, entonces se recorre tres veces la `lista`, lo cual tiene complejidad lineal. 

Por lo tanto luego de todo esto podemos concluir que 

$$T(n) = O(1) + O(n) + O(n) + O(log(n))$$ 

Y esto para **Big-O** es $O(n)$.

---
<div align="center">
<img width="70%" src="">
<div>Representacion de como se veria insertado un pokemon con sus ataques en el hash y el abb. No estan dibujados los punteros NULL</div>
</div>

---
Al momento de hacer una jugada se hace uso de la funcion `juego_jugar_turno`. En esta se recibira tanto la jugada del `jugador1` como la del `jugador2`, ambas deben ser validadas. Y ¿como se valida?, lo que se debe hacer es verificar que el pokemon seleccionado exista en el `hash` del jugador usando la funcion `hash_obtener`, la cual devuelve el elemento que coincida con la clave pasada o `NULL` si no existe. En el caso de que exista, entonces obtenemos el ataque usando `pokemon_buscar_ataque` y por ultimo, usando `abb_buscar` se procede a buscar el ataque seleccionado en el `abb`.

Veamos que la complejidad para determinar si una jugada es valida es $O(log(n))$. Pues lo que debemos hacer es primero verificar si el pokemon realmente lo tiene el jugador, entonces lo buscamos en el `hash` y esto tiene una complejidad constante $O(1)$. En caso de que exista, pasamos a buscar el ataque del pokemon y como solo hay tres posbiles casos, podemos decir que la complejidad es $O(3) = O(1)$. Y por ultimo lo buscamos en el `abb`, y si suponemos que esta balanceado, la complejidad es $O(log(n))$. Entonces:

$$T(n) = O(1) + O(log(n)) + O(1)$$

Y para **Big-O** esto es $T(n) = O(log(n))$

Ya habiendo validado la jugada, tenemos luz verde para pasar a determinar la efectividad del ataque, el poder/puntaje y eliminar el ataque usado del `abb`.

Para determinar la efectividad, se me ocurrio crear un array con todos los tipos. Estan acomodados de tal manera que el tipo en la posicion $x$ es efectivo contra el tipo en $x+1$ e infectivo contra $x-1$. Entonces lo primero que hago es verificar las posiciones de los tipos en el array. Y plantear la cuenta `ataque_actual` - `pokemon_adversario`, si:
- Es igual a $-1$, entonces el ataque es efectivo, pues eso significa que `pokemon_adversario` $\ge$ `ataque_actual`, es decir, el tipo del ataque esta en $x$ y el del pokemon en $x+1$, pues difieren en uno.
- Es igual a $1$, entonces el ataque es inefectivo, pues es la inversa de lo que paso antes. En este caso `pokemon_adversario` $\le$ `ataque_actual`, y como difieren en uno, significa que el tipo del ataque esta en $x$ y el del pokemon en $x-1$.
- Para cualquier otro valor el ataque es regular.

Ahora existe un problema y es cuando tocan los tipos de las puntas, en este caso como no se puede hacer un array circular, lo que hago es: 
- Si el tipo del ataque esta en la primera posicion y el tipo del pokemon en la ultima, muevo `pokemon_adversario` una posicion antes `posicion_actual`. 
- Si es al revez, muevo `pokemon_adversario` una posicion despues de `ataque_actual`.

A pesar de ser muchas cosas, la complejidad de determinar la efectividad es constante $O(1)$, pues el array siempre tiene la misma cantidad de elementos y lo otro es hacer cuentas simples que no aportan al tamaño del problema.

Para determinar el puntaje es hacer algunas cuentas simples las cuales varian dependiendo de si el ataque fue efectivo, regular o infectivo. Pero para cualquier caso la complejidad es $O(1)$.

Y por ultimo debemos quitar el ataque utilizado del `abb` usando la funcion `abb_quitar`. Suponiendo que este esta balanceado, la complejidad de esto seria $O(log(n))$

Por lo tanto, veamos que la complejidad total de la funcion `juego_jugar_turno` es la siguiente: 

$$T(n) = O(1) + O(log(n)) + O(1) + O(1) + O(log(n))$$

Y esto para **Big-O** es $O(log(n))$.

---
