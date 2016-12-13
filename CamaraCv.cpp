/*
 * Modulo: Vision artificial
 *
 * Descripcion: Implementacion de la clase CamaraCv para capturar imagenes
 *				CVCAM no funciona el callback devuelve siempre null....
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2008. All rights reserved.
 *
 * $Id: CamaraCv.cpp 396 2008-04-05 01:56:56Z Alvaro $
 */

#include "stdafx.h"
#include "CamaraCv.h"
#include "cvcam.h"


CamaraCv*	CamaraCv::Camaras[CamaraCv::MAX_CAMARAS];
int			CamaraCv::NumCamaras = 0;

template<int N> inline void	_callbackCamaras(IplImage* img)
{
	CamaraCv::Camaras[N]->callback(img);
}
void _callbackCamaras0(IplImage* img) { _callbackCamaras<0>(img); }
void _callbackCamaras1(IplImage* img) { _callbackCamaras<1>(img); }
void _callbackCamaras2(IplImage* img) { _callbackCamaras<2>(img); }
void _callbackCamaras3(IplImage* img) { _callbackCamaras<3>(img); }
void _callbackCamaras4(IplImage* img) { _callbackCamaras<4>(img); }
void _callbackCamaras5(IplImage* img) { _callbackCamaras<5>(img); }
void _callbackCamaras6(IplImage* img) { _callbackCamaras<6>(img); }
void _callbackCamaras7(IplImage* img) { _callbackCamaras<7>(img); }

void CamaraCv::ConfigCamara(int index)
{
	cvcamSetProperty(index, CVCAM_CAMERAPROPS, 0);
}


///

CamaraCv::CamaraCv(int index/* =0 */) : m_imagen(NULL), m_ncamara(index), m_cvcam(NULL)
{
	int ncamaras = cvcamGetCamerasCount();
	NumCamaras = (ncamaras>MAX_CAMARAS) ? MAX_CAMARAS : ncamaras;

	if (m_ncamara<0) m_ncamara = 0;
	else 
	if (m_ncamara>=NumCamaras) m_ncamara = NumCamaras-1;

//	printf("ncamara=%d\n", m_ncamara);

	int res=0;
	res = cvcamSetProperty(m_ncamara, CVCAM_PROP_ENABLE, CVCAMTRUE);
//	printf("cvspE=%d\n", res);
	
	res = cvcamSetProperty(m_ncamara, CVCAM_PROP_RENDER, 0);
//	printf("cvspR=%d\n", res);
	//cvcamSetProperty(m_ncamara, CVCAM_PROP_WINDOW, 0);

//	 CameraDescription scam;
//	cvcamGetProperty(m_ncamara, CVCAM_DESCRIPTION, (void*)&scam);
//	printf("DEVICEDESCRIPTION: %s\n", scam.DeviceDescription);

	Camaras[m_ncamara] = this;

//	printf("enabled\n");

	switch(m_ncamara)
	{
	 case 0: cvcamSetProperty(index, CVCAM_PROP_CALLBACK, (void*)&_callbackCamaras0); break;
	 case 1: cvcamSetProperty(index, CVCAM_PROP_CALLBACK, (void*)&_callbackCamaras1); break;
	 case 2: cvcamSetProperty(index, CVCAM_PROP_CALLBACK, (void*)&_callbackCamaras2); break;
	 case 3: cvcamSetProperty(index, CVCAM_PROP_CALLBACK, (void*)&_callbackCamaras3); break;
	 case 4: cvcamSetProperty(index, CVCAM_PROP_CALLBACK, (void*)&_callbackCamaras4); break;
	 case 5: cvcamSetProperty(index, CVCAM_PROP_CALLBACK, (void*)&_callbackCamaras5); break;
	 case 6: cvcamSetProperty(index, CVCAM_PROP_CALLBACK, (void*)&_callbackCamaras6); break;
	 case 7: cvcamSetProperty(index, CVCAM_PROP_CALLBACK, (void*)&_callbackCamaras7); break;
	 default: cvcamSetProperty(index, CVCAM_PROP_CALLBACK, (void*)&_callbackCamaras0); break;
	}
//	printf("SP\n");

	res=cvcamInit();
//	printf("i=%d\n",res);

//	ConfigCamara(m_ncamara);

	res=cvcamStart();
//	printf("s=%d\n",res);
}

CamaraCv::~CamaraCv(void)
{
	cvcamStop();
	cvcamExit();

	//cvReleaseImage(&m_imagen);

	Camaras[m_ncamara] = NULL;
}


void CamaraCv::callback(IplImage* img)
{
	//IplImage* lastimage = m_imagen;
	m_imagen = img; // el cambio de imagen es una operacion atómica

	Camaras[0]->m_imagen = img;

	// notificar..

	//cvReleaseImage(&lastimage);
}

IplImage* CamaraCv::queryFrame(bool bSelect)
{
	return m_imagen;
}



