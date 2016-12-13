/*
 * Modulo: Vision artificial
 *
 * Descripcion: Implementacion de la clase Imagen
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 *
 * $Id: Imagen.cpp 307 2008-03-20 02:16:17Z Alvaro $
 */

#include "stdafx.h"
#include "Imagen.h"

#include <utility>
using namespace std; // para swap


const _RgbPixel RgbPixel::Max = _RgbPixel(255, 255, 255);
const _RgbPixel RgbPixel::Min = _RgbPixel(0, 0, 0);

static unsigned char mval = 0;


// forzamos que se evaluen todas las clases para que se creen aqui todos los metodos usados en otros .cpp
template class Imagen<BwPixel>;
template class Imagen<RgbPixel>;
template class Imagen<float>;


#ifdef DBG_IMG_INST
 int ImagenBn::Instances = 0;
 int ImagenRgb::Instances = 0;
 int ImagenFloatRgb::Instances = 0;
 int ImagenFloat::Instances = 0;
#endif


// constructores

template<> ImagenBn::Imagen(CvSize size) : _bWrap(false)
{
	_img = cvCreateImage(size, 8, 1);
	cvZero(_img);
#	ifdef DBG_IMG_INST
	Instances++;
#	endif
}

template<> ImagenRgb::Imagen(CvSize size) : _bWrap(false)
{
	_img = cvCreateImage(size, 8, 3);
	cvZero(_img);
#	ifdef DBG_IMG_INST
	Instances++;
#	endif
}

template<> ImagenFloat::Imagen(CvSize size) : _bWrap(false)
{
	_img = cvCreateImage(size, 32, 1);
	cvZero(_img);
#	ifdef DBG_IMG_INST
	Instances++;
#	endif
}

template<> ImagenFloatRgb::Imagen(CvSize size) : _bWrap(false)
{
	_img = cvCreateImage(size, 32, 3);
	cvZero(_img);
#	ifdef DBG_IMG_INST
	Instances++;
#	endif
}

template<class T>  Imagen<T>::Imagen(CvSize size) : _bWrap(true), _img(0)
{
	assert(0);
}

///// operadores acceso/asignacion/etc

// Mantiene una copia
template<class T> void Imagen<T>::operator=(const IplImage* img) 
{ 
	if (_img && !_bWrap) 
		cvReleaseImage(&_img);
	
	_bWrap = false;
	_img = cvCloneImage(img);
}

// Se apropia de la imagen
template<class T> void Imagen<T>::operator=(const Imagen<T>& img) 
{ 
	if (_img && !_bWrap) 
		cvReleaseImage(&_img);

#	ifdef DBG_IMG_INST
	Instances--;
#	endif
	
	_bWrap = false;
	const_cast<Imagen<T>&>(img)._bWrap = true;  // cuidado con hacer estas cosas..
	_img = img._img;
}


template<class T> inline IplImage* Imagen<T>::operator -> ()
{
	return _img;
}
	
template<class T> inline const IplImage* Imagen<T>::operator -> () const
{
	return _img;
}

// Acceso a pixels de la imagen (const)
template<class T> inline const T* Imagen<T>::operator[]( int rowIndx) const
{
	return ((T *)(_img->imageData + rowIndx*_img->widthStep));
}

// Acceso a pixels de la imagen
template<class T> inline T* Imagen<T>::operator[]( int rowIndx)
{
	return ((T *)(_img->imageData + rowIndx*_img->widthStep));
}

template<class T> inline Imagen<T>::operator IplImage *()
{
	return _img;
}

template<class T> inline Imagen<T>::operator IplImage const *() const
{
	return _img;
}


/////////////////////////////


