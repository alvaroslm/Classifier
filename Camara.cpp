/*
 * Modulo: Vision artificial
 *
 * Descripcion: Implementacion de la clase Camara para capturar imagenes
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 * 
 * $Id: Camara.cpp 404 2008-04-06 14:47:51Z Alvaro $
 */

#include "stdafx.h"
#include "Camara.h"

Camara::Camara(int index/*=CV_CAP_ANY*/)
{
	IplImage* frame;
	//_capture = cvCaptureFromCAM(index);  //CV_CAP_ANY es 0

	do
	{
		_capture = cvCaptureFromCAM(index++);
		if (_capture)
			frame = cvQueryFrame(_capture);
		else
			frame = NULL;
	}
	while(frame==NULL && index<8);

	//cvReleaseImage(&frame); no hay q hacerlo sobre las de webcam 

	/*if (_capture)
	{ no funciona
		cvQueryFrame(_capture);

		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH, 640);
		cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT, 480);

	}*/
}

Camara::~Camara(void)
{
	cvReleaseCapture(&_capture);
}


IplImage* Camara::queryFrame(bool bSelect)
{
	if (!_capture)
		return NULL;

	if (bSelect)
		cvNamedWindow( "Camara", CV_WINDOW_AUTOSIZE );

	int wk;
	IplImage* frame = NULL;
	do 
	{
		// Get one frame
		frame = cvQueryFrame(_capture);
		if (!frame) 
		{
			if (bSelect)
				cvDestroyWindow( "Camara" );
			fprintf( stderr, "ERROR: frame is null...\n" );
			return NULL;
		}

		if (bSelect)
			cvShowImage( "Camara", frame );
		
		wk = (cvWaitKey(10) & 255);
	} while (bSelect && wk!=13 && wk!=' '); // escape seria 27

	cvDestroyWindow("Camara");

	return frame;
}
