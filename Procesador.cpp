/*
 * Modulo: Vision artificial
 *
 * Descripcion: Implementacion de la clase Procesador
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 *
 * $Id: Procesador.cpp 424 2008-04-09 18:40:11Z vegas $
 */

#include "stdafx.h"
#include <time.h>

#include "Procesador.h"

#include "Moments.h"

#include "Imagen.h"
#include "Objeto.h"
#include "Matrix.h"

#include "Classifier.h"


#undef RELLENAR_CONCAVIDADES // determina si se rellenaran concavidades del contorno
							 // detectadas en los objetos separados despues de segmentar


CvMemStorage* Procesador::Storage = 0;

//	wsh_steps, wsh_step, wsh_softness, wsh_umbral, (double)th_adjOtsu, th_smooth, th_threshold;
const Procesador::Config Procesador::CFG_DEFAULT = {10, 2, 23, 40, -15., 29, -1};


Procesador::Procesador() :	_img(0), _imgdown(0), _imgbw(0), _imgbw2(0), _dmap(0), _src(0),
							_dst(0), _edges(0), _watershed(0), _histogram(0), _imgobjetos(0)
{
	srand((unsigned int)timeGetTime()^0xA573);
}

Procesador::~Procesador(void)
{
	if (_img) cvReleaseImage(&_img);
	if (_imgdown) cvReleaseImage(&_imgdown);
	if (_imgobjetos) cvReleaseImage(&_imgobjetos);
    if (_imgbw) cvReleaseImage(&_imgbw);
    if (_imgbw2) cvReleaseImage(&_imgbw2);
    if (_src) cvReleaseImage(&_src);
	if (_edges) cvReleaseImage(&_edges);
	if (_dst) cvReleaseImage(&_dst);
	if (_dmap) cvReleaseImage(&_dmap);
	if (_watershed) cvReleaseImage(&_watershed);
	if (_histogram) cvReleaseImage(&_histogram);
	
	while(!_v_obj.empty())
	{
		delete _v_obj.back();
		_v_obj.pop_back();
	}
}



void rellenarHuecos(IplImage* src)
{
	ImagenBn isrc(src);

	int px=-1,py=-1;
	// buscamos un pixel del borde q sea negro para usarlo de seed
	for(int xx=0; px<0 && xx<10; ++xx)
	{
		int i;
		for(i=0; i<isrc->width; ++i)
		 if (isrc[xx][i]==0) {
			px=i; py=xx; break;
		 }
		if (px<0) break;
		for(i=0; i<isrc->height; ++i)
		 if (isrc[i][xx]==0) {
			px=xx; py=i; break;
		 }
	}

	// ahora hacemos un marco negro alrededor de la imagen para conectar todos los espacios negros
	// en caso de que varios objetos partan la imagen por completo en varias partes
	for(int i=0; i<isrc->width; ++i)
	{
		isrc[0][i] = 0;
		isrc[isrc->height-1][i] = 0;
	}
	for(int j=0; j<isrc->height; ++j)
	{
		isrc[j][0] = 0;
		isrc[j][isrc->width-1] = 0;
	}

	if (px>=0)
	{
		// rellenamos de color 120 el negro externo
		cvFloodFill(isrc, cvPoint(px,py), cvScalar(120));
		// y finalmente rellenamos de blanco el negro interno y de negro otra vez el 120
		for(int j=0; j<isrc->height; ++j)
			for(int i=0; i<isrc->width; ++i)
			{
				if (isrc[j][i]==120)
					isrc[j][i] = 0;
				else
					isrc[j][i] = 255;
			}
	}
}



