/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion de la clase Moments
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 *
 * $Id: Moments.h 266 2008-03-16 18:34:30Z Alvaro $
 */

#pragma once

#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#pragma warning( disable : 4996 )
#include "cv.h"
#include "cxcore.h"
#include "cvaux.h"
#include "highgui.h"
#pragma warning( disable :  )

#include "IShapeDesc.h"

#include <vector>
using namespace std;



// Momentos de una imagen, parametros asociados, valores derivados etc
class Moments : public IShapeDesc
{
public:
	// Constructor
	Moments(const CvArr* arr, bool binary=true);

	// Destructor
	virtual ~Moments();


	// Momento de orden x,y
	inline double m(int x_order, int y_order) const;

	// Momento central
	inline double mu(int p, int q) const;

	// Momento central normalizado
	//inline double nu(int x_order, int y_order) const;



	// Area
	inline double area() const
	{
		return _m00; //m(0,0);
	}

	// Centro de gravedad (x)
	inline double cgx() const
	{
		return _cgx; //m(1,0)/m(0,0);
	}

	// Centro de gravedad (y)
	inline double cgy() const
	{
		return _cgy; //m(0,1)/m(0,0);
	}

	// Excentricidad
	inline double epsilon() const
	{
		// return abs( ((mu(2,0)-mu(0,2))*(mu(2,0)-mu(0,2)) -
		//			4.*mu(1,1)*mu(1,1))/( (mu(2,0)+mu(0,2))*(mu(2,0)+mu(0,2)) ) );

		return _epsilon;
	}

	// Inclinacion
	inline double theta() const
	{
		return _theta;
	}

	// Invariantes de Hu (1-7)
	inline double Hu(int n) const;

	// Semieje mayor de la elipse equivalente
	inline double semimajor_axis() const; //a

	// Semieje menor de la elipse equivalente
	inline double semiminor_axis() const; //b

	// Dibuja los ejes calculados en la imagen que se pida
	void dibujarEjes(CvArr* img) const;

	// Calcula el ancho y el largo segun los ejes usando los bordes del objeto (no usando la elipse)
	void calcular_ancho_largo(CvArr* imgbin, double* ancho, double* largo) const;

	// Vector que describe la forma
	virtual vector<double> getVector() const;
	
	// Numero de dimensiones
	virtual int getDim() const;

	// Muestra en pantalla
	void printHu() const;


private:
	Moments() {}

	double calc_theta() const;

	inline void normaliza(double a, double& x, double& y) const;

	// Momentos
	CvMoments	_moments;

	// Momentos Hu
	CvHuMoments	_momentsHu;

	// Variables internas precalculadas
	double _u2002, _u11, _cgx, _cgy, _theta, _epsilon, _m00;
};


//////////////////////////


		/*
	void eigenvects_etc() 
	{		
		
		CvMat *mat, *evals, *ev;
		mat = cvCreateMat(2, 2, CV_32F);
		evals = cvCreateMat(2, 1, CV_32F);
		ev = cvCreateMat(2, 2, CV_32F);

		cvSetReal2D(mat, 0,0, mu(2,0));
		cvSetReal2D(mat, 1,1, mu(0,2));
		cvSetReal2D(mat, 1,0, -mu(1,1));
		cvSetReal2D(mat, 0,1, -mu(1,1));

		CvMat *copy = cvCloneMat(mat);

		///cvEigenVV(mat, eigenvals, eigenvects);
		cvSVD(mat, evals, ev, 0, CV_SVD_U_T + CV_SVD_MODIFY_A);

		printf("\nevec\n[ %f %f ]",   cvGetReal2D(ev, 0, 0), cvGetReal2D(ev, 0, 1)); 
		printf("\n[ %f %f ]\n", cvGetReal2D(ev, 1, 0), cvGetReal2D(ev, 1, 1)); 

		printf("\nmat\n[ %f %f ]",   cvGetReal2D(copy, 0, 0), cvGetReal2D(copy, 0, 1)); 
		printf("\n[ %f %f ]\n", cvGetReal2D(copy, 1, 0), cvGetReal2D(copy, 1, 1)); 

		printf("\nevec* a\n[ %f ]",  cvGetReal2D(ev, 0, 0)*cvGetReal2D(copy, 0, 0) + cvGetReal2D(ev, 0, 1)*cvGetReal2D(copy, 0, 1) ); 
		printf("\n[ %f ]\n",  cvGetReal2D(ev, 0, 0)*cvGetReal2D(copy, 1, 0) + cvGetReal2D(ev, 0, 1)*cvGetReal2D(copy, 1, 1) ); 

		printf("\nevec* b\n[ %f ]",  cvGetReal2D(ev, 1, 0)*cvGetReal2D(copy, 0, 0) + cvGetReal2D(ev, 1, 1)*cvGetReal2D(copy, 0, 1) ); 
		printf("\n[ %f ]\n",  cvGetReal2D(ev, 1, 0)*cvGetReal2D(copy, 1, 0) + cvGetReal2D(ev, 1, 1)*cvGetReal2D(copy, 1, 1) ); 

		printf("\nevals\n[ %f %f ]\n",   cvGetReal2D(evals, 0, 0), cvGetReal2D(evals, 1, 0)); 


		//return atan(cvGetReal2D(ev, 0, 1)/ cvGetReal2D(ev, 1, 1)) * 180./M_PI; << angulo
		printf("--ANGULO = %f\nn", atan(cvGetReal2D(ev, 0, 1)/ cvGetReal2D(ev, 1, 1)) * 180./M_PI);
		//return sqrt( 0.5*(mu(2,0)+mu(0,2)) + sqrt( 4.*mu(1,1)*mu(1,1) - (mu(2,0)-mu(0,2))*(mu(2,0)-mu(0,2)) ) );
	}
		/*double ax = a*cvGetReal2D(ev, 1, 1), ay = a*cvGetReal2D(ev, 0, 1);
		double bx = b*cvGetReal2D(ev, 1, 0), by = b*cvGetReal2D(ev, 0, 0);*/

		//printf("-- %f --\n", atan(cvGetReal2D(ev, 0, 1)/cvGetReal2D(ev, 1, 1))/M_PI*180.);
	/* */


