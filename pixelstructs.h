/*
 * Modulo: Vision artificial
 *
 * Descripcion: Estructuras de pixels
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007. All rights reserved.
 *
 * $Id: pixelstructs.h 22 2007-12-13 23:41:39Z Alvaro $
 */

#pragma once


typedef struct _RgbPixel
{
	unsigned char b, g, r;

	inline _RgbPixel(unsigned char __val) :
				b(__val), g(__val), r(__val)
	{ }

	static const _RgbPixel Max;
	static const _RgbPixel Min;


	inline _RgbPixel(unsigned char __b, unsigned char __g, unsigned char __r) :
				b(__b), g(__g), r(__r)
	{ }

	inline const _RgbPixel& operator+=(const _RgbPixel& p)
	{
		b += p.b;
		g += p.g;
		r += p.r;

		return *this;
	}

	inline const _RgbPixel& operator-=(const _RgbPixel& p)
	{
		b -= p.b;
		g -= p.g;
		r -= p.r;

		return *this;
	}

	inline _RgbPixel operator+(const _RgbPixel& p) const 
	{
		unsigned _b = b + p.b;
		unsigned _g = g + p.g;
		unsigned _r = r + p.r;
		
		return _RgbPixel(_b>255 ? 255 : _b, 
						 _g>255 ? 255 : _g, 
						 _r>255 ? 255 : _r);
	}

	inline _RgbPixel operator-(const _RgbPixel& p) const 
	{
		int _b = b - p.b;
		int _g = g - p.g;
		int _r = r - p.r;
		
		return _RgbPixel(_b<0 ? 0 : (unsigned)_b, 
						 _g<0 ? 0 : (unsigned)_g, 
						 _r<0 ? 0 : (unsigned)_r);
	}

	inline _RgbPixel operator*(const _RgbPixel& p) const 
	{
		return _RgbPixel(cvRound(b * p.b / 255.f), 
						 cvRound(g * p.g / 255.f), 
						 cvRound(r * p.r / 255.f));
	}


	/// b/w

	inline _RgbPixel operator+(unsigned char p) const 
	{
		unsigned _b = b + p;
		unsigned _g = g + p;
		unsigned _r = r + p;
		
		return _RgbPixel(_b>255 ? 255 : _b, 
						 _g>255 ? 255 : _g, 
						 _r>255 ? 255 : _r);
	}

	inline _RgbPixel operator-(unsigned char p) const 
	{
		int _b = b - p;
		int _g = g - p;
		int _r = r - p;
		
		return _RgbPixel(_b<0 ? 0 : (unsigned)_b, 
						 _g<0 ? 0 : (unsigned)_g, 
						 _r<0 ? 0 : (unsigned)_r);
	}

	inline _RgbPixel operator*(unsigned char p) const 
	{
		return _RgbPixel(cvRound(b * p / 255.f), 
						 cvRound(g * p / 255.f), 
						 cvRound(r * p / 255.f));
	}

	inline const _RgbPixel& operator*=(const unsigned char p) 
	{
		b = (unsigned)b*p/255;
		g = (unsigned)g*p/255;
		r = (unsigned)r*p/255;
		return *this;
	}

	inline bool operator==(const _RgbPixel& p) const
	{
		return p.b==b && p.g==g && p.r==r;
	}

	inline bool operator!=(const _RgbPixel& p) const
	{
		return p.b!=b || p.g!=g || p.r!=r;
	}

	inline unsigned char satX() const
	{
		int max = MAX(MAX(b,g),r);
		int min = MIN(MIN(b,g),r);
		
        if (max==0) return 0;

		int res = (max + (max-min)/max)*max/128;

		return res>255 ? 255 : res;
	}

} RgbPixel;


typedef struct _BwPixel
{
	unsigned char b;

	static const unsigned char Max = 255;
	static const unsigned char Min = 0;

	inline _BwPixel(unsigned char __b) : b(__b)
	{ }

	inline operator unsigned char& () {
		return b;
	}

	inline operator const unsigned char () const {
		return b;
	}

	inline unsigned char operator+=(const unsigned char p)
	{
		b = (b+p)>255 ? 255 : (b+p); 
		return b;
	}

	inline unsigned char operator-=(const unsigned char p)
	{
		b = (b-p)<0 ? 0 : (b-p);
		return b;
	}

	inline unsigned char operator+(const unsigned char p) const 
	{
		return (b+p)>255 ? 255 : (b+p); 
	}

	inline unsigned char operator-(const unsigned char p) const 
	{
		return (b-p)<0 ? 0 : (b-p);
	}

	inline unsigned char operator*(const unsigned char p) const 
	{
		return (unsigned)b*p/255;
	}

	inline unsigned char operator*=(const unsigned char p) 
	{
		b = (unsigned)b*p/255;
		return b;
	}

} BwPixel;


typedef struct _RgbPixelFloat
{
	float b, g, r;

	inline _RgbPixelFloat(float __b, float __g, float __r) :
				b(__b), g(__g), r(__r)
	{ }

	inline _RgbPixelFloat(float __val) :
				b(__val), g(__val), r(__val)
	{ }

} RgbPixelFloat;