void Procesador::preprocesar()
{
	printf("_img->width, height = %d, %d\n", _img->width, _img->height);

	_imgbw = cvCreateImage( cvSize(_img->width, _img->height), 8, 1 );
	_imgbw2 = cvCreateImage( cvSize(Width, Height), 8, 1);
	
	IplImage* imgbwS = cvCreateImage( cvSize(_img->width, _img->height), 8, 1 );
	IplImage* imgbwS2 = cvCreateImage( cvSize(Width, Height), 8, 1 );
	
	_src = cvCreateImage( cvSize(Width, Height), 8, 1);

	ImagenRgb(_img).splitChannel('V', _imgbw);
	ImagenRgb(_img).splitChannel('S', imgbwS); // metemos saturacion? opcional
	// ademas, saturacion 'X'? opcional
	ImagenBn imgtmp = ImagenRgb(_img).splitChannel('X');
	ImagenBn(imgbwS) += imgtmp;

	// filtrar ruido
	IplImage* imgbwHalf = cvCreateImage( cvSize(_imgbw->width/2, _imgbw->height/2), 8, 1 );
	cvPyrDown( _imgbw, imgbwHalf, 7 );  // PARAM?
	cvPyrUp( imgbwHalf, _imgbw, 7 );
	cvReleaseImage(&imgbwHalf);

	cvResize( _imgbw, _imgbw2, CV_INTER_LINEAR );
	
	cvResize( imgbwS, imgbwS2, CV_INTER_LINEAR );
	cvReleaseImage(&imgbwS);


	///////// canny
	_edges = cvCreateImage( cvSize(Width, Height), 8, 1);
	cvCanny( _imgbw2, _edges, 100., 200., 3 );
    cvDilate(_edges, _edges, 0, 1);


	// smooth
	cvSmooth( _imgbw, _imgbw, CV_GAUSSIAN, _cfg.th_smooth);			// PARAM 29
	cvSmooth( imgbwS2, imgbwS2, CV_GAUSSIAN, _cfg.th_smooth);			// PARAM 29
	cvResize( _imgbw, _src, CV_INTER_LINEAR );



//	ImagenBn(_src).threshold(/*20*/ 0, 60, true);
//	ImagenBn(imgbwS2).threshold(80, 114);
	
	 // estas 3 lineas son lo añadido para trabajar con saturation, descomentarlas para pruebas
		//importante: probar con botella de agua azulada. comprobar si es ventajoso usar la saturacion del
		// azul para aislarla mejor de fondo.. si es apenas apreciable mejor usar el metodo mas simple
		// y pasar totalmente de usar la saturacion en el thresholding

	ImagenBn(imgbwS2) += ImagenBn(_imgbw2);
	cvSmooth(imgbwS2, imgbwS2, CV_GAUSSIAN, 9);
	/**/


	//ImagenBn(_src).threshold(0, _cfg.th_threshold, true);


	/// CALCULAMOS EL THRESHOLD
	int t0 = calcularHistograma(_imgbw2); // devuelve un threshold x el metodo "tradicional"
	int threshold = ImagenBn(imgbwS2).otsuThreshold(_cfg.th_adjOtsu, _edges);
//s	int threshold = ImagenBn(_src).otsuThreshold(_cfg.th_adjOtsu, _edges);
	//int threshold = (otsu+t0)/2; // FIXME de momento usamos la media entre otsu y el metodo antiguo
	//printf("otsu=%d, t0=%d, final=%d\n", otsu, t0, threshold);

	if (_cfg.th_threshold>0)
		threshold = _cfg.th_threshold;

/*	cvNamedWindow("Source isrc", 1 );
	cvMoveWindow("Source isrc", 20, 10+290);
	cvShowImage("Source isrc", _src );*/

	/// APLICAMOS EL THRESHOLD
	ImagenBn isrc(_src);
	isrc.threshold(0, threshold, true, imgbwS2);  // usamos bwS2(sat,opcional) o src?
//s	isrc.threshold(0, threshold, true, isrc);  // usamos bwS2(sat,opcional) o src?
	//cvThreshold( _src, _src, threshold/*+30*/, 255, CV_THRESH_BINARY); //auto-ajustado // PARAM 30?

	// Añadimos los bordes tambien
	isrc += _edges;
	
	/// RELLENAMOS los huecos internos (ultimo paso)
	rellenarHuecos(_src);

	/// MOSTRAMOS ALGUNAS IMAGENES PARA COMPROBAR
///quitar para el componente cOM <<!!!

/*	cvNamedWindow("SourceS2", 1 );
	cvMoveWindow("SourceS2", 20, 10+290);
	cvShowImage("SourceS2", imgbwS2 );

	cvNamedWindow("Source00", 1 );
	cvMoveWindow("Source00", 20, 10+290);
	cvShowImage("Source00", _src );
*/
	cvReleaseImage(&imgbwS2);
}



IplImage* Procesador::segmentar()
{
	ImagenBn	isrc = _src;
	ImagenBn	idst(isrc.createDistanceMap(_cfg.wsh_steps, _cfg.wsh_step, _cfg.wsh_softness));

	// para visualizar
	_dmap = idst.getClone();
	ImagenBn(_dmap).normalizar(80,254); // esto es solo para visualizar

	DWORD t0 = timeGetTime();

	// "inundamos" un nivel
	while(idst.watershed(_cfg.wsh_umbral)>0) ;				// PARAM 20 -umbral

	printf("watershed hecho en %dms\n", (int)(timeGetTime()-t0));

	ImagenBn is(_src);
	is *= idst;

	return _watershed = idst.unBind();
}


