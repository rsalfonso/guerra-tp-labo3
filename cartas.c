#include "cartas.h"

/**
 * Esta funcion devuelve el nombre de la carta basado en su posicion en un mazo ordenado por numero y palo.
 * */
char* obtener_nombre_de_carta(int numero_carta) {

	char palo[LONGITUD_CODIGO_PALO];
  	char valor[LONGITUD_VALOR_CARTA];
	char* carta = (char*)malloc((strlen(valor) + strlen(palo)) * sizeof(char));

	/**
	 * Con este switch determinamos el palo de la carta.
	 * Suponemos las cartas ordenadas primero por numero y luego por palo (Del 2 al As y Pica, Trebol, Diamante, Corazon).
	 * */ 
	switch (numero_carta % 4)
	{
		case 0:
			strcpy(palo, CODIGO_PALO_PICA);
			break;
		case 1:
			strcpy(palo, CODIGO_PALO_TREBOL);
			break;
		case 2:
			strcpy(palo, CODIGO_PALO_DIAMANTE);
			break;
		case 3:
			strcpy(palo, CODIGO_PALO_CORAZON);
			break;
		//No hacemos nada en el default porque ya abarcamos todos los valores posibles.
		default:
			break;
	}
	
	/**
	 * Nos valemos del orden de las cartas para determinar el valor corrspondiente segun
	 * la posicion de la misma en un mazo ordenado.
	 * */
	switch (numero_carta / 4)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			sprintf(valor, "%d", (numero_carta / 4) + 2);
			break;
		case 9:
			strcpy(valor, "C");
			break;
		case 10:
			strcpy(valor, "D");
			break;
		case 11:
			strcpy(valor, "R");
			break;
		case 12:
			strcpy(valor, "A");
			break;
		default:
			//Esto quiere decir que recibimos un valor mayor a 51, lo cual es invalido.
			exit(1);
			break;
	}

	//Concatenamos el valor de la carta con su palo
	strcpy(carta, valor);
	strcat(carta, palo);

	return carta;
}


int obtener_puntos_de_carta(int numero_carta) {
	if (numero_carta > 47) {
		return 1;
	}
	return (numero_carta / 4) + 2;
}
