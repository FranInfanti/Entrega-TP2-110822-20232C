#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "src/pokemon.h"
#include "src/ataque.h"
#include "src/juego.h"
#include "src/lista.h"
#include "src/adversario.h"
#include "src/hash.h"
#include "src/comun.h"

#define BLANCO "\x1b[37;1m"
#define VERDE "\x1b[32;1m"
#define ROJO "\x1b[31;1m"
#define AZUL "\x1b[34;1m"
#define AMARILLO "\x1b[33;1m"
#define COMUN "\x1b[0m"
#define MAGNETA "\x1b[35;1m"
#define CYAN "\x1b[36;1m"
#define NARANJA "\x1b[38;2;255;128;0m"
#define GRIS "\x1b[38;2;176;174;174m"

enum RESULTADO { ERROR, OK, SALIR, COMANDO_INVALIDO };

#define CMD_AYUDA "ayuda"
#define CMD_SALIR "salir"
#define CMD_CLEAR "clear"
#define CMD_SELECCIONAR_POKEMONES "s"
#define CMD_HACER_JUGADA "j"
#define CMD_MOSTRAR_PUNTAJE "p"
#define CMD_MOSTRAR_POKEMONES "m"
#define MAX_CARACTERES 50
#define MAX_ELECCIONES 3

struct paquete {
	juego_t *juego;
	adversario_t *ia;
	lista_t *pokemones_usuario;
};

void informar_aviso(char *aviso, bool error)
{
	error ? printf(ROJO "ERROR: " COMUN) : printf(VERDE "EXITO: " COMUN);
	printf("%s\n", aviso);		
}

void mostrar_comando(char *comando, char *descripcion)
{
	printf(AMARILLO "%s: ", comando);
	printf(COMUN "%s\n", descripcion);
}

void determinar_color(enum TIPO tipo, const char *str)
{	
	if (tipo == NORMAL)
		printf(NARANJA "%s\n" COMUN, str);
	else if (tipo == FUEGO)
		printf(ROJO "%s\n" COMUN, str);
	else if (tipo == AGUA)
		printf(AZUL "%s\n" COMUN, str);
	else if (tipo == PLANTA)
		printf(VERDE "%s\n" COMUN, str);
	else if (tipo == ELECTRICO)
		printf(CYAN "%s\n" COMUN, str);
	else if (tipo == ROCA)
		printf(GRIS "%s\n" COMUN, str);	
}

void mostrar_ataques_disponibles(const struct ataque *ataque, void *aux)
{
	printf(BLANCO "Poder: ");
	printf(COMUN "%u", ataque->poder);
	printf(BLANCO "\t Ataque: ");
	determinar_color(ataque->tipo, ataque->nombre);
}

bool mostrar_pokemones_disponibles(void *pokemon, void *aux)
{
	printf(AMARILLO "Nombre: ");
	determinar_color(pokemon_tipo(pokemon), pokemon_nombre(pokemon));
	con_cada_ataque(pokemon, mostrar_ataques_disponibles, aux);
	return true;
}

void listar_pokemones(void *lista)
{
        lista_con_cada_elemento(lista, mostrar_pokemones_disponibles, NULL);
}

enum RESULTADO seleccionar_pokemones_usuario(juego_t *juego, adversario_t *ia, lista_t *pokemones_usuario)
{
	char nombre1[MAX_CARACTERES];
	char nombre2[MAX_CARACTERES];
	char nombre3[MAX_CARACTERES];
	char *nombres[] = { nombre1, nombre2, nombre3 };

	for (int i = 0; i < MAX_ELECCIONES; i++) {
		printf(MAGNETA "==TP2== ");
		printf(AMARILLO "Nombre: " COMUN);
		fscanf(stdin, "%s", nombres[i]);
	}

	JUEGO_ESTADO estado = juego_seleccionar_pokemon((juego_t *)juego, JUGADOR1, nombre1, nombre2, nombre3);

        if (estado == POKEMON_INEXISTENTE || estado == POKEMON_REPETIDO) {
		informar_aviso(estado == POKEMON_INEXISTENTE ? "Ingresa pokemones de la lista" : "No se pueden pokemones repetidos", true);
                return seleccionar_pokemones_usuario(juego, ia, pokemones_usuario);
        }
        
	if (estado == ERROR_GENERAL) {
		informar_aviso("No es tu culpa", true);
		return ERROR;
	}

	if (!adversario_pokemon_seleccionado(ia, nombre1, nombre2, nombre3))
		return ERROR;

	lista_insertar(pokemones_usuario, lista_buscar_elemento(juego_listar_pokemon(juego), comparar_nombres, nombre1));
	lista_insertar(pokemones_usuario, lista_buscar_elemento(juego_listar_pokemon(juego), comparar_nombres, nombre2));

