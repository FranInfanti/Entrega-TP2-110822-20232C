#include "lista.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct nodo {
	void *elemento;
	struct nodo *siguiente;
} nodo_t;

struct lista {
	nodo_t *nodo_inicio;
	nodo_t *nodo_fin;
	size_t cantidad;
};

struct lista_iterador {
	struct lista *lista;
	nodo_t *actual;
};

/*
 * Crea un nodo con todos sus campos inicializados en 0. Y luego 
 * le asigna el valor de elemento. 
 */
struct nodo *crear_nodo_lista(void *elemento)
{
	struct nodo *nuevo = calloc(1, sizeof(nodo_t));
	if (nuevo == NULL)
		return NULL;
	nuevo->elemento = elemento;
	return nuevo;
}

/*
 * Recorre los nodos hasta la posicion indicada y devuelve el nodo de dicha posicion.
 */
struct nodo *recorrer_hasta_posicion(struct lista *lista, size_t posicion)
{
	struct nodo *actual = lista->nodo_inicio;
	int i = 0;
	while (i != posicion) {
		actual = actual->siguiente;
		i++;
	}
	return actual;
}

/*
 * Libera toda la memoria ocupada por los nodos. Ademas se aplica la funcion (f)
 * en caso de no ser NULL.
 */
void destruir_nodos(nodo_t *eliminar, size_t cantidad, void (*f)(void *))
{
	size_t n = 0;
	void *aux = NULL;
	while (n < cantidad) {
		if (f != NULL)
			f(eliminar->elemento);
		aux = eliminar->siguiente;
		free(eliminar);
		eliminar = aux;
		n++;
	}
}

lista_t *lista_crear()
{
	return calloc(1, sizeof(struct lista));
}

lista_t *lista_insertar(lista_t *lista, void *elemento)
{
	if (lista == NULL)
		return NULL;

	struct nodo *nuevo = crear_nodo_lista(elemento);
	if (nuevo == NULL)
		return NULL;

	if (lista->cantidad == 0)
		lista->nodo_inicio = nuevo;
	else
		lista->nodo_fin->siguiente = nuevo;

	lista->nodo_fin = nuevo;
	lista->cantidad++;
	return lista;
}

lista_t *lista_insertar_en_posicion(lista_t *lista, void *elemento,
				    size_t posicion)
{
	if (lista == NULL)
		return NULL;
	if (posicion >= lista->cantidad)
		return lista_insertar(lista, elemento);

	struct nodo *nuevo = crear_nodo_lista(elemento);
	if (nuevo == NULL)
		return NULL;

	struct nodo *anterior = NULL;
	if (posicion == 0) {
		nuevo->siguiente = lista->nodo_inicio;
		lista->nodo_inicio = nuevo;
	} else {
		anterior = recorrer_hasta_posicion(lista, posicion - 1);
		nuevo->siguiente = anterior->siguiente;
		anterior->siguiente = nuevo;
	}
	lista->cantidad++;

	return lista;
}

void *lista_quitar(lista_t *lista)
{
	if (lista == NULL)
		return NULL;
	if (lista->cantidad == 0)
		return NULL;

	struct nodo *removido = lista->nodo_fin;
	void *elemento_removido = removido->elemento;
	free(removido);
	lista->cantidad--;

	if (lista->cantidad == 0) {
		lista->nodo_inicio = NULL;
		lista->nodo_fin = NULL;
	} else {
		struct nodo *ultimo =
			recorrer_hasta_posicion(lista, lista->cantidad - 1);
		ultimo->siguiente = NULL;
		lista->nodo_fin = ultimo;
	}

	return elemento_removido;
}

void *lista_quitar_de_posicion(lista_t *lista, size_t posicion)
{
	if (lista == NULL)
		return NULL;
	if (lista->cantidad == 0)
		return NULL;
	if (posicion >= lista->cantidad - 1)
		return lista_quitar(lista);

	struct nodo *removido = NULL;
	if (posicion == 0) {
		removido = lista->nodo_inicio;
		lista->nodo_inicio = removido->siguiente;
	} else {
		struct nodo *anterior =
			recorrer_hasta_posicion(lista, posicion - 1);
		removido = anterior->siguiente;
		anterior->siguiente = removido->siguiente;
	}

	void *elemento_removido = removido->elemento;
	free(removido);
	lista->cantidad--;

	return elemento_removido;
}

