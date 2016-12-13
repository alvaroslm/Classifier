/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion de la clase CamaraCv para capturar imagenes
 *					cvcam no funciona bien, el callback devuelve siempre null..
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2008. All rights reserved.
 *
 * $Id: CamaraCv.h 396 2008-04-05 01:56:56Z Alvaro $
 */

#pragma once

#include "stdafx.h"
#include "Camara.h"

class CamaraCv : public ICamara
{
	template<int N> friend	void	_callbackCamaras(IplImage* img);

public:
	// constructor
	CamaraCv(int index=0);

	// destructor
	virtual ~CamaraCv();

	// Devuelve una imagen capturada
	virtual IplImage* queryFrame(bool bSelect=true);

	// Saca el cuadro de dialogo de configuracion para la webcam
	static void ConfigCamara(int index);

	static const int MAX_CAMARAS = 8;

private:
	void	callback(IplImage* img);

	static CamaraCv*	Camaras[MAX_CAMARAS];
	static int			NumCamaras;

	IplImage*	m_imagen;
	void*		m_cvcam;
	int			m_ncamara;
};