template<> double Imagen<BwPixel>::buscar_borde_en_linea(int x0, int y0, int x1, int y1) const
{
	int xini=-1, yini=-1, xfin=-1, yfin=-1;
	unsigned char lastpix = 0;

	// sigue la linea con el algoritmo de Bresenham. encuentra los dos primeros bordes y 
	// devuelve el grosor entre uno y otro

	bool steep = abs(y1-y0) > abs(x1-x0);

	if (steep) 
	{
		swap(x0, y0);
		swap(x1, y1);
	}
	
	if (x0 > x1) 
	{
		swap(x0, x1);
		swap(y0, y1);
	}

	int deltax = x1-x0;
	int deltay = abs(y1-y0);
	int error = -(deltax+1)/2;
	int ystep;
	int y = y0;

	if (y0<y1) 
		ystep = 1;
	else
		ystep = -1;

	for(int x=x0; x<=x1; ++x)
	{
		unsigned char pix = 0;
		if (steep) {
			//plot(y,x);
			if (x<0 || y<0 || x>=_img->height || y>=_img->width)
				pix = 0;
			else
				pix = (const unsigned char)((*this)[x][y]);
		} else {
			//plot(x,y);
			//estos x/y son correctos. aqui van al derecho antes arriba al reves
			if (x<0 || y<0 || y>=_img->height || x>=_img->width)
				pix = 0;
			else
				pix = (const unsigned char)((*this)[y][x]);
		}

		if (pix>0 && lastpix==0)
		{
			// primer borde, guardamos x,y iniciales
			xini = x;
			yini = y;
		}
		else if (pix==0 && lastpix>0)
		{
			// segundo borde, guardamos y salimos
			xfin = x;
			yfin = y;

			break;
		}
		lastpix = pix;

		error += deltay;

		if (error>=0)
		{
			y += ystep;
			error -= deltax;
		}
	}

	if (xini<0 || yini<0 || xfin<0 || yfin<0)
		return 0;

	if (steep) // no es necesario xq estamos hallando distancia, pero bueno asi la x es x y la y es y
	{
		swap(xini, yini);
		swap(xfin, yfin);
	}

	// distancia del borde de entrada al objeto hasta el borde de salida segun la linea
	return sqrt((double)( (xfin-xini)*(xfin-xini) + (yfin-yini)*(yfin-yini) ));
}

template<class T> double Imagen<T>::buscar_borde_en_linea(int x0, int y0, int x1, int y1) const
{
	// solo vale para imagenes binarias
	return 0;
}


// especializacion para ImagenBn
template<> void Imagen<BwPixel>::normalizar(int minimo, int maximo)
{
	uchar min=255, max=0;

	int i;

	for(i=0;i<_img->height; i++) 
	{
		for(int j=0; j<_img->width; j++) 
		{
			unsigned char val = (*this)[i][j];
			if (val>0)
			{
				if (val>max)
					max=val;
				else
					if(val<min)
						min = val;
			}
		}
	}

	for(i=0;i<_img->height; i++) 
	{
		for(int j=0; j<_img->width; j++) 
		{
			unsigned char val = (*this)[i][j];
			if (val>0 && max-min!=0)
			{
				(*this)[i][j] = minimo + (val-min)*(maximo-minimo)/(max-min);
			}
		}
	}

}

template<class T> void Imagen<T>::normalizar(int minimo, int maximo)
{
	assert(0);
	// TO-DO
}



template<> IplImage* Imagen<RgbPixel>::splitChannel(const char channel, IplImage* dest) const
{
	IplImage* imgHSV = 0;
	
	if (channel=='H' || channel=='S' || channel=='V')
	{
		imgHSV = cvCreateImage( cvSize(_img->width, _img->height), 8, 3 );
		cvCvtColor(_img, imgHSV, CV_RGB2HSV); // tambien valdria CV_RGB2Lab, pero da menos luminosidad 
												// o la calcula 1 poco distinto; hsv da el maximo de r,g,b
	}
	else
		imgHSV = _img;

	if (dest==0)
		dest = cvCreateImage(cvGetSize(_img), 8, 1);

	switch(channel)
	{
	case 'V':
	case 'R':
		cvCvtPixToPlane( imgHSV, 0, 0, dest, 0);
		break;
	case 'S':
	case 'G':
		cvCvtPixToPlane( imgHSV, 0, dest, 0, 0);
		break;
	case 'H':
	case 'B':
		cvCvtPixToPlane( imgHSV, dest, 0, 0, 0);
		break;
	case 'X':
		{	
			// formula alternativa para saturacion (ver pixelstructs.h, RgbPixel::satX())
			ImagenRgb	img(imgHSV);
			ImagenBn	dst(dest);

			for(int i=0; i<dest->height; ++i)
				for(int j=0; j<dest->width; ++j)
					dst[i][j] = img[i][j].satX();
		}
		break;
	default:
		cvZero(dest);
		break;
	}

	if (imgHSV!=0 && imgHSV!=_img)
		cvReleaseImage(&imgHSV);

	return dest;
}


