/*
 * Modulo: Vision artificial
 *
 * Descripcion: Implementacion de la clase FourierShapeDesc
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007. All rights reserved.
 * 
 * $Id: FourierShapeDesc.cpp 400 2008-04-05 23:54:09Z Alvaro $
 */


#include "stdafx.h"
#include "FourierShapeDesc.h"
#include "Procesador.h"


int FourierShapeDesc::Nsamples = 32; // por defecto 32


FourierShapeDesc::FourierShapeDesc(const CvArr* arr, const Moments& momentos, bool binary)
{
	_xn = new complex<double> [Nsamples];
	_Xk = new complex<double> [Nsamples];
	_ixn = new complex<double> [Nsamples];
	_vector = new double [Nsamples-2];

	CvSize	size = cvGetSize(arr);
	CvPoint centro; 
	centro.x=size.width/2; 
	centro.y=size.height/2;
	
	// rotar imagen y centrarla
/*	_img = cvCreateImage(size, 8, 1);
	
	CvMat* map_matrix = cvCreateMat(2, 3, CV_32F);
	cv2DRotationMatrix( cvPoint2D32f(momentos.cgx(), momentos.cgy()),  momentos.theta()*180./M_PI, 1., map_matrix );
	cvSetReal2D(map_matrix, 0, 2, cvGetReal2D(map_matrix,0,2) + (double)centro.x - momentos.cgx());
	cvSetReal2D(map_matrix, 1, 2, cvGetReal2D(map_matrix,1,2) + (double)centro.y - momentos.cgy());
	
	cvWarpAffine(arr, _img, map_matrix);
	cvReleaseMat(&map_matrix); /**/
	// no hace falta ^, el vector resultado es invariante a rotacion, tras., scale; simplemente la copiamos:
	_img = cvCloneImage(static_cast<const IplImage*>(arr));

	// ahora hallamos su contorno
	IplImage* src2 = cvCloneImage(_img);

	CvSeq* contour = 0;
	cvFindContours( src2, Procesador::GetStorage(), &contour, sizeof(CvContour),
					CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	
	if (contour!=0 && contour->total>Nsamples)
	{
		_bbox = NULL;

		CvScalar color = cvScalar(0);

		CvPoint punto;
		int i=0, total=contour->total;
		while(contour->total>0)
		{
			cvSeqPopFront(contour, &punto);
			if (_bbox==NULL)
				_bbox = new BBox<int>(punto.x, punto.y);
			else
				_bbox->addPoint(punto.x, punto.y);

			if (i%(total/Nsamples)==0)
			{
				//printf("%d x=%d, y=%d\n", Nsamples*i/total, punto.x, punto.y);
				assert( i<total);
				_xn[Nsamples*i/total] = complex<double>((double) punto.x - centro.x, (double) punto.y - centro.y);
			}
			++i;
		}
 
		// dibujamos el centro
		punto = centro;
		color = cvScalar(0);
		cvCircle( _img, punto, 3, color);

		for(i=0; i<Nsamples+2; ++i)
		{
			//if (_startxn<0 && _xn[i%Nsamples].imag()<=0. && _xn[(i+1)%Nsamples].imag()>0.)
			//	_startxn = (i+1)%Nsamples;

			punto.x = ((int)_xn[i%Nsamples].real()) + centro.x;
			punto.y = ((int)_xn[i%Nsamples].imag()) + centro.y;

			if (i==0)
			{
				color = cvScalar(0);
				cvCircle( _img, punto, 6, color, CV_FILLED);
				color = cvScalar(255);
				cvCircle( _img, punto, 6, color);
			}
			else
			{
				color = cvScalar(255);
				cvCircle( _img, punto, 4, color, CV_FILLED);
				color = cvScalar(0);
				cvCircle( _img, punto, 4, color);
			}
		}

		calc_dft();
		calc_idft();

		//P printf("vector = ");
		for(i=0; i<Nsamples; ++i)
		{
			//P if (i<Nsamples-2) 
			//P	 printf("%.4f, ", _vector[i]);

			punto.x = (int)(_ixn[i].real()* 250.) + centro.x;
			punto.y = (int)(_ixn[i].imag()* 250.) + centro.y;

			color = cvScalar(255);
			cvCircle( _img, punto, 2, color, CV_FILLED);
			color = cvScalar(0);
			cvCircle( _img, punto, 2, color);
			//P printf("\nXk[%d] = (%.3f, %.3f)", i, _Xk[i].real(), _Xk[i].imag());
		}
		//P printf("\n");
	}
	else
		_bbox = new BBox<int>(0, 0);

	cvReleaseImage(&src2);

}


void FourierShapeDesc::calc_dft()
{
	int k;
	for(k=0; k<Nsamples; ++k)
	{
		_Xk[k] = complex<double>(0.,0.);

		for(int n=0; n<Nsamples; ++n)
			_Xk[k] += _xn[n] * exp(complex<double>(0., -2.*M_PI* (k-Nsamples/2+1) *n/(double)Nsamples ));
	}

	printf("\nVecF = ");
	for(k=0; k<Nsamples/2-1; ++k)
	{
		complex<double> absterm = abs(_Xk[k]) / abs(_Xk[Nsamples/2]);
		_vector[k] = absterm.real();
		printf("%.4f ", (float)_vector[k]);
	}
	printf("* ");
	// pasamos del F0, el F1 lo usamos para normalizar
	complex<double> norm = abs(_Xk[Nsamples/2]);
	for(; k<Nsamples-2; ++k)
	{
		complex<double> absterm = abs(_Xk[k+2]) / norm;
		_vector[k] = absterm.real();
		printf("%.4f ", (float)_vector[k]);
	}
	printf("\n\n");
}

void FourierShapeDesc::calc_idft()
{
	for(int n=0; n<Nsamples; ++n)
	{
		_ixn[n] = complex<double>(0.,0.);

		for(int k=0; k<Nsamples; ++k)
		{
			// reconstruimos la forma "normalizada"
			complex<double> Xkk = abs(_Xk[k])/abs(_Xk[Nsamples/2]); //(0., abs(_Xk[k].imag() / _Xk[Nsamples/2].imag()));
			if ((k-Nsamples/2+1)!=0) // el F0 seria 0 xq lo descartamos (solo depende de la pos. del obj)
				_ixn[n] += Xkk * exp(complex<double>(0., 2.*M_PI* (k-Nsamples/2+1) *n/(double)Nsamples ));
		}

		_ixn[n] /= (double)Nsamples;
	}
}


FourierShapeDesc::~FourierShapeDesc()
{
	if (_img) 
		cvReleaseImage(&_img);

	delete [] _xn;
	delete [] _Xk;
	delete [] _ixn;
	delete [] _vector;
	delete _bbox;
}


// de IShapeDesc
vector<double> FourierShapeDesc::getVector() const
{
	vector<double> v;

	for(int i=0; i<Nsamples-2; ++i)
		v.push_back(_vector[i]);

	return v;
}

// de IShapeDesc
int FourierShapeDesc::getDim() const
{
	return Nsamples-2;
}

const IplImage* FourierShapeDesc::getImg() const
{
	return _img;
}


const BBox<int>* FourierShapeDesc::getBBox() const
{
	return _bbox;
}
