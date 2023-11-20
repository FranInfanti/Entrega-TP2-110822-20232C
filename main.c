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

#define BLANCO "\x1b[37;1m"
#define VERDE "\x1b[32;1m"
#define ROJO "\x1b[31;1m"
#define AZUL "\x1b[34;1m"
#define AMARILLO "\x1b[33;1m"
#define NORMAL "\x1b[0m"
#define MAGNETA "\x1b[35;1m"
#define CYAN "\x1b[36;1m"

enum RESULTADO { ERROR, OK, SALIR, COMANDO_INVALIDO };

#define CMD_AYUDA "ayuda"
#define CMD_SALIR "salir"
#define CMD_CLEAR "clear"
#define CMD_SELECCIONAR_POKEMONES "s"
#define CMD_HACER_JUGADA "j"
#define CMD_MOSTRAR_PUNTAJE "p"
#define MAX_CARACTERES 50
#define MAX_ELECCIONES 3

void informar_aviso(char *aviso, bool error)
{
	error ? printf(ROJO "ERROR: " NORMAL) : printf(VERDE "EXITO: " NORMAL);
	printf("%s\n", aviso);		
}

void mostrar_comando(char *comando, char *descripcion)
{
	printf(AMARILLO "%s: ", comando);
	printf(NORMAL "%s\n", descripcion);
}

enum RESULTADO mostrar_comandos_disponibles()
{
	printf("Tenes disponibles los siguientes comandos: \n");
	mostrar_comando(CMD_AYUDA, "Muestra por pantalla los comandos disponibles");
	mostrar_comando(CMD_SALIR, "Sale del programa");
	mostrar_comando(CMD_CLEAR, "Limpia la pantalla");
	mostrar_comando(CMD_SELECCIONAR_POKEMONES, "Selecciona los pokemones para comenzar a jugar");
	mostrar_comando(CMD_HACER_JUGADA, "Selecciona el pokemon y ataque para poder realizar una jugada");
	mostrar_comando(CMD_MOSTRAR_PUNTAJE, "Muestra el puntaje de los jugadores");
	return OK;
}

void mostrar_ataques_disponibles(const struct ataque *ataque, void *aux)
{
	printf(MAGNETA "Poder: ");
	printf(NORMAL "%u\t", ataque->poder);
	printf(ROJO "\t Ataque: ");
	printf(NORMAL "%s\n", ataque->nombre);
}

bool mostrar_pokemones_disponibles(void *pokemon, void *aux)
{
	printf(AMARILLO "Nombre: ");
	printf(NORMAL "%s\n", pokemon_nombre(pokemon));
	con_cada_ataque(pokemon, mostrar_ataques_disponibles, aux);
	return true;
}

void listar_pokemones(void *juego)
{
        lista_con_cada_elemento(juego_listar_pokemon((juego_t *)juego), mostrar_pokemones_disponibles, NULL);
}

enum RESULTADO seleccionar_pokemones_usuario(juego_t *juego, char *nombre1, char *nombre2, char *nombre3)
{
	char *nombres[] = { nombre1, nombre2, nombre3 };

	for (int i = 0; i < MAX_ELECCIONES; i++) {
		printf(MAGNETA "==TP2== ");
		printf(AMARILLO "Nombre: " NORMAL);
		fscanf(stdin, "%s", nombres[i]);
	}

	JUEGO_ESTADO estado = juego_seleccionar_pokemon((juego_t *)juego, JUGADOR1, nombre1, nombre2, nombre3);

        if (estado == POKEMON_INEXISTENTE || estado == POKEMON_REPETIDO) {
		informar_aviso(estado == POKEMON_INEXISTENTE ? "Ingresa pokemones de la lista" : "No se pueden pokemones repetidos", true);
                return seleccionar_pokemones_usuario(juego, nombre1, nombre2, nombre3);
        }
        
	if (estado == ERROR_GENERAL) {
		informar_aviso("No es tu culpa", true);
		return ERROR;
	}

	informar_aviso("Pokemones cargados :)", false);
	return OK;
}

enum RESULTADO seleccionar_pokemones_ia(juego_t *juego, adversario_t *ia)
{
	char *nombre1;
	char *nombre2;
	char *nombre3;
	if (!adversario_seleccionar_pokemon(ia, &nombre1, &nombre2, &nombre3))
		return ERROR;

	printf(AZUL "%s\n", nombre1);
	printf(AZUL "%s\n", nombre2);
	printf(AZUL "%s\n", nombre3);

	if (juego_seleccionar_pokemon(juego, JUGADOR2, nombre1, nombre2, nombre3) == ERROR_GENERAL)
		return ERROR;

	return OK;
}

enum RESULTADO seleccionar_pokemones(juego_t *juego, adversario_t *ia)
{	
        printf("Tenes todos estos pokemones, selecciona tres. Los dos primeros son para vos y el tercero para tu adversario.\n");
	listar_pokemones(juego);