	informar_aviso("Pokemones cargados :)", false);
	return OK;
}

enum RESULTADO seleccionar_pokemones_ia(juego_t *juego, adversario_t *ia, lista_t *pokemones_usuario)
{
	char *nombre1;
	char *nombre2;
	char *nombre3;
	if (!adversario_seleccionar_pokemon(ia, &nombre1, &nombre2, &nombre3))
		return ERROR;

	lista_insertar(pokemones_usuario, lista_buscar_elemento(juego_listar_pokemon(juego), comparar_nombres, nombre3));

	printf(AZUL "%s\n", nombre1);
	printf(AZUL "%s\n", nombre2);
	printf(AZUL "%s\n", nombre3);

	if (juego_seleccionar_pokemon(juego, JUGADOR2, nombre1, nombre2, nombre3) == ERROR_GENERAL)
		return ERROR;

	return OK;
}

jugada_t realizar_jugada_usuario(juego_t *juego)
{
	jugada_t jugada = { .ataque = "", .pokemon = "" };
	char pokemon[MAX_CARACTERES];
	char ataque[MAX_CARACTERES];

	printf("Selecciona un pokemon y el ataque que quieras usar de este\n");

	printf(AMARILLO "Pokemon: " COMUN);
	fscanf(stdin, "%s", pokemon);
	printf(AMARILLO "Ataque: " COMUN);
	fscanf(stdin, "%s", ataque);

	strcpy(jugada.pokemon, pokemon);
	strcpy(jugada.ataque, ataque);

	return jugada;
}

char *resultado_ataque(RESULTADO_ATAQUE resultado)
{
	if (resultado == ATAQUE_REGULAR)
		return AMARILLO "regular" COMUN;
	if (resultado == ATAQUE_EFECTIVO)
		return VERDE "efectivo" COMUN;
	return ROJO "inefectivo" COMUN;
}

void mostrar_resultado_ataque(resultado_jugada_t resultado, jugada_t usuario, jugada_t ia)
{
	printf("El ataque ");
	printf(BLANCO "%s " COMUN, usuario.ataque);
	printf("fue %s contra el pokemon de tu adversario\n", resultado_ataque(resultado.jugador1));
	printf("El ataque ");
	printf(BLANCO "%s " COMUN, ia.ataque);
	printf("fue %s contra tu pokemon\n", resultado_ataque(resultado.jugador2));
}



enum RESULTADO mostrar_comandos_disponibles()
{
	printf("Tenes disponibles los siguientes comandos: \n");
	mostrar_comando(CMD_AYUDA, "Muestra por pantalla los comandos disponibles");
	mostrar_comando(CMD_SALIR, "Sale del programa");
	mostrar_comando(CMD_CLEAR, "Limpia la pantalla");
	mostrar_comando(CMD_SELECCIONAR_POKEMONES, "Selecciona los pokemones para comenzar a jugar");
	mostrar_comando(CMD_HACER_JUGADA, "Selecciona el pokemon y ataque para poder realizar una jugada");
	mostrar_comando(CMD_MOSTRAR_POKEMONES, "Muestra por pantalla los pokemones disponibles");
	mostrar_comando(CMD_MOSTRAR_PUNTAJE, "Muestra el puntaje de los jugadores");
	return OK;
}

enum RESULTADO seleccionar_pokemones(juego_t *juego, adversario_t *ia, lista_t *pokemones_usuario)
{	
        printf("Tenes todos estos pokemones, selecciona tres. Los dos primeros son para vos y el tercero para tu adversario\n");
	printf("El color del pokemon y ataque corresponde al tipo de este\n");
	listar_pokemones(juego_listar_pokemon(juego));

	if (seleccionar_pokemones_usuario(juego, ia, pokemones_usuario) == ERROR)
		return ERROR;

	if (seleccionar_pokemones_ia(juego, ia, pokemones_usuario) == ERROR)
		return ERROR;

	return OK;
}

enum RESULTADO jugar_ronda(juego_t *juego, adversario_t *ia)
{
	jugada_t jugada_ia = jugada_ia = adversario_proxima_jugada(ia);
	jugada_t jugada_usuario = { .ataque = "", .pokemon = "" };
	resultado_jugada_t resultado = { .jugador1 = ATAQUE_ERROR, .jugador2 = ATAQUE_ERROR };

	while (resultado.jugador1 == ATAQUE_ERROR) {
		jugada_usuario = realizar_jugada_usuario(juego);
		resultado = juego_jugar_turno(juego, jugada_usuario, jugada_ia);
		if (resultado.jugador1 == ATAQUE_ERROR)
			informar_aviso("Fijate de estar ingresando el pokemon correcto y acordate de que no podes repetir ataques", true);
	}	

