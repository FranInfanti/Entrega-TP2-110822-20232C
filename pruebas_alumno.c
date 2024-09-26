#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "src/pokemon.h"
#include "src/ataque.h"
#include "src/juego.h"
#include "src/lista.h"
#include "src/tipo.h"
#include "src/hash.h"
#include "pa2m.h"
#include "src/adversario.h"

void no_se_puede_seleccionar_pokemones_inexistentes()
{
	juego_t *juego = juego_crear();
	juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

	char *nombre1 = "Toto";
	char *nombre2 = "Pikachu";
	char *nombre3 = "Charmander";

	pa2m_afirmar(juego_seleccionar_pokemon(juego, JUGADOR1, nombre1,
					       nombre2,
					       nombre3) == POKEMON_INEXISTENTE,
		     "No se puede seleccionar pokemones que no existen");
	juego_destruir(juego);
}

void no_se_puede_seleccionar_pokemones_repetidos()
{
	juego_t *juego = juego_crear();
	juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

	char *nombre1 = "Charmander";
	char *nombre2 = "Pikachu";
	char *nombre3 = "Charmander";

	pa2m_afirmar(juego_seleccionar_pokemon(juego, JUGADOR1, nombre1,
					       nombre2,
					       nombre3) == POKEMON_REPETIDO,
		     "No se puede seleccionar pokemones repetidos entre si");
	juego_destruir(juego);
}

void se_puede_selecionnar_pokemones_validos()
{
	juego_t *juego = juego_crear();
	juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

	char *nombre1 = "Cacnea";
	char *nombre2 = "Pikachu";
	char *nombre3 = "Charmander";

	pa2m_afirmar(juego_seleccionar_pokemon(juego, JUGADOR1, nombre1,
					       nombre2, nombre3) == TODO_OK,
		     "Se puede seleccionar pokemones validos correctamente");
	juego_destruir(juego);
}

void prueba_de_ataque1()
{
	juego_t *juego = juego_crear();
	juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

	juego_seleccionar_pokemon(juego, JUGADOR1, "Pikachu", "Charmander",
				  "Cacnea");
	juego_seleccionar_pokemon(juego, JUGADOR2, "Floatzel", "Pikachu",
				  "Togepi");

	jugada_t jugador1 = { .pokemon = "Charmander",
			      .ataque = "Lanzallamas" };
	jugada_t jugador2 = { .pokemon = "Floatzel", .ataque = "Cascada" };

	resultado_jugada_t jugada =
		juego_jugar_turno(juego, jugador1, jugador2);
	pa2m_afirmar(
		jugada.jugador1 == ATAQUE_INEFECTIVO,
		"Un ataque de fuego, hecho por el jugador1, contra un pokemon de agua, del jugador2, resulta inefectivo");
	pa2m_afirmar(
		jugada.jugador2 == ATAQUE_EFECTIVO,
		"Un ataque de agua, hecho por el jugador2, contra un pokemon de fuego, del jugador1, resulta efectivo");
	pa2m_afirmar(juego_obtener_puntaje(juego, JUGADOR1) == 2 &&
			     juego_obtener_puntaje(juego, JUGADOR2) == 6,
		     "Los puntajes son correctos");
	pa2m_afirmar(juego_finalizado(juego) == false,
		     "El juego sigue en curso");

	juego_destruir(juego);
}

void prueba_de_ataque2()
{
	juego_t *juego = juego_crear();
	juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

	juego_seleccionar_pokemon(juego, JUGADOR1, "Pikachu", "Charmander",
				  "Cacnea");
	juego_seleccionar_pokemon(juego, JUGADOR2, "Floatzel", "Pikachu",
				  "Togepi");

	jugada_t jugador1 = { .pokemon = "Charmander",
			      .ataque = "Lanzallamas" };
	jugada_t jugador2 = { .pokemon = "Cacnea", .ataque = "Hojas" };

	resultado_jugada_t jugada =
		juego_jugar_turno(juego, jugador1, jugador2);
	pa2m_afirmar(
		jugada.jugador1 == ATAQUE_EFECTIVO,
		"Un ataque de fuego, hecho por el jugador1, contra un pokemon de planta, del jugador2, resulta efectivo");
	pa2m_afirmar(
		jugada.jugador2 == ATAQUE_INEFECTIVO,
		"Un ataque de planta, hecho por el jugador2, contra un pokemon de fuego, del jugador1, resulta inefectivo");
	pa2m_afirmar(juego_obtener_puntaje(juego, JUGADOR1) == 12 &&
			     juego_obtener_puntaje(juego, JUGADOR2) == 1,
		     "Los puntajes son correctos");

	juego_destruir(juego);
}