void *lista_elemento_en_posicion(lista_t *lista, size_t posicion)
{
	if (lista == NULL)
		return NULL;
	if (lista->cantidad <= posicion)
		return NULL;
	if (lista->cantidad == 0)
		return NULL;
	if (posicion == lista->cantidad - 1)
		return lista->nodo_fin->elemento;

	struct nodo *buscado = recorrer_hasta_posicion(lista, posicion);
	return buscado->elemento;
}

void *lista_buscar_elemento(lista_t *lista, int (*comparador)(void *, void *),
			    void *contexto)
{
	if (lista == NULL || comparador == NULL)
		return NULL;

	struct nodo *buscado = lista->nodo_inicio;
	size_t i = 0;
	while ((i < lista->cantidad) &&
	       comparador(buscado->elemento, contexto)) {
		buscado = buscado->siguiente;
		i++;
	}
	return i < lista->cantidad ? buscado->elemento : NULL;
}

void *lista_primero(lista_t *lista)
{
	if (lista == NULL || lista->cantidad == 0)
		return NULL;
	return lista->nodo_inicio->elemento;
}

void *lista_ultimo(lista_t *lista)
{
	if (lista == NULL || lista->cantidad == 0)
		return NULL;
	return lista->nodo_fin->elemento;
}

bool lista_vacia(lista_t *lista)
{
	if (lista == NULL)
		return true;
	return lista->cantidad == 0;
}

size_t lista_tamanio(lista_t *lista)
{
	return lista != NULL ? lista->cantidad : 0;
}

void lista_destruir(lista_t *lista)
{
	if (lista == NULL)
		return;
	destruir_nodos(lista->nodo_inicio, lista->cantidad, NULL);
	free(lista);
}

void lista_destruir_todo(lista_t *lista, void (*funcion)(void *))
{
	if (lista == NULL)
		return;
	destruir_nodos(lista->nodo_inicio, lista->cantidad, funcion);
	free(lista);
}

lista_iterador_t *lista_iterador_crear(lista_t *lista)
{
	if (lista == NULL)
		return NULL;

	struct lista_iterador *iterador = malloc(sizeof(struct lista_iterador));
	if (iterador == NULL)
		return NULL;
	iterador->lista = lista;
	iterador->actual = lista->nodo_inicio;
	return iterador;
}

bool lista_iterador_tiene_siguiente(lista_iterador_t *iterador)
{
	if (iterador == NULL)
		return false;
	if (iterador->actual == NULL)
		return false;

	return true;
}

bool lista_iterador_avanzar(lista_iterador_t *iterador)
{
	if (iterador == NULL || iterador->lista->cantidad == 0)
		return false;
	if (iterador->actual == NULL)
		return false;
	iterador->actual = iterador->actual->siguiente;
	return iterador->actual != NULL ? true : false;
}

void *lista_iterador_elemento_actual(lista_iterador_t *iterador)
{
	if (iterador == NULL)
		return NULL;
	if (iterador->actual == NULL)
		return NULL;
	return iterador->actual->elemento;
}

void lista_iterador_destruir(lista_iterador_t *iterador)
{
	free(iterador);
}

size_t lista_con_cada_elemento(lista_t *lista, bool (*funcion)(void *, void *),
			       void *contexto)
{
	if (lista == NULL || funcion == NULL)
		return 0;
	size_t elementos_aplicados = 0;
	bool sigo_aplicando = true;
	struct nodo *actual = lista->nodo_inicio;

	while (elementos_aplicados < lista->cantidad && sigo_aplicando) {
		if (!funcion(actual->elemento, contexto))
			sigo_aplicando = false;
		actual = actual->siguiente;
		elementos_aplicados++;
	}
	return elementos_aplicados;
}
