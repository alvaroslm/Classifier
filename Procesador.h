/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion de la clase Procesador
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 *
 * $Id: Procesador.h 403 2008-04-06 14:46:23Z Alvaro $
 */

#pragma once


#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>

using namespace std;

#pragma warning( disable : 4996 )
#include "cv.h"
#include "cxcore.h"
#include "cvaux.h"
#include "highgui.h"
#pragma warning( disable :  )

class Moments;
class Objeto;
template<class T, class Id> class Classifier;


// Procesa y contiene varias etapas del procesado de una imagen adquirida y puede mostrarlas.
class Procesador
{
public:
	struct Config;
	static Procesador* Cargar(const char* imgname, const Procesador::Config& cfg, bool bSegmentar=true);
	Procesador(const IplImage* imagen, const Procesador::Config& cfg, bool bSegmentar=true, Classifier<vector<double>, int>* pClas=NULL);

	// Destructor
	~Procesador();

	static const int Width = 320;
	static const int Height = 240;

	// Muestra las imagenes de las distintas etapas del procesado
	void display();

	const IplImage*	getImg(int ncanal) const;
	
	// Devuelve el objeto i-ésimo que se ha detectado en la imagen segmentada
	inline const Objeto* objeto(int i) const
	{
		if (i>=0 && _v_obj.size()>(unsigned)i)
			return _v_obj[i];
		else
			return NULL;
	}

	static inline CvMemStorage* GetStorage()
	{
		return Storage;
	}

	// Parámetros de configuracion de este procesador
	struct Config 
	{
		int		wsh_steps;
		int		wsh_step;
		int		wsh_softness;
		int		wsh_umbral;
		double	th_adjOtsu;
		int		th_smooth;
		int		th_threshold;
	} _cfg;

	static const Config CFG_DEFAULT;


private:
	Procesador();

	// Preprocesa la imagen (separa del fondo, calcula histograma, convierte a B/n, binario, etc.)
	void preprocesar();

	// Segmenta la imagen (separa objetos juntos)
	IplImage*	segmentar();

	int			calcularHistograma(const IplImage* src);

	static CvMemStorage* Storage;

	//Moments* _momentos;
	
	// Imagenes de las distintas etapas del procesado
	IplImage *_img, *_imgdown, *_imgbw, *_imgbw2, *_src, *_dst, *_edges, *_watershed, *_dmap, *_histogram, *_imgobjetos;

	// Contenedor de objetos detectados en la imagen
	vector<Objeto *>	_v_obj;
};
