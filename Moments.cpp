/*
 * Modulo: Vision artificial
 *
 * Descripcion: Implementacion de la clase Moments
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007. All rights reserved.
 * 
 * $Id: Moments.cpp 266 2008-03-16 18:34:30Z Alvaro $
 */

#include "stdafx.h"
#include "Imagen.h"
#include "Moments.h"

Moments::Moments(const CvArr* arr, bool binary)
{
	cvMoments(arr, &_moments, binary?1:0);
	cvGetHuMoments(&_moments, &_momentsHu);

	_m00 = m(0,0);
	_cgx = m(1,0)/_m00;
	_cgy = m(0,1)/_m00;
	_u2002 = mu(2,0)-mu(0,2);
	_u11 = mu(1,1);

	_theta = calc_theta();

	// epsilon
	_epsilon = abs( (_u2002*_u2002 - 4.*_u11*_u11) /
					( (mu(2,0)+mu(0,2))*(mu(2,0)+mu(0,2)) ) );
}

Moments::~Moments()
{
}


inline double Moments::m(int x_order, int y_order) const
{
	return cvGetSpatialMoment(const_cast<CvMoments*>(&_moments), x_order, y_order);
}

inline double Moments::mu(int p, int q) const
{
	if (p==0 && q==0) return m(0,0);
	if ((p==1 && q==0) || (p==0 && q==1)) return 0.;

	double mupq = m(p,q)/m(0,0);

	double g=1.,h=1.;
	while(p>0) { g *= m(1,0)/m(0,0); p--; }
	while(q>0) { h *= m(0,1)/m(0,0); q--; }

	return mupq - g * h;
	
	// hace lo mismo
	// return cvGetCentralMoment(const_cast<CvMoments*>(&_moments), p, q);
}

// inline double nu(int x_order, int y_order) const
// {
//	return cvGetNormalizedCentralMoment(const_cast<CvMoments*>(&_moments), x_order, y_order);
// }


inline double Moments::Hu(int n) const
{
	switch(n)
	{
	case 0:
		return 0.;
	case 1:
		return _momentsHu.hu1;
	case 2:
		return _momentsHu.hu2;
	case 3:
		return _momentsHu.hu3;
	case 4:
		return _momentsHu.hu4;
	case 5:
		return _momentsHu.hu5;
	case 6:
		return _momentsHu.hu6;
	case 7:
		return _momentsHu.hu7;
	default:
		return 0.;
	}
	return 0.;
}

inline double Moments::semimajor_axis() const //a
{
	return m(0,0)/(M_PI*semiminor_axis());
}

inline double Moments::semiminor_axis() const //b
{
	// double bdiva = (1.-abs(epsilon())) / (1.+abs(epsilon())); //sqrt(1.-epsilon()*epsilon()); =
	// double bmula = m(0,0)/M_PI;
	// double b = sqrt(bdiva*bmula);
	// double a = bmula/b;

	return sqrt( m(0,0) * (1.-abs(epsilon())) / ((1.+abs(epsilon()))*M_PI) );
}

// de IShapeDesc
vector<double> Moments::getVector() const
{
	vector<double> v;

	v.push_back(_momentsHu.hu1);
	v.push_back(_momentsHu.hu2);
	v.push_back(_momentsHu.hu3);
	v.push_back(_momentsHu.hu4);
	v.push_back(_momentsHu.hu5);
	v.push_back(_momentsHu.hu6);
	v.push_back(_momentsHu.hu7);
	
	return v;
}

// de IShapeDesc
int Moments::getDim() const
{
	return 7;
}




void Moments::calcular_ancho_largo(CvArr* imgbin, double* ancho, double* largo) const
{
	double b = semiminor_axis();
	double a = semimajor_axis();

	if (b>a) printf("*************** ERROR: b>a!\n");

	double ax = -1., ay = tan(-theta());
	double bx = tan(-theta()), by = 1.;

	normaliza(a, ax, ay);
	normaliza(b, bx, by);

	ImagenBn img(static_cast<IplImage*>(imgbin)); // solo recubre
	
	////////////

	// determinamos el grosor del objeto que cruza a la linea
	*largo = img.buscar_borde_en_linea((int)(cgx()-ax*2), (int)(cgy()-ay*2), (int)(cgx()+ax*2),(int)(cgy()+ay*2));
	*ancho = img.buscar_borde_en_linea((int)(cgx()-bx*2), (int)(cgy()-by*2), (int)(cgx()+bx*2),(int)(cgy()+by*2));
}


void Moments::dibujarEjes(CvArr* img) const
{
	double b = semiminor_axis();
	double a = semimajor_axis();

	if (b>a) printf("*************** ERROR: b>a!\n");

	double ax = -1., ay = tan(-theta());
	double bx = tan(-theta()), by = 1.;

	normaliza(a, ax, ay);
	normaliza(b, bx, by);

	cvLine(img, cvPoint((int)(cgx()-ax), (int)(cgy()-ay)), cvPoint((int)(cgx()+ax),(int)(cgy()+ay)), cvScalar(255,100,0), 2, CV_AA);
	cvLine(img, cvPoint((int)(cgx()-bx), (int)(cgy()-by)), cvPoint((int)(cgx()+bx),(int)(cgy()+by)), cvScalar(200,200,0), 2, CV_AA);
	
	cvEllipse(img, cvPoint((int)cgx(), (int)cgy()), cvSize((int)(a+2),(int)(b+2)), -theta()*180./M_PI,
            0, 360, cvScalar(150,255,255), 1, CV_AA);

	
	cvCircle(img, cvPoint((int)cgx(), (int)cgy()), 4, cvScalar(255,255,255), -1, CV_AA);
	cvCircle(img, cvPoint((int)cgx(), (int)cgy()), 3, cvScalar(50,180,0), -1, CV_AA);

}

void Moments::printHu() const
{
	for (int i=1; i<=7; i++)
		printf("Hu%d = %f\n", i, Hu(i));
}



double Moments::calc_theta() const
{
	// theta
	if (_u2002==0)
	{
		if (_u11==0)
			return 0.;
		else if (_u11>0)
			return M_PI_4;  // /4.;
		else
			return -M_PI_4; // /4.;
	}
	else if (_u11==0)
	{
		if (_u2002>0)
			return 0.;
		else
			return -M_PI_2; // /2.;
	}

	double theta = .5 * atan(2.*_u11/_u2002);

	if (_u2002<0)
	{
		if (_u11>0)
			theta += M_PI_2; // / 2.;
		else if (_u11<0)
			theta -= M_PI_2; // / 2.;
	}

	return theta;
}

inline void Moments::normaliza(double a, double& x, double& y) const
{
	double n = sqrt(x*x+y*y);
	x *= a / n;
	y *= a / n;
}