	mostrar_resultado_ataque(resultado, jugada_usuario, jugada_ia);
	return OK;
}

enum RESULTADO mostrar_puntaje(juego_t *juego)
{
	printf(AMARILLO "Usuario: " COMUN);
	printf("%i\n", juego_obtener_puntaje(juego, JUGADOR1));
	printf(AMARILLO "Ia: " COMUN);
	printf("%i\n", juego_obtener_puntaje(juego, JUGADOR2));
	return OK;
}


enum RESULTADO ejecutar_comando(char *comando, juego_t *juego, adversario_t *ia, lista_t *pokemones_usuario, bool *selecciono)
{
	if (strcmp(comando, CMD_AYUDA) == 0)
		return mostrar_comandos_disponibles();

	if (strcmp(comando, CMD_SALIR) == 0)
		return SALIR;

	if (strcmp(comando, CMD_CLEAR) == 0) {
		system(CMD_CLEAR);
		return OK;
	}
		
	if (strcmp(comando, CMD_SELECCIONAR_POKEMONES) == 0) {
		if (*selecciono) {
			informar_aviso("No podes volver a elegir pokemones", true);
			return COMANDO_INVALIDO;
		} 
		*selecciono = true;
		return seleccionar_pokemones(juego, ia, pokemones_usuario);			
	}
	
	if (strcmp(comando, CMD_HACER_JUGADA) == 0) {
		if (!*selecciono) {
			informar_aviso("Tenes que seleccionar pokemones primero", true);
			return COMANDO_INVALIDO;
		}
		return jugar_ronda(juego, ia);
	}

	if (strcmp(comando, CMD_MOSTRAR_POKEMONES) == 0) {
		if (!*selecciono)
			informar_aviso("No tenes pokemones asignados, intenta con 's'", true);
		listar_pokemones(pokemones_usuario);
		return OK;
	}
		
	if (strcmp(comando, CMD_MOSTRAR_PUNTAJE) == 0)
		return mostrar_puntaje(juego);
	
	informar_aviso(("El comando no existe, intenta con 'ayuda'"), true);
	return COMANDO_INVALIDO;
}




void liberar_todo(juego_t *juego, adversario_t *ia, lista_t *pokemones_usuario)
{
	lista_destruir(pokemones_usuario);
        adversario_destruir(ia);
        juego_destruir(juego);        
}

bool inicializar_juego(char *argv[], juego_t **juego, adversario_t **ia)
{
        *juego = juego_crear();

	JUEGO_ESTADO estado = juego_cargar_pokemon(*juego, *(argv + 1));
        if (estado != TODO_OK) {
		informar_aviso(estado == ERROR_GENERAL ? "El archivo no existe" : "La cantidad de pokemones es invalida, intenta con otro archivo", true);
                return false;
        }

	*ia = adversario_crear(juego_listar_pokemon(*juego));

	return *juego && *ia ;
}

int main(int argc, char *argv[])
{
	srand(( unsigned)time(NULL));
        juego_t *juego = juego_crear();
	if (!juego)
		return -1;
	
	JUEGO_ESTADO estado = juego_cargar_pokemon(juego, "ejemplos/correcto.txt");
        if (estado != TODO_OK) {
		informar_aviso(estado == ERROR_GENERAL ? "El archivo no existe" : "La cantidad de pokemones es invalida, intenta con otro archivo", true);
                liberar_todo(juego, NULL, NULL);
		return -1;
        }

	adversario_t *ia = adversario_crear(juego_listar_pokemon(juego));
	if (!ia) {
		liberar_todo(juego, NULL, NULL);
		return -1;
	}

	lista_t *pokemones_usuario = lista_crear();
	if (!pokemones_usuario) {
		liberar_todo(juego, ia, NULL);
		return -1;	
	}

	printf("Ingrese 'ayuda' para ver los comandos disponibles\n");
	enum RESULTADO resultado = OK;
	bool selecciono = false;

	while (!juego_finalizado(juego) && (resultado == OK || resultado == COMANDO_INVALIDO)) {
		printf(MAGNETA "==TP2== " COMUN);
		char comando[MAX_CARACTERES];
		fscanf(stdin, "%s", comando);
		resultado = ejecutar_comando(comando, juego, ia, pokemones_usuario, &selecciono);
	}

	if (resultado == OK) {
		mostrar_puntaje(juego);
		printf("Parece que %s\n", juego_obtener_puntaje(juego, JUGADOR1) > juego_obtener_puntaje(juego, JUGADOR2) ? VERDE "ganaste, felicitaciones :)" COMUN : ROJO "perdiste, mala suerte :(" COMUN);
		printf(VERDE "Gracias por jugar\n" COMUN);
	}
	
        liberar_todo(juego, ia, pokemones_usuario);
        return resultado == OK ? 0 : -1;
}