template<class T> IplImage* Imagen<T>::splitChannel(const char channel, IplImage* dest) const
{
	return dest;
}


IplImage*	Imagen<BwPixel>::createDistanceMap(const int steps, const int step, const int softness) const
{
	ImagenBn	idst = *this; //copia //cvSize(_src->width, _src->height));
	ImagenBn	ierode = *this;

	idst.invertir(true);

	DWORD t0 = timeGetTime();

	int i;
	for ( i=0; i<steps && !ierode.empty(); ++i)		// PARAM steps 30
	{
		cvErode(ierode, ierode, 0, step);			// PARAM step 2
		idst += ierode - (255 - step); //era -254; ahora he puesto -(255-step) para poder con el smooth sacar tonos intermedios
	}

	printf("distmap hecho en %dms\n", (int)(timeGetTime()-t0));


	cvSmooth(idst, idst, CV_GAUSSIAN, softness|1);	// PARAM softness 25
	idst.normalizar(1,255);

	return idst.unBind();
}

template<class T> IplImage*	Imagen<T>::createDistanceMap(const int steps, const int step, const int softness) const
{
	assert(0);
	return NULL;
}


template <>	void Imagen<float>::invertir(bool bIgnore0)
{
	for(int i=0;i<_img->height; i++) 
		for(int j=0; j<_img->width; j++) 
			(*this)[i][j] = -(*this)[i][j];
}


template <class T>	void Imagen<T>::invertir(bool bIgnore0)
{
	for(int i=0;i<_img->height; i++) 
		for(int j=0; j<_img->width; j++) 
			if ((*this)[i][j]!=T::Min)
				(*this)[i][j] = T::Max - (*this)[i][j];
}



template <class T>	bool Imagen<T>::empty() const
{
	for(int i=0;i<_img->height; i++) 
		for(int j=0; j<_img->width; j++) 
			if ((*this)[i][j] != T(0))
				return false;

	return true;
}

//////////////////

template <>	int Imagen<BwPixel>::watershed(unsigned char umbralObjeto)
{
	// no es reentrant, mval es static de este archivo

	unsigned char val = 0;
	int i,j;

	mval = 0;
	// calculamos el valor maximo (blanco)
	for(i=0;i<_img->height; i++) 
		for(j=0; j<_img->width; j++) 
			if ((*this)[i][j]>mval)
				mval = (*this)[i][j];
	
	if (mval==0) 
		return 0;

	// calculamos el valor maximo que toca rellenar (siguiente a mval)
	for(i=0;i<_img->height; i++) 
		for(j=0; j<_img->width; j++) 
			if ((*this)[i][j]>val && (*this)[i][j]<mval)
				val = (*this)[i][j];

	if (val==0) return val;

	
	int suma;
	do
	{
		Imagen<BwPixel> limpia = *this; // crea copia

		suma = 0;
		for(i=0; i<_img->height; i++) 
			for(j=0; j<_img->width; j++)
			{
				// rellenamos los pixels de valor val que sean adyacentes a pixels blancos
				// repetimos hasta que no quede ninguno o hasta que los que queden no sean rellenables
				if ((*this)[i][j]==mval)
					suma += watershedBlanco(limpia, i, j, val);
			}

	}
	while(suma>0);

	
	// rellenamos los pixels de valor val que no sean ya blancos porque estan aislados,
	// siempre que superen el umbral para marcar objetos nuevos
	for(i=0; i<_img->height; i++) 
		for(j=0; j<_img->width; j++) 
			if ((*this)[i][j]==val)
			{
				if (val>=umbralObjeto)
					(*this)[i][j] = mval;
				else
					(*this)[i][j] = val-1;
			}

	return val;
}

