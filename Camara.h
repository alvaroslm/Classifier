/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion de la clase Camara basica e interfaz ICamara para capturar imagenes
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 * 
 * $Id: Camara.h 404 2008-04-06 14:47:51Z Alvaro $
 */

#pragma once

#include <stdio.h>

#pragma warning( disable : 4996 )
#include "cv.h"
#include "highgui.h"
#pragma warning( disable :  )

// Interfaz para camara generica
class ICamara
{
public:
	virtual IplImage* queryFrame(bool bSelect) = 0;
	virtual ~ICamara() { 
		// importante ; sin esto no llamaria a los destructores virtuales de las derivadas
	}
};


// Gestiona la camara y proporciona imagenes capturadas
class Camara : public ICamara
{
public:
	// constructor
	Camara(int index=CV_CAP_ANY);
	virtual ~Camara();

	// Devuelve una imagen capturada
	virtual IplImage* queryFrame(bool bSelect=true);

private:
	CvCapture* _capture;
};
