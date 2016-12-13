/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion de la clase FourierShapeDesc
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 *
 * $Id: FourierShapeDesc.h 400 2008-04-05 23:54:09Z Alvaro $
 */

#pragma once

#include "Moments.h"
#include "Imagen.h"
#include "IShapeDesc.h"
#include "Matrix.h"

#include <complex>
using namespace std;


class FourierShapeDesc : public IShapeDesc
{
public:
	// Constructor
	FourierShapeDesc(const CvArr* arr, const Moments& momentos, bool binary=true);

	// Destructor
	~FourierShapeDesc();

	// Vector que describe la forma
	virtual vector<double> getVector() const;
	
	// Numero de dimensiones
	virtual int getDim() const;

	// Imagen interna
	virtual const IplImage* getImg() const;

	// Devuelve el bounding box de la forma
	virtual const BBox<int>* getBBox() const;

	static int Nsamples;

protected:
	void calc_dft();
	void calc_idft();

	//static const int Nsamples = 32;

	IplImage*		_img;
	complex<double>	*_xn; //[Nsamples];
	complex<double>	*_Xk; //[Nsamples];
	complex<double>	*_ixn; //[Nsamples];
	double			*_vector; //[Nsamples-2];
	BBox<int>		*_bbox;

private:
	FourierShapeDesc() {}
};