template <class T>	int Imagen<T>::watershed(unsigned char umbralObjeto)
{
	assert(0);
	return 0;
}


template<> inline int Imagen<BwPixel>::watershedBlanco(const Imagen<BwPixel>& limpia, int i, int j, unsigned char val)
{
	int suma=0;

	// hay que tener cuidado de no acceder fuera del rango i,j - height,width
	
	if (i<_img->height-1 && j<_img->width-1 && (*this)[i+1][j+1]==val)
		suma += watershedpixNew(limpia, i+1, j+1);
	if (i<_img->height-1 && (*this)[i+1][j]==val)
		suma += watershedpixNew(limpia, i+1, j);
	if (i<_img->height-1 && j>0 && (*this)[i+1][j-1]==val)
		suma += watershedpixNew(limpia, i+1, j-1);

	if (j<_img->width-1 && (*this)[i][j+1]==val)
		suma += watershedpixNew(limpia, i, j+1);
	if (j>0 && (*this)[i][j-1]==val)
		suma += watershedpixNew(limpia, i, j-1);

	// este nunca lo tuve puesto
	//	suma += watershedpixNew(limpia, i-1, j+1);

	// este parece no hacer falta
	//	suma += watershedpixNew(limpia, i-1, j);

	if (i>0 && j>0 && (*this)[i-1][j-1]==val)
		suma += watershedpixNew(limpia, i-1, j-1);

	return suma;
}

template<class T> inline int Imagen<T>::watershedBlanco(const Imagen<BwPixel>& limpia, int i, int j, unsigned char val)
{
	return 0;
}


template<> inline int Imagen<BwPixel>::watershedpixNew(const Imagen<BwPixel>& limpia, int i, int j)
{
	// la version no opt junto con wshed_sefunde esta en frozen/3opt-back

	if (i<1 || j<1 || i>=_img->height-1 || j>=_img->width-1)
		return 0;   // blanco nos manda pixels 1 fuera del limite a veces, evitamos escribir en ellos
					// y ya puestos ahora tambien leer... que sigue petando el tema a veces y no se xq
					// con esto tal y como esta AUN no ha petado pero hay q estar atento.

	// si no tenemos adyacente un pixel blanco nos piramos directamente
	if (limpia[i-1][j-1]==mval || limpia[i][j-1]==mval || limpia[i+1][j-1]==mval ||
		limpia[i+1][j]==mval || limpia[i+1][j+1]==mval || limpia[i][j+1]==mval ||
		limpia[i-1][j+1]==mval || limpia[i-1][j]==mval)
	{
		int at[8];

		// los pixels que rodean a i,j recorridos en circulo
		at[0] = (*this)[i-1][j-1];
		at[1] = (*this)[i][j-1];
		at[2] = (*this)[i+1][j-1];

		at[3] = (*this)[i+1][j];
		at[4] = (*this)[i+1][j+1];
		
		at[5] = (*this)[i][j+1];
		at[6] = (*this)[i-1][j+1];

		at[7] = (*this)[i-1][j];


		// contamos los conjuntos de pixles blancos de at, si son mas de 2 significa que el colocar
		// un elemento central provocaria unirlos (en ese caso ponemos un pixel negro)
		int nc_at=0;

		// contamos los de at antes de meter elemento central
		for(int ii=0; ii<8 ; ii++)
		{
			if (at[ii]==mval && at[(ii+1)&7]!=mval)
			{
				if ((ii&1) && at[(ii+2)&7]==mval)
					ii++;
				else
					nc_at++;
			}

			if (nc_at>1)
			{
				(*this)[i][j] = 0;
				return 0;
			}
		}

		(*this)[i][j] = mval;
		return 1;
	}
	else
		return 0;
}

