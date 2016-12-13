/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion de la clase Objeto
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007. All rights reserved.
 * 
 * $Id: Objeto.h 391 2008-04-04 18:09:59Z Alvaro $
 */

#pragma once

#include <string>

#include "Imagen.h"
#include "IShapeDesc.h"

#include "Moments.h"
#include "FourierShapeDesc.h"


// Contiene un objeto visualmente separado en la imagen
class Objeto
{
public:
	// Constructor, recibe la imagen y una mascara binaria para separar el objeto
	Objeto(IplImage* img, IplImage* imgbin, const char* name, bool dibujarEjes=false);
	
	// Destructor
	~Objeto();

	// Devuelve el nombre
	inline const char* getName() const
	{
		return _name.c_str();
	}

	// Muestra el objeto en una ventana
	void display(int x=-1, int y=-1) const;

	// Pinta el objeto en el DC especificado
	void display(HDC dc, int width=0, int height=0, int xoffs=0, int yoffs=0) const;

	// Devuelve la imagen del objeto
	inline const IplImage* getImg() const
	{
		return (const IplImage *)_img;
	}


	// cuando el procesador detecte los bordes de la plataforma, pasara a objeto
	// la matriz de transformacion correspondiente, y aqui se corregiran la posicion 
	// y ejes y demas para dar coordenadas sobre el plano de la plataforma, no sobre
	// la imagen capturada por la camara
	// 
	// Actualizacion: las coordenadas aqui siguen siendo las de la imagen; la traduccion
	//				  de coordenadas a las de la cinta se hace en ejecutivo

	// Angulo del objeto
	inline double angulo() const
	{
		return _momentos.theta();
	}

	// Posicion X del objeto
	inline double posX() const
	{
		return _momentos.cgx();
	}

	// Posicion Y del objeto
	inline double posY() const
	{
		return _momentos.cgy();
	}

	// Anchura del objeto
	inline double ancho() const
	{
		return _ancho;
	}

	// Longitud del objeto
	inline double largo() const
	{
		return _largo;		
	}

	// Longitud del semieje menor de la elipse (aprox ancho)
	inline double ancho_eje() const
	{
		return _momentos.semiminor_axis()*2.0;
	}

	// Longitud del semieje mayor de la elipse (aprox largo)
	inline double largo_eje() const
	{
		return _momentos.semimajor_axis()*2.0;
	}

	inline const IShapeDesc* getDesc_Fourier() const
	{
		return &_fourierDescriptor;
	}

	inline const IShapeDesc* getDesc_HuMoments() const
	{
		return &_momentos;
	}

	// Devuelve el vector combinado de varios descriptores
	inline const vector<double> getCombinedVector() const
	{
		vector<double> v = _momentos.getVector();
		v.push_back(largo()/10.0); // metemos tambien la longitud del objeto como indicacion de tamaño
		const vector<double>& f = _fourierDescriptor.getVector();
		v.insert(v.end(), f.begin(), f.end()); // metemos el otro descriptor despues
		return v;
	}

	inline const BBox<int>*	getBBox() const
	{
		return _fourierDescriptor.getBBox();
	}

	// Devuelve los momentos correspondientes a la imagen del objeto
	/*inline const Moments& getMomentos() const
	{
		return _momentos;
	}
	// Devuelve el descriptor de Fourier correspondientes a la imagen del objeto
	inline const FourierShapeDesc& getFourierDescriptor() const
	{
		return _fourierDescriptor;
	}*/

//	Objeto() : _img(0), _imgbin(0) {}

private:
	// Momentos del objeto
	Moments		_momentos;

	// Descriptor DFT
	FourierShapeDesc	_fourierDescriptor;

	// Imagen en color del objeto
	ImagenRgb	_img;

	// Imagen binaria 0/255 del objeto
	ImagenBn	_imgbin;

	// Nombre del objeto
	string		_name;

	// Ancho (menor dimension)
	double	_ancho;

	// Largo (mayor dimension)
	double  _largo;
};
