/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion de la interfaz IShapeDesc
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007. All rights reserved.
 *
 * $Id: IShapeDesc.h 22 2007-12-13 23:41:39Z Alvaro $
 */

#pragma once

#include <vector>
using namespace std;

class IShapeDesc
{
public:
	// Vector que describe la forma
	virtual vector<double> getVector() const=0;
	
	// Numero de dimensiones
	virtual int getDim() const=0;

	// Imagen interna, si la hubiera
	virtual const IplImage* getImg() const;
};

// en objeto.cpp:
//virtual const IplImage* IShapeDesc::getImg() const { return 0; }