template<class T> inline int Imagen<T>::watershedpixNew(const Imagen<BwPixel>& limpia, int i, int j)
{
	return 0;
}


/////////////////////////////////
/// operadores aritmeticos, etc

// Suma de los pixels
template<class T> Imagen<T> Imagen<T>::operator +(const Imagen<T>& img2) const
{
	Imagen<T> imret(cvSize(_img->width, _img->height));

	for(int i=0;i<imret->height; i++) 
		for(int j=0; j<imret->width; j++) 
			imret[i][j] = (*this)[i][j] + img2[i][j];

	//imret._bWrap = true;
	return imret;
}


// Suma de los pixels + valor
template<class T> Imagen<T> Imagen<T>::operator +(const T& val) const
{
	Imagen<T> imret(cvSize(_img->width, _img->height));

	for(int i=0;i<imret->height; i++) 
		for(int j=0; j<imret->width; j++) 
			imret[i][j] = (*this)[i][j] + val;

	//imret._bWrap = true;
	return imret;
}


// Resta de los pixels - valor
template<class T> Imagen<T> Imagen<T>::operator -(const T& val) const
{
	Imagen<T> imret(cvSize(_img->width, _img->height));

	for(int i=0;i<imret->height; i++) 
		for(int j=0; j<imret->width; j++) 
			imret[i][j] = (*this)[i][j] - val;

	//imret._bWrap = true;
	return imret;
}

// Multiplicacion de los pixels * valor
template<class T> Imagen<T> Imagen<T>::operator *(const T& val) const
{
	Imagen<T> imret(cvSize(_img->width, _img->height));

	for(int i=0;i<imret->height; i++) 
		for(int j=0; j<imret->width; j++) 
			imret[i][j] = (*this)[i][j] * val;

	//imret._bWrap = true;
	return imret;
}

// Division de los pixels / valor
/*template<class T> Imagen<T> Imagen<T>::operator /(const T& val) const
{
	Imagen<T> imret(cvSize(_img->width, _img->height));

	for(int i=0;i<imret->height; i++) 
		for(int j=0; j<imret->width; j++) 
			imret[i][j] = (*this)[i][j] / val;

	//imret._bWrap = true;
	return imret;
}*/


// Resta de los pixels
template<class T> Imagen<T> Imagen<T>::operator -(const Imagen<T>& img2) const
{
	Imagen<T> imret(cvSize(_img->width, _img->height));

	for(int i=0;i<imret->height; i++) 
		for(int j=0; j<imret->width; j++) 
			imret[i][j] = (*this)[i][j] - img2[i][j];

	//imret._bWrap = true;
	return imret;
}

// Multiplicacion de los pixels
template<class T> Imagen<T> Imagen<T>::operator *(const Imagen<T>& img2) const
{
	Imagen<T> imret(cvSize(_img->width, _img->height));

	for(int i=0;i<imret->height; i++) 
		for(int j=0; j<imret->width; j++) 
			imret[i][j] = (*this)[i][j] * img2[i][j];

	//imret._bWrap = true;
	return imret;
}

// Suma y asignacion
template<class T> const Imagen<T>& Imagen<T>::operator +=(const Imagen<T>& img2)
{
	for(int i=0;i<_img->height; i++) 
		for(int j=0; j<_img->width; j++) 
			(*this)[i][j] += img2[i][j];

	return *this;
}

template<class T> inline const Imagen<T>& Imagen<T>::operator *=(const Imagen<BwPixel>& img2)
{
	for(int i=0;i<_img->height; i++) 
		for(int j=0; j<_img->width; j++) 
			(*this)[i][j] *= img2[i][j];

	return *this;
}

// Resta y asignacion
template<class T> const Imagen<T>& Imagen<T>::operator -=(const Imagen<T>& img2)
{
	for(int i=0;i<_img->height; i++) 
		for(int j=0; j<_img->width; j++) 
			(*this)[i][j] -= img2[i][j];

	return *this;
}

