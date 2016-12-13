/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion del template Classifier
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007. All rights reserved.
 *
 * $Id: Classifier.h 22 2007-12-13 23:41:39Z Alvaro $
 */

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>
#include <map>
#include <set>

#include "Procesador.h"
#include "Moments.h"
#include "Matrix.h"

using namespace std;


// Clasificador generico
template<class T=vector<double>, class Id=int> class Classifier
{
public:
	Classifier();

	// Entrena el clasificador con el vector v que corresponde al tipo id
	virtual void train(const T& v, const Id& id)=0;
	
	// Hace los calculos que sean necesarios cuando se ha terminado de proporcionar vectores de entrenamiento
	virtual void end_train()=0;

	// Busca el tipo que corresponde a un vector v
	virtual Id find(const T& v) const = 0;

	// Guarda en un archivo el entrenamiento conseguido
	virtual bool store(const char* fname) const = 0;

	// Carga de un archivo entrenamiento previo
	virtual bool load(const char* fname) = 0;

	// Elimina el entrenamiento adquirido
	virtual void clear() = 0;

	virtual ~Classifier();

protected:
	multimap<Id, T>	_map;
};