Procesador* Procesador::Cargar(const char* imgname, const Procesador::Config& cfg, bool bSegmentar)
{
	IplImage* img = NULL;

	if (!(img = cvLoadImage(imgname, 1)))
		return NULL;
	else
		return new Procesador(img, cfg, bSegmentar);
}

Procesador::Procesador(const IplImage* imagen, const Procesador::Config& cfg, bool bSegmentar, Classifier<vector<double>, int>* pClas) : _cfg(cfg),
							_img(0), _imgdown(0), _imgbw(0), _imgbw2(0), _dmap(0), _src(0),
							_dst(0), _edges(0), _watershed(0), _histogram(0), _imgobjetos(0)
{
	// The function cvQueryFrame grabs a frame from camera or video file, decompresses and returns it. 
	// This function is just a combination of cvGrabFrame and cvRetrieveFrame in one call. 
	// The returned image should not be released or modified by user. <<<< por eso el clone aqui

	if (imagen->origin==0)
	{
		_img = cvCloneImage(imagen);
	}
	else  // para corregir imagenes de webcam
	{
		_img = cvCreateImage(cvGetSize(imagen), 8, 3);
		cvFlip(imagen, _img, 0);
	}

	if (Storage==0) // es static
		Storage = cvCreateMemStorage(0);

	_imgdown = cvCreateImage( cvSize(Width, Height), 8, 3);
	// downsampling
	cvResize( _img, _imgdown, CV_INTER_LINEAR );


	///////// PRE-PROCESAR (thresholding y separacion de canales principalmente)
	preprocesar();


	///////// SEGMENTAR
	if (bSegmentar)
		segmentar();


	///////// BUSCAR LOS CONTORNOS DE LOS OBJETOS ya segmentados y crear la lista de objetos
	CvSeq* contour = 0;
 	IplImage* src2 = cvCloneImage(_src); // _src es la imagen binaria
	cvFindContours( src2, Storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
	_dst = cvCreateImage( cvGetSize(_src), 8, 3 );
	cvZero( _dst );

	_imgobjetos = cvCloneImage(_imgdown);

	for(int j=0; contour!=0; contour=contour->h_next )
	{
		CvScalar color = CV_RGB(255, 255, 255);
		
		//ImagenBn  iob_bn(cvSize(Width, Height)); //img nueva
		//ImagenRgb iob_rgb = _imgdown; //copia
		
		IplImage* i_bn = cvCreateImage(cvSize(Width, Height), 8, 1);
		cvZero(i_bn);
		IplImage* i_rgb = cvCloneImage(_imgdown);
		
		////////////////////////////////////////////////////////////////

		int count = contour->total; // This is number point in contour
        CvPoint* pointArray;
		int hullsize;
                
        // Alloc memory for contour point set.    
        pointArray = new CvPoint [count];
                
        // Alloc memory for indices of convex hull vertices.
        int *hull = new int [count];
        
        // Get contour point set.
        cvCvtSeqToArray(contour, pointArray, CV_WHOLE_SEQ);
                        
		// Find convex hull for curent contour.
        cvConvexHull( pointArray,
                      count,
                      NULL,
                      CV_COUNTER_CLOCKWISE,
                      hull,
                      &hullsize);
      
		///////////////////////////////////////////////////////////////
		cvDrawContours( i_bn, contour, color, color, -1, 2, 8 ); // outline grosor 2 para cepillarnos las grietas hacia formas concavas interiores si hubiera
		cvDrawContours( i_bn, contour, color, color, -1, CV_FILLED, 8 );

		char name[256];
		sprintf(name, "Objeto %d", ++j);
		Objeto* obj = new Objeto(i_rgb, i_bn, name, false); //true);

		// eliminamos objetos demasiado pequeños y demasiado grandes, usando los semiejes de la elipse como guia de tamaños
		//if (obj->largo_eje()*obj->ancho_eje()<560 || (obj->largo_eje()>230 && obj->ancho_eje()>230))
		if (obj->largo()*obj->ancho()<300 || (obj->largo_eje()>230 && obj->ancho_eje()>230) || obj->ancho()<=15)
		{
			// 300 es 17*17 aprox
			delete obj;
			delete [] pointArray;
			delete [] hull;
		
			continue;
		}

		if (pClas)
		{
			int find2 = pClas->find(obj->getCombinedVector());
			switch(find2)
			{
			case 1:
				color = CV_RGB( 250, 150, 0 ); // metal - naranja
				break;
			case 2:
				color = CV_RGB( 250, 250, 0 ); // plastico - amarillo
				break;
			case 3:
				color = CV_RGB( 80, 80, 250); //papel - azul
				break;
			case 4:
				color = CV_RGB( 50, 200, 0 ); // vidrio - verde
				break;
			default:
				color = CV_RGB( 150, 150, 150); // desconocido - gris medio
				break;
			};
		}
		else
			color = CV_RGB( 96|rand()&255, 96|rand()&255, 96|rand()&255 );
		
		//antes se hacia arriba del todo
		cvDrawContours( _dst, contour, color, color, -1, 2, 8); //CV_FILLED, 8 ); 
		cvDrawContours( _imgobjetos, contour, color, color, -1, 2, 8); //CV_FILLED, 8 ); 

		if (hullsize>=4)
		{
			CvScalar color2 = CV_RGB(255, 255, 255);

			for(int i=0; i<hullsize-1; i++)
			{
				cvLine(_dst, pointArray[hull[i]], 
								pointArray[hull[i+1]],color2, 0, 8);
#			ifdef RELLENAR_CONCAVIDADES
				cvLine(i_bn, pointArray[hull[i]], 
								pointArray[hull[i+1]],color2, 0, 8);
#			endif
			}
			cvLine(_dst, pointArray[hull[hullsize-1]],
								pointArray[hull[0]],color2, 0, 8);
#		ifdef RELLENAR_CONCAVIDADES
			cvLine(i_bn, pointArray[hull[hullsize-1]],
								pointArray[hull[0]],color2, 0, 8);

			rellenarHuecos(i_bn);
#		endif
		}

		delete [] pointArray;
		delete [] hull;

		_v_obj.push_back(obj);
	}

	cvReleaseImage(&src2);

	/* rotar imagen
	_imgrot = cvCreateImage(cvSize(Width, Height), 8, 3); //cvSize(_img->width/2, _img->height/2), 8, 3 );
	CvMat* map_matrix = cvCreateMat(2, 3, CV_32F);
	cv2DRotationMatrix( cvPoint2D32f(momentos.cgx(), momentos.cgy()), momentos.theta()*180./M_PI, 1., map_matrix );
	cvWarpAffine(_imgdown, _imgrot, map_matrix);
	cvReleaseMat(&map_matrix);
	*/
}



int Procesador::calcularHistograma(const IplImage* src)
{
	const IplImage* v_plane = src; //cvCreateImage( cvGetSize(src), 8, 1 );
	const IplImage* planes[] = { v_plane };

//	ImagenRgb hsv(cvGetSize(src));
//	cvCvtColor( src, hsv, CV_BGR2HSV );

	int bins = 128, s_bins = 48;
	int hist_size[] = {bins};//, s_bins};
	float hrange[] = {0, 255};
	float* ranges[] = { hrange };
	int scale = 4;

	_histogram = cvCreateImage( cvSize(bins*scale,s_bins*scale), 8, 3 );
	
	CvHistogram* hist;
	float max_value = 0, min_value = 0;
	int minmax_idx[2];
	int h;

//	cvCvtPixToPlane( hsv, 0, 0, v_plane, 0); //s_plane, v_plane, 0 );

	hist = cvCreateHist( 1, hist_size, CV_HIST_ARRAY, ranges, 1 );
	cvCalcHist( const_cast<IplImage**>(planes), hist, 0, 0 );
	cvGetMinMaxHistValue( hist, &min_value, &max_value, minmax_idx, minmax_idx+1 );
	cvZero( _histogram );

	int threshold=-1, t2=0, t0=-1;
	float v0 = 0;
	for( h = 0; h < bins; h++ )
	{
		float bin_val = cvQueryHistValue_1D( hist, h );
		
		int s = cvRound( bin_val*s_bins*scale/max_value );
		int intensity = cvRound( 256*h/bins );

		cvRectangle( _histogram, cvPoint( h*scale, s_bins*scale ),
						cvPoint( (h+1)*scale - 1, s_bins*scale-(s+1) - 1),
						CV_RGB(255-64+intensity/4,intensity,intensity),
						CV_FILLED );

		if (t2>=0 && threshold>=0 && bin_val<v0) {
			// el valor ha comenzado a bajar, estamos en el pico justo
			t2 = intensity;
			if (t0<0)
				t0 = intensity;
		}
		else
			if (bin_val>v0 && t2>0 && (bin_val*100.f/max_value)<20.f) // PARAM 20.f
			{
				t2 = -t2; // cuando vuelve a subir (hay nu pico nuevo distinto del negro de fondo) paramos
				cvRectangle( _histogram, cvPoint( h*scale, s_bins*scale ),
						cvPoint( (h+1)*scale - 1, s_bins*scale-(s+1) - 1),
						CV_RGB(64+intensity/4, 64+intensity/4, 255-64+intensity/4),
						CV_FILLED );
			}

		// calculamos donde empieza el primer pico de fondo negro
		if (threshold<0 && (bin_val*100.f/max_value)>1.f)
		{
			threshold = intensity;
			cvRectangle( _histogram, cvPoint( h*scale, s_bins*scale ),
						cvPoint( (h+1)*scale - 1, s_bins*scale-(s+1) - 1),
						CV_RGB(255-64+intensity/4, 255-64+intensity/4, 0),
						CV_FILLED );
		}

		v0 = bin_val;
	} 

	printf("idx(min,max)=(%d,%d)\n", minmax_idx[0], minmax_idx[1]);
	printf("threshold=%d\n", threshold);

	printf("<threshold0=%d, -t2=%d, t0=%d>\n", threshold, -t2, t0);

	if (-t2>100)
		t2 = -100;

	//cvReleaseImage(&v_plane);

	return -t2; //threshold;
}



const IplImage*	Procesador::getImg(int ncanal) const
{
	switch(ncanal)
	{
	case 0:
		return _dst;
	case 1:
		return _imgdown;
	case 2:
		return _imgobjetos;
/*	case 2:
		return _imgbw2;
	case 3:
		return _edges;*/
	default:
		// a partir del canal 10 son los objetos
		unsigned nobj = (unsigned)(ncanal-10);
		if (nobj>=0 && nobj<_v_obj.size())
		{
			return _v_obj[nobj]->getImg();
		}
		else
			return NULL;
	}
}


void Procesador::display()
{
	cvNamedWindow( "original", 1 );
	cvMoveWindow("original", 30, 20);
	cvShowImage( "original", _imgdown );

	cvNamedWindow( "edges", 1 );
	cvMoveWindow("edges", 20+350, 10);
	cvShowImage( "edges", _edges );

	cvNamedWindow( "B/W image", 1 );
	cvMoveWindow("B/W image", 30+350, 20);
	cvShowImage( "B/W image", _imgbw2 );

	if (_dmap!=0)
	{
		cvNamedWindow( "distmap", 1 );
		cvMoveWindow("distmap", 30+700, 20);
		cvShowImage( "distmap", _dmap );
	}
	cvNamedWindow( "Source", 1 );
	cvMoveWindow("Source", 50+700, 40); //30, 20+290);
	cvShowImage( "Source", _src );

	cvNamedWindow( "Components", 1 );
	cvMoveWindow("Components", 30, 20+290); //30+350, 20+290);
	cvShowImage( "Components", _dst );

	cvNamedWindow("V Histogram", 1 );
	cvShowImage("V Histogram", _histogram );
	cvMoveWindow("V Histogram", 30, 590);

	////_momentos->dibujarEjes(_imgdown);
	//_momentos->dibujarEjes(_imgbw2);
	cvShowImage( "original", _imgdown );
	cvShowImage( "B/W image", _imgbw2 );

	int i=0;
	for(vector<Objeto*>::iterator I=_v_obj.begin(); I!=_v_obj.end(); ++I,++i)
	{
		string str = "Fourier - ";
		str += (*I)->getName();
		cvNamedWindow(str.c_str(), 1 );
		cvMoveWindow(str.c_str(), 30+350+15*i, 20+290+20*i);
		//cvShowImage("Fourier obj1", _v_obj.front()->getFourierDescriptor().getImg());
		cvShowImage(str.c_str(), (*I)->getDesc_Fourier()->getImg());
	}
}





///////////////////////////////////////

/* funcion antigua
void Procesador::cvtLuminance(IplImage* img, IplImage* imgbw)
{
	IplImage* imgLab = cvCreateImage( cvSize(img->width, img->height), 8, 3 );
    
	cvCvtColor(img, imgLab, CV_RGB2HSV); // tambien valdria CV_RGB2Lab, pero da menos luminosidad 
										 // o la calcula 1 poco distinto; hsv da el maximo de r,g,b

	cvCvtPixToPlane( imgLab, 0, 0, imgbw, 0); //s_plane, v_plane, 0 );

/*	ImagenBn imgbwA(imgbw);
	ImagenRgb imgA(imgLab);
	
	for(int i=0;i<imgbw->height; i++) 
		for(int j=0; j<imgbw->width; j++) 
			imgbwA[i][j]= imgA[i][j].r; // r para hsv; b sacamos el canal L de lab
	//		imgbwA[i][j]= (uchar)(imgA[i][j].b*0.114 + 
    //					         imgA[i][j].g*0.587 + 
    //						     imgA[i][j].r*0.299);
* /

	cvReleaseImage( &imgLab );
}
*/