// Multiplicacion y asignacion
/*template<class T> const Imagen<T>& Imagen<T>::operator *=(const Imagen<T>& img2)
{
	for(int i=0;i<_img->height; i++) 
		for(int j=0; j<_img->width; j++) 
			(*this)[i][j] *= img2[i][j];

	return *this;
}*/



///////////////////////////
// otras funciones


template<> int Imagen<BwPixel>::otsuThreshold(double ajuste, const IplImage* imask) const
{
	// el funcionamiento normal esta probado, con mask parece ok pero habria q asegurarse
	int nb=0, no=_img->height*_img->width;
	double ub=0., uo=0.;
	const ImagenBn mask(const_cast<IplImage*>(imask)); // wrap solo (x eso quito el const)

#ifdef _DEBUG
	if (imask)
		assert(_img->height==imask->height && _img->width==imask->width);
#endif

	int threshold[256];
	int t;
	for(t=0; t<256; ++t)
		threshold[t] = 0;

	if (imask)
	{
		for(int i=0;i<_img->height; i++) 
		{
			for(int j=0; j<_img->width; j++) 
			{
				if (mask[i][j]!=0) // si esta en la mask, ok
				{
					threshold[(*this)[i][j]]++;
					uo += (*this)[i][j];
				}
				else
					no--; // si no esta, lo quitamos de la cuenta inicial total
			}
		}
	}
	else
	{
		for(int i=0;i<_img->height; i++) 
		{
			for(int j=0; j<_img->width; j++) 
			{
				threshold[(*this)[i][j]]++;
				uo += (*this)[i][j];
			}
		}
	}

	nb+=threshold[0];
	no-=threshold[0];

	if (no==0.) uo=0.; else	uo /= no;
	ub = 0.;

	double sigma2[256];
	sigma2[0]=0.;
	double sigma2max = -1.;
	int tmax = 0;
	for(t=1; t<256; ++t)
	{	
		if (threshold[t]==0)
			continue;

		if (no==0)
			break;

		int nbt0=nb, not0=no;
		nb += threshold[t];
		no -= threshold[t];
		
		if (nb==0) ub=0.; 
		else 
			ub = (ub*(double)nbt0 + (double)threshold[t]*t) / (double)nb;
		
		if (no==0) uo=0.; 
		else
			uo = (double)(uo*not0 - threshold[t]*t) / (double)no;

		sigma2[t] = (ub-uo)*(ub-uo) * (double)(nb*no);
		if (sigma2[t]>sigma2max)
		{
			sigma2max = sigma2[t];
			tmax = t;
		}

		// printf("th[%3d]=%8d sigma2[%3d]=%16.1f\n", t, threshold[t], t, (float)sigma2[t]);
	}

    // ajuste indica el porcentaje por debajo del sigma^2 maximo que queremos establecer como umbral
	// si ajuste>=0 se coge la solucion superior al maximo sino, la inferior
	int signo = (ajuste>=0) ? 1 : -1;
	double ajuste1 = (1-abs(ajuste)/100.);
	for(t=tmax; t<256 && t>=0; t+=signo)
		if (sigma2max*ajuste1 >= sigma2[t])
			break;

	printf("t=%d; tmax=%d, ajuste=%.2f\n", t, tmax, ajuste);

	return t;
}

template<class T> int Imagen<T>::otsuThreshold(double ajuste, const IplImage* imask) const
{
	return 0;
}



template<> int Imagen<BwPixel>::calcStats(int& min, int& max, const int percent, bool sat) const
{
	int media=0;
	int n[256];
	int total = _img->height*_img->width;
	
	int i;

	for(i=0; i<256; ++i)
		n[i] = 0;

	for(i=0;i<_img->height; i++) {
		for(int j=0; j<_img->width; j++) {
			if (sat && ((*this)[i][j]==255 || (*this)[i][j]==0)) {
				--total;
			} else {
				media += (*this)[i][j];
				++n[(*this)[i][j]];
			}
		}
	}

	if (total==0)
		return min=max=0;

	media /= total;

	int total_p = total*percent/100;
	int acumulado = n[media];
	for(i=1; acumulado<total_p && i<256; ++i)
	{
		if (media-i<0 && media+i>=256)
			break;
		if (media-i>=0)
			acumulado += n[media-i];
		if (media+i<256)
			acumulado += n[media+i];
	}

	max = media+i;
	min = media-i;

	if (max>=256) max=255;
	if (min<0) min=0;

	return max;
}