	char nombre1[MAX_CARACTERES];
	char nombre2[MAX_CARACTERES];
	char nombre3[MAX_CARACTERES];

	if (seleccionar_pokemones_usuario(juego, nombre1, nombre2, nombre3) == ERROR)
		return ERROR;

	if (!adversario_pokemon_seleccionado(ia, nombre1, nombre2, nombre3))
		return ERROR;

	if (seleccionar_pokemones_ia(juego, ia) == ERROR)
		return ERROR;

	return OK;
}

jugada_t realizar_jugada_usuario(juego_t *juego)
{
	jugada_t jugada = { .ataque = "", .pokemon = "" };
	char pokemon[MAX_CARACTERES];
	char ataque[MAX_CARACTERES];

	printf("Selecciona un pokemon y el ataque que quieras usar de este\n");

	printf(AMARILLO "Pokemon: " NORMAL);
	fscanf(stdin, "%s", pokemon);
	printf(AMARILLO "Ataque: " NORMAL);
	fscanf(stdin, "%s", ataque);

	strcpy(jugada.pokemon, pokemon);
	strcpy(jugada.ataque, ataque);

	return jugada;
}

char *resultado_ataque(RESULTADO_ATAQUE resultado)
{
	if (resultado == ATAQUE_REGULAR)
		return AMARILLO "regular" NORMAL;
	if (resultado == ATAQUE_EFECTIVO)
		return VERDE "efectivo" NORMAL;
	return ROJO "inefectivo" NORMAL;
}

void mostrar_resultado_ataque(resultado_jugada_t resultado, jugada_t usuario, jugada_t ia)
{
	printf("El ataque %s fue %s contra el pokemon de tu adversario\n", usuario.ataque, resultado_ataque(resultado.jugador1));
	printf("El ataque %s fue %s contra tu pokemon\n", ia.ataque, resultado_ataque(resultado.jugador2));
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
	printf(AMARILLO "Usuario: " NORMAL);
	printf("%i\n", juego_obtener_puntaje(juego, JUGADOR1));
	printf(AMARILLO "Ia: " NORMAL);
	printf("%i\n", juego_obtener_puntaje(juego, JUGADOR2));
	return OK;
}

enum RESULTADO ejecutar_comando(char *comando, juego_t *juego, adversario_t *ia, bool *selecciono)
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
		return seleccionar_pokemones(juego, ia);			
	}
	
	if (strcmp(comando, CMD_HACER_JUGADA) == 0) {
		if (!*selecciono) {
			informar_aviso("Tenes que seleccionar pokemones primero", true);
			return COMANDO_INVALIDO;
		}
		return jugar_ronda(juego, ia);
	}
		
	if (strcmp(comando, CMD_MOSTRAR_PUNTAJE) == 0)
		return mostrar_puntaje(juego);
	
	informar_aviso(("El comando no existe, intenta con 'ayuda'"), true);
	return COMANDO_INVALIDO;
}

void liberar_todo(juego_t *juego, adversario_t *ia)
{
        adversario_destruir(ia);
        juego_destruir(juego);        
}

bool inicializar_juego(char **argv, juego_t **juego, adversario_t **ia)
{
        *juego = juego_crear();

	JUEGO_ESTADO estado = juego_cargar_pokemon(juego, *(argv + 1));
        if (estado != TODO_OK) {
		informar_aviso(estado == ERROR_GENERAL ? "El archivo no existe" : "La cantidad de pokemones es invalida, intenta con otro archivo", true);
                return false;
        }

	*ia = adversario_crear(juego_listar_pokemon(juego));

	return *juego && *ia ;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		informar_aviso("Ingrese el nombre del archivo de pokemones :)", true);
        	return -1;
      	}

        juego_t *juego;
	adversario_t *ia;
	
	if (inicializar_juego(argv, &juego, &ia)) {
		liberar_todo(juego, ia);
		return -1;
	}

	printf("Ingrese 'ayuda' para ver los comandos disponibles\n");
	enum RESULTADO resultado = OK;
	bool selecciono = false;

	while (!juego_finalizado(juego) && (resultado == TODO_OK || resultado == COMANDO_INVALIDO)) {
		printf(MAGNETA "==TP2== " NORMAL);
		char comando[MAX_CARACTERES];
		fscanf(stdin, "%s", comando);
		resultado = ejecutar_comando(comando, juego, ia, &selecciono);
	}

	if (resultado == OK) {
		mostrar_puntaje(juego);
		printf("Parece que %s\n", juego_obtener_puntaje(juego, JUGADOR1) > juego_obtener_puntaje(juego, JUGADOR2) ? "Ganaste, felicitaciones" : "Perdiste, mala suerte");
		printf(VERDE "Gracias por jugar\n" NORMAL);
	}
	
        liberar_todo(juego, ia);
        return resultado == OK ? 0 : -1;
}