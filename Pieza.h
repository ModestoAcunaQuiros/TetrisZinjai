#ifndef PIEZA_H
#define PIEZA_H

// Las 7 piezas
enum TipoPieza {
	PIEZA_I = 0,
	PIEZA_O,
	PIEZA_T,
	PIEZA_S,
	PIEZA_Z,
	PIEZA_J,
	PIEZA_L,
	CANTIDAD_TIPOS_PIEZA 
};


struct Pieza {
	TipoPieza tipo;
	int orientacion;      
	int filaOrigen;      
	int colOrigen;
};


const int* obtenerFormaPieza(TipoPieza tipo, int orientacion);


Pieza crearPieza(TipoPieza tipo);

#endif