template<class T> int Imagen<T>::calcStats(int& min, int& max, const int percent, bool sat) const
{
	return 0;
}

template<> void Imagen<BwPixel>::threshold(int low, int high, bool binary, IplImage* origen)
{
	if (origen==0)
		origen = _img;

	ImagenBn iorigen(origen); //wrap

	for(int i=0;i<_img->height; i++) 
	{
		for(int j=0; j<_img->width; j++) 
		{
			int val = iorigen[i][j];
			if (val<low || val>high)
			{
				if (binary)
					(*this)[i][j] = 255;
				else if (val<255)
					++((*this)[i][j]);
			}
			else
				(*this)[i][j] = 0;
		}
	}
}

template<class T> void Imagen<T>::threshold(int low, int high, bool binary, IplImage* origen)
{
}


template<class T> void Imagen<T>::fillBitmapInfo( BITMAPINFO* bmi, int width, int height, int bpp, int origin ) const
{
    assert( bmi && width >= 0 && height >= 0 && (bpp == 8 || bpp == 24 || bpp == 32));

    BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);

    memset( bmih, 0, sizeof(*bmih));
    bmih->biSize = sizeof(BITMAPINFOHEADER);
    bmih->biWidth = width;
    bmih->biHeight = origin ? abs(height) : -abs(height);
    bmih->biPlanes = 1;
    bmih->biBitCount = (unsigned short)bpp;
    bmih->biCompression = BI_RGB;

    if( bpp == 8 )
    {
        RGBQUAD* palette = bmi->bmiColors;
        int i;
        for( i = 0; i < 256; i++ )
        {
            palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
            palette[i].rgbReserved = 0;
        }
    }
}

template<class T> void Imagen<T>::paint(HDC hdc, const RECT& rc, int from_x, int from_y) const
{
	paint(hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, from_x, from_y);
}

template<class T> void Imagen<T>::paint(HDC hdc, int x, int y, int w, int h, int from_x, int from_y) const
{	
	// basado en CvvImage::Show
    if( _img && _img->depth == IPL_DEPTH_8U )
    {
        uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
        BITMAPINFO* bmi = (BITMAPINFO*)buffer;
        int bmp_w = _img->width, bmp_h = _img->height;

        fillBitmapInfo( bmi, bmp_w, bmp_h, _img->nChannels*8, _img->origin );

        from_x = MIN( MAX( from_x, 0 ), bmp_w - 1 );
        from_y = MIN( MAX( from_y, 0 ), bmp_h - 1 );

        int sw = MAX( MIN( bmp_w - from_x, w ), 0 );
        int sh = MAX( MIN( bmp_h - from_y, h ), 0 );

        /*SetDIBitsToDevice(
              hdc, x, y, sw, sh, from_x, from_y, from_y, sh,
              _img->imageData + from_y*_img->widthStep,
              bmi, DIB_RGB_COLORS );*/
		
		// After setting the HALFTONE stretching mode, an application must call the SetBrushOrgEx function
		// to set the brush origin. If it fails to do so, brush misalignment occurs.
		SetStretchBltMode(hdc, HALFTONE);

		POINT pt;
		SetBrushOrgEx(hdc, 0, 0, &pt);

		int res = StretchDIBits(
					  hdc, //dc
					  x, //x dest
					  y, //y dest
					  w, //x dest dims
					  h, //y dest dims
					  from_x, // src x
					  from_y, // src y
					  bmp_w, // src dims
					  bmp_h, // src dims
					  _img->imageData, // array of DIB bits
					  (BITMAPINFO*)bmi, // bitmap information
					  DIB_RGB_COLORS, // RGB or palette indexes
					  SRCCOPY); // raster operation code


		return;
    }

/*		//otro metodo que funciona

		BITMAPINFO* bmi;
		BITMAPINFOHEADER* bmih;
		RGBQUAD* palette;
		unsigned int buffer[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256]; 

			  //Initialize the BMP display buffer
		bmi = (BITMAPINFO*)buffer;
		bmih = &(bmi->bmiHeader);
		memset( bmih, 0, sizeof(*bmih));
		bmih->biSize = sizeof(BITMAPINFOHEADER);
		bmih->biWidth = m_canales[channel]->width;
		bmih->biHeight = m_canales[channel]->height;
		bmih->biPlanes = 1;
		bmih->biCompression = BI_RGB;
		bmih->biBitCount = 8 * m_canales[channel]->nChannels; //TheImage->nChannels;
		palette = bmi->bmiColors;

		 int res = StretchDIBits(
					  hdcDraw, //dc
					  0, //x dest
					  0, //y dest
					  rc.right-rc.left, //x dest dims
					  rc.bottom-rc.top, //y dest dims
					  0, // src x
					  0, // src y
					  m_canales[channel]->width, // src dims
					  m_canales[channel]->height, // src dims
					  m_canales[channel]->imageData, // array of DIB bits
					  (BITMAPINFO*)bmi, // bitmap information
					  DIB_RGB_COLORS, // RGB or palette indexes
					  SRCCOPY); // raster operation code
		 /**/


	assert(0);
}