void prueba_de_ataque3()
{
	juego_t *juego = juego_crear();
	juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

	juego_seleccionar_pokemon(juego, JUGADOR1, "Cacnea", "Charmander",
				  "Larvitar");
	juego_seleccionar_pokemon(juego, JUGADOR2, "Floatzel", "Pikachu",
				  "Togepi");

	jugada_t jugador1 = { .pokemon = "Cacnea", .ataque = "Hojas" };
	jugada_t jugador2 = { .pokemon = "Larvitar", .ataque = "Terremoto" };

	resultado_jugada_t jugada =
		juego_jugar_turno(juego, jugador1, jugador2);
	pa2m_afirmar(
		jugada.jugador1 == ATAQUE_EFECTIVO,
		"Un ataque de planta, hecho por el jugador1, contra un pokemon de roca, del jugador2, resulta efectivo");
	pa2m_afirmar(
		jugada.jugador2 == ATAQUE_INEFECTIVO,
		"Un ataque de roca, hecho por el jugador2, contra un pokemon de planta, del jugador1, resulta inefectivo");
	pa2m_afirmar(juego_obtener_puntaje(juego, JUGADOR1) == 6 &&
			     juego_obtener_puntaje(juego, JUGADOR2) == 2,
		     "Los puntajes son correctos");

	juego_destruir(juego);
}

void prueba_de_ataque4()
{
	juego_t *juego = juego_crear();
	juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

	juego_seleccionar_pokemon(juego, JUGADOR1, "Pikachu", "Charmander",
				  "Larvitar");
	juego_seleccionar_pokemon(juego, JUGADOR2, "Floatzel", "Pikachu",
				  "Togepi");

	jugada_t jugador1 = { .pokemon = "Pikachu", .ataque = "Rayo" };
	jugada_t jugador2 = { .pokemon = "Larvitar", .ataque = "Terremoto" };

	resultado_jugada_t jugada =
		juego_jugar_turno(juego, jugador1, jugador2);
	pa2m_afirmar(
		jugada.jugador1 == ATAQUE_INEFECTIVO,
		"Un ataque electrico, hecho por el jugador1, contra un pokemon de roca, del jugador2, resulta inefectivo");
	pa2m_afirmar(
		jugada.jugador2 == ATAQUE_EFECTIVO,
		"Un ataque de roca, hecho por el jugador2, contra un pokemon electrico, del jugador1, resulta efectivo");
	pa2m_afirmar(juego_obtener_puntaje(juego, JUGADOR1) == 3 &&
			     juego_obtener_puntaje(juego, JUGADOR2) == 9,
		     "Los puntajes son correctos");

	juego_destruir(juego);
}

void prueba_de_ataque5()
{
	juego_t *juego = juego_crear();
	juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

	juego_seleccionar_pokemon(juego, JUGADOR1, "Pikachu", "Charmander",
				  "Larvitar");
	juego_seleccionar_pokemon(juego, JUGADOR2, "Floatzel", "Pikachu",
				  "Togepi");

	jugada_t jugador1 = { .pokemon = "Pikachu", .ataque = "Rayo" };
	jugada_t jugador2 = { .pokemon = "Floatzel", .ataque = "Cascada" };

	resultado_jugada_t jugada =
		juego_jugar_turno(juego, jugador1, jugador2);
	pa2m_afirmar(
		jugada.jugador1 == ATAQUE_EFECTIVO,
		"Un ataque electrico, hecho por el jugador1, contra un pokemon de agua, del jugador2, resulta efectivo");
	pa2m_afirmar(
		jugada.jugador2 == ATAQUE_INEFECTIVO,
		"Un ataque de agua, hecho por el jugador2, contra un pokemon electrico, del jugador1, resulta inefectivo");
	pa2m_afirmar(juego_obtener_puntaje(juego, JUGADOR1) == 15 &&
			     juego_obtener_puntaje(juego, JUGADOR2) == 1,
		     "Los puntajes son correctos");

	juego_destruir(juego);
}

int main()
{
	srand((unsigned)time(NULL));
	pa2m_nuevo_grupo("Pruebas de Seleccion");
	no_se_puede_seleccionar_pokemones_inexistentes();
	no_se_puede_seleccionar_pokemones_repetidos();
	se_puede_selecionnar_pokemones_validos();

	pa2m_nuevo_grupo("Pruebas de Ataque");
	prueba_de_ataque1();
	prueba_de_ataque2();
	prueba_de_ataque3();
	prueba_de_ataque4();
	prueba_de_ataque5();

	return pa2m_mostrar_reporte();
}