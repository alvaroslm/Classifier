/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion de la clase Imagen
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007. All rights reserved.
 *
 * $Id: Imagen.h 400 2008-04-05 23:54:09Z Alvaro $
 */

#pragma once

#undef DBG_IMG_INST // si esta activado mantiene contador de instancias de imagen

#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

#pragma warning( disable : 4996 )
#include "cv.h"
#include "cxcore.h"
#include "cvaux.h"
#include "highgui.h"
#pragma warning( disable :  )

#include "pixelstructs.h"



// Clase para manipulacion de imagenes IplImage
template<class T> class Imagen
{
public:
	// Constructores y destructores

	// Constructor, crea una nueva imagen (especializado segun tipo de imagen)
	Imagen(CvSize size);

	// Constructor de copia
	inline Imagen(const Imagen<T>& im) : _bWrap(false)
	{
		_img = cvCloneImage(im._img);
#		ifdef DBG_IMG_INST
		Instances++;
#		endif
	}

	// Constructor de copia para const IplImage* (implicito, se usara en asignaciones de inicializacion por ejemplo)
	inline Imagen(const IplImage* img) : _bWrap(false)
	{
		_img = cvCloneImage(img);
#		ifdef DBG_IMG_INST
		Instances++;
#		endif
	} 

	// Constructor explicito para IplImage*, se usara para no crear copia (en inicializacion con constructor)
	explicit inline Imagen(IplImage* img, bool bManage=false)  : _bWrap(!bManage)
	{
		_img = img;
#		ifdef DBG_IMG_INST
		if (bManage) Instances++;
#		endif
	}

	// Destructor
	~Imagen()
	{
		if (_img && !_bWrap) 
		{
			cvReleaseImage(&_img);
#			ifdef DBG_IMG_INST
			Instances--;
#			endif
		}
		
		_img = 0;
	}

	/////////////////////////////

	// Operadores y funciones de gestion de memoria

	// Devuelve una copia de la imagen
	inline IplImage* getClone() const
	{
		return cvCloneImage(_img);
	}

	// Devuelve la imagen y desasocia el objeto
	inline IplImage* unBind()
	{
	#	ifdef DBG_IMG_INST
		if (_bWrap==false)
			Instances--;
	#	endif

		_bWrap = true;
		IplImage* imag = _img;
		_img = 0;
		return imag;
	}

	// Cast para poder usar esta clase como equivalente a IplImage
	inline operator IplImage *();

	// Cast para poder usar esta clase como equivalente a IplImage (const)
	inline operator IplImage const *() const;

	// operador -> para acceder a miembros de IplImage directamente
	IplImage* operator -> ();
	
	// operador -> para acceder a miembros de IplImage directamente (const)
	const IplImage* operator -> () const;

	// Se asigna a img (para copiar la imagen; para recubrir se usa el constructor)
	inline void operator=(const IplImage* img);

	// Este objeto se apropia de la imagen (NO la copia)
	inline void operator=(const Imagen<T>& img);

	// Acceso a pixels de la imagen (const)
	inline const T* operator[]( int rowIndx) const;

	// Acceso a pixels de la imagen
	inline T* operator[]( int rowIndx);

	
	/////////////////////////////


	// Crea un mapa de distancias a partir de la imagen
	IplImage*	createDistanceMap(const int steps, const int step, const int softness) const;

	// Normaliza los valores de los pixels de la imagen
	void normalizar(int minimo=0, int maximo=255);
	
	// Invierte los valores de los pixels
	void invertir(bool bIgnore0=false);

	// Comprueba si una imagen esta vacia (todo 0)
	bool empty() const;

	// Aplica algoritmo de watershed a una imagen que sea ya mapa de distancias
	int watershed(unsigned char umbralObjeto);

	// Pinta la imagen a un DC destino, de acuerdo con ciertos parametros
	void paint(HDC hdc, int x, int y, int w, int h, int from_x=0, int from_y=0) const;

	// Pinta la imagen a un DC destino, de acuerdo con ciertos parametros
	void paint(HDC hdc, const RECT& rc, int from_x=0, int from_y=0) const;

	// Pinta la imagen (teniendo en cuenta ROI) a un DC destino
	// void paint( HDC hDCDst, RECT* pDstRect ) const;

	// Calcula un umbral para el fondo, con el algoritmo de Otsu (optimizado y modificado)
	int otsuThreshold(double ajuste=0., const IplImage* imask=0) const;

	// Devuelve el canal h,s,v,r,g ó b de la imagen
	IplImage* splitChannel(const char channel, IplImage* dest=0) const;

	// Analiza el histograma
	int	calcStats(int& min, int& max, const int percent=100, bool sat=false) const;

	// Threshold
	void threshold(int low, int high, bool binary=false, IplImage* origen=0);

	// Busca dos bordes negro->blanco segun la linea para devolver el grosor del objeto segun esa linea
	double buscar_borde_en_linea(int x0, int y0, int x1, int y1) const;


	///////  operadores //////////

	// Suma de los pixels + valor
	inline Imagen<T> operator +(const T& val) const;
	
	// Resta de los pixels - valor
	inline Imagen<T> operator -(const T& val) const;
	
	// Multiplicacion de los pixels * valor
	inline Imagen<T> operator *(const T& val) const;
	
	// Division de los pixels / valor
	//inline Imagen<T> operator /(const T& val) const;

	// Suma de los pixels
	inline Imagen<T> operator +(const Imagen<T>& img2) const;

	// Resta de los pixels
	inline Imagen<T> operator -(const Imagen<T>& img2) const;

	// Multiplicacion de los pixels
	inline Imagen<T> operator *(const Imagen<T>& img2) const;

	// Suma y asignacion
	inline const Imagen<T>& operator +=(const Imagen<T>& img2);

	// Resta y asignacion
	inline const Imagen<T>& operator -=(const Imagen<T>& img2);
	
	// Multiplicacion y asignacion
	inline const Imagen<T>& operator *=(const Imagen<BwPixel>& img2);

	// Estas 4 funciones son necesarias porque no sabe si hacer cast a IplImage* y sumarle un int al ptr
	// o al reves, construir un T con int y usar el operador +- de Image (esto ultimo es lo que queremos).
	inline Imagen<T> operator +(int val) const { return operator +(T(val)); }
	inline Imagen<T> operator -(int val) const { return operator -(T(val)); }
	//inline const Imagen<T>& operator +=(int val) { return operator +=(T(val)); }
	//inline const Imagen<T>& operator -=(int val) { return operator -=(T(val)); }

#ifdef DBG_IMG_INST
	static int Instances;
#endif

private:

	Imagen() : _bWrap(true), _img(0) {}

	// Va procesando pixels adyacentes a un blanco en orden adecuado acorde con el bucle i++ j++
	inline int watershedBlanco(const Imagen<BwPixel>& limpia, int i, int j, unsigned char val);

	// Procesa un pixel
	inline int watershedpixNew(const Imagen<BwPixel>& limpia, int i, int j);

	// Rellena BITMAPINFO
	void fillBitmapInfo( BITMAPINFO* bmi, int width, int height, int bpp, int origin ) const;

	//// Variables privadas

	// Imagen que contenemos
	IplImage*	_img;

	// Indica si solo recubrimos la imagen o si vamos a liberarla al ser destruidos
	bool		_bWrap;
};


typedef Imagen<RgbPixel>		ImagenRgb;
typedef Imagen<RgbPixelFloat>	ImagenFloatRgb;
typedef Imagen<BwPixel>			ImagenBn;
typedef Imagen<float>			ImagenFloat;