/*
template<> void Imagen<_RgbPixel>::paint( HDC hDCDst, RECT* pDstRect ) const
{
	// basado en CImage::DrawToHDC
    if( pDstRect && _img && _img->depth == IPL_DEPTH_8U && _img->imageData )
    {
        uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
        BITMAPINFO* bmi = (BITMAPINFO*)buffer;
        int bmp_w = _img->width, bmp_h = _img->height;

        CvRect roi = cvGetImageROI( _img );
        CvRect dst = RectToCvRect( *pDstRect );

        if( roi.width == dst.width && roi.height == dst.height )
        {
            Show( hDCDst, dst.x, dst.y, dst.width, dst.height, roi.x, roi.y );
            return;
        }
    
        if( roi.width > dst.width )
        {
            SetStretchBltMode(
                   hDCDst,           // handle to device context
                   HALFTONE );
        }
        else
        {
            SetStretchBltMode(
                   hDCDst,           // handle to device context
                   COLORONCOLOR );
        }

        FillBitmapInfo( bmi, bmp_w, bmp_h, Bpp(), _img->origin );

        ::StretchDIBits(
            hDCDst,
            dst.x, dst.y, dst.width, dst.height,
            roi.x, roi.y, roi.width, roi.height,
            _img->imageData, bmi, DIB_RGB_COLORS, SRCCOPY );
    }
}

*/

	/*
	class Buffer5x5 
	{
	public:
		inline Buffer5x5(int icentral, int jcentral, const Imagen<BwPixel>&limpia) : _icentral(icentral),
																				  _jcentral(jcentral)
		{
			for(int i=-2; i<=2; i++)
				for(int j=-2; j<=2; j++)
					if (icentral+i<0 || icentral+i>limpia->height ||
						jcentral+j<0 || jcentral+j>limpia->width)
					  _buffer[i+2][j+2] = 0;
					else
					  _buffer[i+2][j+2] = limpia[icentral+i][jcentral+j];
		}

		inline const unsigned char operator ()(const int i, const int j) const
		{
			if (i-_icentral+2<0 || i-_icentral+2>4 || j-_jcentral+2<0 || j-_jcentral+2>4)
				return 0;
			else
				return _buffer[i-_icentral+2][j-_jcentral+2];
		}
	private:
		Buffer5x5();

		int _icentral, _jcentral;
		unsigned char _buffer[5][5];
	};*/


