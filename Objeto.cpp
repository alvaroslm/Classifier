/*
 * Modulo: Vision artificial
 *
 * Descripcion: Implementacion de la clase Objeto
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007. All rights reserved.
 *
 * $Id: Objeto.cpp 400 2008-04-05 23:54:09Z Alvaro $
 */

#include "stdafx.h"
#include "Objeto.h"
#include "IShapeDesc.h"

Objeto::Objeto(IplImage* img, 
			   IplImage* imgbin,
			   const char* name,
			   bool dibujarEjes) :	_img(img, true), // ahora el objeto se encarga de la imagen
									_imgbin(imgbin, true), // ahora el objeto se encarga de la imagen
									_name(name),
									_momentos((const IplImage*)imgbin),
									_fourierDescriptor((const IplImage*)imgbin, _momentos, true)

{
	_img *= _imgbin;

	if (dibujarEjes)
		_momentos.dibujarEjes(_img);

	// por el momento lo hacemos asi, ya se calculara de otra manera
	//_ancho = _momentos.semiminor_axis();
	//_largo = _momentos.semimajor_axis();
	//printf("XXXXXXXXXX CON ELIPSE: ancho=%.2f, largo=%.2f XXXXXXXXXX\n", (float)_ancho, (float)_largo);

	// Nuevo metodo, traza lineas y busca el borde
	_momentos.calcular_ancho_largo(_imgbin, &_ancho, &_largo);

	//printf("XXXXXXXXXX CON BORDES: ancho=%.2f, largo=%.2f XXXXXXXXXX\n", (float)_ancho, (float)_largo);
}

Objeto::~Objeto()
{
	assert(_img!=0 && _imgbin!=0 && _name!="");
}

// funcion por defecto de IShapeDesc:getImg
const IplImage* IShapeDesc::getImg() const
{
	return NULL;
}


void Objeto::display(int x, int y) const
{
	cvNamedWindow(_name.c_str(), 1 );
	
	if (x>0 && y>0)
		cvMoveWindow(_name.c_str(), x, y);

	cvShowImage(_name.c_str(), _img );
}

void Objeto::display(HDC dc, int width, int height, int xoffs, int yoffs) const
{
	_img.paint(dc, xoffs, yoffs, (width<=0)?_img->width:width, (height<=0)?_img->height:height, 0, 0);
}