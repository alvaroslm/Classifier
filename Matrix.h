/*
 * Modulo: Vision artificial
 *
 * Descripcion: Templates de matrices y vectores
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007. All rights reserved.
 *
 * $Id: Matrix.h 391 2008-04-04 18:09:59Z Alvaro $
 */

#pragma once

#include <stdio.h>
#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>
#include <string>

using namespace std;

template <typename T> class MatrixT; 


const char PRINTF_STR[] = "%8.4f ";  //"%18.16f ";


// Excepciones que lanzaran las matrices
class MatrixException
{
public:
	inline MatrixException(const char* str, int err=0) : _str(str), _err(err) {
	}

	inline MatrixException(int err) : _str(""), _err(err) {
	}

	inline const char* getMessage() const {
		return _str.c_str();
	}

	inline int getNum() const {
		return _err;
	}

protected:
	string	_str;
	int		_err;

private:
	MatrixException() {}
};


/////////////////////////////////////////////////////////////////////////////////


// Template de vector de tipo T de N dimensiones
template <typename T> class VectorT 
{
#ifndef _DEBUG
	#pragma optimize( "wgt", on )
	#pragma inline_depth( 255 )
#endif

public:
	const int N;

	inline VectorT() : N(0) 
	{
	}

	inline VectorT(int n) : N(n)
	{
		_vector = new T [N];
		clear();
	}

	inline void clear()
	{
		for(int i=0; i<N; ++i)
			_vector[i] = 0.;
	}

	explicit inline VectorT(int n, bool noInit) : N(n)
	{
		_vector = new T [N];
	}

	inline VectorT(int n, const T* vector) : N(n)
	{
		_vector = new T [N];
		for(int i=0; i<N; ++i)
			_vector[i] = vector[i];
	}

	inline VectorT(const VectorT<T>& m) : N(m.N)
	{
		_vector = new T [N];
		for(int i=0; i<N; ++i)
				_vector[i] = m._vector[i];
	}

	// Podemos forzar explicitamente que copie un vector de dimension distinta recortandolo o rellenando con 0s
	/*template <int N2> explicit inline VectorT(const VectorT<T,N2>& m)
	{
		int i=0;

		for(; i<N && i<N2; ++i)
				_vector[i] = m[i];
		
		for(; i<N; ++i)
			_vector[i] = 0.;
	}*/

	/*inline VectorT(const vector<T>& v)
	{
		for(unsigned i=0; i<v.size() && i<N; i++)
			_vector[i] = v[i];
		
		for(; i<N; ++i)
			_vector[i] = 0.;
	}*/

	inline VectorT(const vector<T>& v) : N((int)v.size())
	{
		_vector = new T [N];
		
		for(int i=0; i<N; ++i)
			_vector[i] = v[i];
	}

	inline const VectorT<T>& operator =(const VectorT<T>& v)
	{
		if (v.N!=N)
#	ifdef _DEBUG
			throw MatrixException("VectorT::operator =: el tamaño del vector no concuerda");
#	else
		{
			const_cast<int&>(N) = v.N;
			delete [] _vector;

			_vector = new T [N];
		}
#	endif
		for(int i=0; i<N; ++i)
			_vector[i] = v._vector[i];

		return *this;
	}

	inline ~VectorT() 
	{
		delete [] _vector;
	}

	void print(const char* pre=0) const
	{
		if (pre)
			printf("%s ", pre);

		for(int i=0; i<N; ++i)
			printf(PRINTF_STR, (float)_vector[i]);

		printf("\n");
	}

	inline const T operator[](const int i) const
	{
#	ifdef _DEBUG
		if (i>=N)
			throw MatrixException("VectorT::operator[]: Range check failed");
#	endif
		return _vector[i];
	}

	inline T& operator[](const int i)
	{
#	ifdef _DEBUG
		if (i>=N)
			throw MatrixException("VectorT::operator[]: Range check failed");
#	endif
		return _vector[i];
	}

	inline T norma2() const
	{
		T norma = 0.;
		
		for(int i=0; i<N; ++i)
			norma += _vector[i]*_vector[i];
		
		return norma;
	}

	inline T norma() const
	{
		return sqrt(norma2());
	}

	// Producto escalar (o de matriz fila x matriz columna)
	inline T operator *(const VectorT<T>& v) const
	{
#	ifdef _DEBUG
		if (v.N!=N)
			throw MatrixException("VectorT::operator *: Vectores de distinto tamaño");
#	endif
		T ret = 0.;

		for(int i=0; i<N; ++i)
			ret += _vector[i] * v[i];

		return ret;
	}

	// Producto por un escalar
	inline VectorT<T> operator *(const T val) const
	{
		VectorT<T> ret(N,true);

		for(int i=0; i<N; ++i)
			ret[i] = _vector[i] * val;

		return ret;
	}

	// Matriz fila por matriz NxN
	inline VectorT<T> operator *(const MatrixT<T>& m) const
	{
#	ifdef _DEBUG
		if (m.N!=N)
			throw MatrixException("VectorT::operator *(MatrixT): Matriz de tamaño inapropiado");
#	endif
		VectorT<T> ret(N);

		for(int j=0; j<N; ++j)
			for(int i=0; i<N; ++i)
				ret._vector[j] += _vector[i] * m[j][i];

		return ret;
	}

	inline const VectorT<T>& operator +=(const VectorT<T>& v)
	{
#	ifdef _DEBUG
		if (v.N!=N)
			throw MatrixException("VectorT::operator +=: Vectores de distinto tamaño");

		for(int i=0; i<N && i<v.N; ++i)
#	else
		//for(int i=0; i<N; ++i)
		for(int i=0; i<N && i<v.N; ++i) // suma solo lo que cabe
#	endif
			_vector[i] += v[i];

		return *this;
	}

	inline const VectorT<T>& operator -=(const VectorT<T>& v)
	{
#	ifdef _DEBUG
		if (v.N!=N)
			throw MatrixException("VectorT::operator -=: Vectores de distinto tamaño");

		for(int i=0; i<N && i<v.N; ++i)
#	else
		//for(int i=0; i<N; ++i)
		for(int i=0; i<N && i<v.N; ++i) // resta solo lo que "cabe"
#	endif
			_vector[i] -= v[i];

		return *this;
	}

	inline const VectorT<T>& operator /=(const T val)
	{
		for(int i=0; i<N; ++i)
			_vector[i] /= val;

		return *this;
	}

	inline VectorT<T> operator +(const VectorT<T>& v) const
	{
#	ifdef _DEBUG
		if (v.N!=N)
			throw MatrixException("VectorT::operator +: Vectores de distinto tamaño");
#	endif
		VectorT<T> res(N,true);

		for(int i=0; i<N; ++i)
			res[i] = _vector[i] + v[i];

		return res;
	}

	inline VectorT<T> operator -(const VectorT<T>& v) const
	{
#	ifdef _DEBUG
		if (v.N!=N)
			throw MatrixException("VectorT::operator -: Vectores de distinto tamaño");
#	endif
		VectorT<T> res(N,true);

		for(int i=0; i<N; ++i)
			res[i] = _vector[i] - v[i];

		return res;
	}

private:
	T*	_vector;

//	VectorT() : N(0) {}
};



/////////////////////////////////////////////////////////////////////////////////



// Template de matriz de tipo T de NxN
template <typename T> class MatrixT 
{
#ifndef _DEBUG
	#pragma optimize( "wgt", on )
	#pragma inline_depth( 255 )
#endif

public:
	const int N;
	const int N2;

	inline MatrixT(int n) : N(n), N2(N*N)
	{
		_matrix = new T [N2];
		clear();
	}

	inline MatrixT() : N(0), N2(0) 
	{
	}

	inline void clear()
	{
		for(int i=0; i<N2; ++i)
			_matrix[i] = 0.;
	}

	explicit inline MatrixT(int n, const bool noInit) : N(n), N2(N*N)
	{
		_matrix = new T [N2];
	}

	inline MatrixT(int n, const T* matrix) : N(n), N2(N*N)
	{
		_matrix = new T [N2];

		for(int i=0; i<N2; ++i)
			_matrix[i] = matrix[i]; // debe estar almacenada por columnas si es array 2d
	}

	inline MatrixT(const MatrixT<T>& m) : N(m.N), N2(N*N)
	{
		_matrix = new T [N2];

		for(int i=0; i<N2; ++i)
			_matrix[i] = m._matrix[i];
	}

	inline ~MatrixT() 
	{
		delete [] _matrix;
	}

	inline const MatrixT<T>& operator =(const MatrixT<T>& m)
	{
		if (m.N!=N)
#	ifdef _DEBUG
			throw MatrixException("MatrixT::operator =: el tamaño de la matriz no concuerda");
#	else
		{
			const_cast<int&>(N) = m.N;
			const_cast<int&>(N2) = N*N;
			delete [] _matrix;
			_matrix = new T [N2];
		}

#	endif

		for(int i=0; i<N2; ++i)
			_matrix[i] = m._matrix[i];

		return *this;
	}

	static inline MatrixT<T> I(int n, T diag=1.)
	{
		MatrixT<T> res(n);

		for(int i=0; i<n; ++i)
			res[i][i] = diag;

		return res;
	}

	inline const T& operator()(const int i, const int j) const
	{
#ifdef _DEBUG
		if (i>=N || j>=N)
			throw MatrixException("MatrixT::operator(): Range check failed");
#endif
		return _matrix[j*N+i];
	}

	inline T& operator()(const int i, const int j)
	{
#ifdef _DEBUG
		if (i>=N || j>=N)
			throw MatrixException("MatrixT::operator(): Range check failed");
#endif
		return _matrix[j*N+i];
	}

	inline const T* operator[](const int j) const
	{
#ifdef _DEBUG
		if (j>=N)
			throw MatrixException("MatrixT::operator[]: Range check failed");
#endif
		return _matrix + j*N;
	}

	inline T* operator[](const int j)
	{
#ifdef _DEBUG
		if (j>=N)
			throw MatrixException("MatrixT::operator[]: Range check failed");
#endif
		return _matrix + j*N;
	}

	inline VectorT<T> fila(const int i) const
	{
		VectorT<T> fila_i(N,true);
		
		for(int j=0; j<N; ++j)
			fila_i[j] = _matrix[j*N+i];

		return fila_i;
	}

	inline VectorT<T> columna(const int j) const
	{
		VectorT<T> columna_j(N,true);
		
		for(int i=0; i<N; ++i)
			columna_j[i] = _matrix[j*N+i];

		return columna_j;
	}

	inline const T* columna_array(const int j) const
	{
		return _matrix + j*N;
	}


	// Matriz NxN postmultiplicada por matriz columna
	inline VectorT<T> operator *(const VectorT<T>& v) const
	{
#ifdef _DEBUG
		if (v.N!=N)
			throw MatrixException("MatrixT::operator *(VectorT): el tamaño del vector no concuerda");
#endif
		VectorT<T> ret(N);

		for(int j=0; j<N; ++j)
			for(int i=0; i<N; ++i)
				ret[j] += _matrix[j*N+i] * v[i];

		return ret;
	}

	// Producto de matrices
	inline MatrixT<T> operator *(const MatrixT<T>& m) const
	{
#ifdef _DEBUG
		if (m.N!=N)
			throw MatrixException("MatrixT::operator *(MatrixT): el tamaño de las matrices no concuerda");
#endif
		MatrixT<T> ret(N);

		for(int i=0; i<N; ++i)
			for(int j=0; j<N; ++j)
				for(int k=0; k<N; ++k)
					ret[j][i] += _matrix[k*N+i] * m[j][k];

		return ret;
	}

	inline MatrixT<T>& operator +=(const MatrixT<T>& m)
	{
#ifdef _DEBUG
		if (m.N!=N)
			throw MatrixException("MatrixT::operator +=: el tamaño de las matrices no concuerda");
#endif
		for(int i=0; i<N; ++i)
			for(int j=0; j<N; ++j)
				(*this)[i][j] += m[i][j];

		return *this;
	}

	inline MatrixT<T>& operator -=(const MatrixT<T>& m)
	{
#ifdef _DEBUG
		if (m.N!=N)
			throw MatrixException("MatrixT::operator +=: el tamaño de las matrices no concuerda");
#endif
		for(int i=0; i<N; ++i)
			for(int j=0; j<N; ++j)
				(*this)[i][j] -= m[i][j];

		return *this;
	}

	// transponer la matriz
	inline void trans() 
	{
		T tmp;
        
		for(int j=1; j<N; ++j)
			for(int i=0; i<j; ++i)
			{
				tmp = _matrix[i*N+j];
				_matrix[i*N+j] = _matrix[j*N+i];
				_matrix[j*N+i] = tmp;
			}
	}

	// Determinante de la matriz
	T det() const;

	// invertir con factorizacion LU si es posible
	inline bool invertir();

	// invertir con factorizacion de Cholesky si es posible
	inline bool invertir_cholesky();

	// Factorizacion LU
	inline bool factor_LU();

	// Factorizacion de Cholesky
	inline bool factor_cholesky(bool bNoupper=false);

	// Resuelve el sistema M<NxN> * X<Nx1> = B<Nx1>, devuelve true si pudo factorizar, output en input (B)
	// bool factor_and_solve(VectorT<T,N>& B) const;

	// Sacar por pantalla la matriz
	void print(const char* pre=0) const
	{
		if (pre) printf("%s\n", pre);
		for(int i=0; i<N; ++i)
		{
			for(int j=0; j<N; ++j)
				printf(PRINTF_STR, (float)_matrix[j*N+i]);

			printf("\n");
		}
		if (pre) printf("\n");
	}

private:
	T*	_matrix;
//	MatrixT() : N(0), N2(0) {}
};



/////////////////////////////////////////////////////////////////////////////////




// Template de matriz de tipo T de tamaño dinamico
template <typename T> class MatrixDyn 
{
#ifndef _DEBUG
	#pragma optimize( "wgt", on )
	#pragma inline_depth( 255 )
#endif

public:
	inline MatrixDyn(int m, int n) : _m(m), _n(n)
	{
		_matrix = new T [m*n];
		clear();
	}

	//template <int N> explicit inline MatrixDyn(const MatrixT<T,N>& mat) : _m(N), _n(N)
	explicit inline MatrixDyn(const MatrixT<T>& mat) : _m(mat.N), _n(mat.N)
	{
		const int N = mat.N;
		_matrix = new T [N];

		for(int j=0; j<N; ++j)
			for(int i=0; i<N; ++i)
				_matrix[j*N+i] = mat[j][i];
	}

	//template <int N> explicit inline MatrixDyn(const VectorT<T,N>& vec, bool isCol=true) : _m(isCol?N:1), _n(isCol?1:N)
	explicit inline MatrixDyn(const VectorT<T>& vec, bool isCol=true) : _m(isCol?vec.N:1), _n(isCol?1:vec.N)
	{
		_matrix = new T [vec.N];
		
		for(int i=0; i<vec.N; ++i)
			_matrix[i] = vec[i];
	}

	inline MatrixDyn<T> trans() const
	{
		MatrixDyn<T>	m(_n, _m);
		
		for(int i=0; i<_m; ++i)
			for(int j=0; j<_n; ++j)
				m(j,i) = (*this)(i,j);

		return m;
	}

	inline void clear()
	{
		for(int i=0; i<_m*_n; ++i)
			_matrix[i] = 0.;
	}

	inline MatrixDyn(const T* matrix, int m, int n) : _m(m), _n(n)
	{
		_matrix = new T [m*n];

		for(int i=0; i<m*n; ++i)
			_matrix[i] = matrix[i]; // debe estar almacenada por columnas si es array 2d
	}

	inline MatrixDyn(const MatrixDyn<T>& m) : _m(m._m), _n(m._n)
	{
		_matrix = new T [_m*_n];

		for(int i=0; i<_m*_n; ++i)
			_matrix[i] = m._matrix[i];
	}

	inline ~MatrixDyn() 
	{
		delete [] _matrix;
	}

	inline const T& operator()(const int i, const int j) const
	{
#ifdef _DEBUG
		if (j>=_n || i>=_m)
			throw MatrixException("MatrixDyn::operator(): Range check failed");
#endif
		return _matrix[j*_m + i];
	}

	inline T& operator()(const int i, const int j)
	{
#ifdef _DEBUG
		if (j>=_n || i>=_m)
			throw MatrixException("MatrixDyn::operator(): Range check failed");
#endif
		return _matrix[j*_m + i];
	}

	inline const T* operator[](const int j) const
	{
#ifdef _DEBUG
		if (j>=_n)
			throw MatrixException("MatrixDyn::operator[]: Range check failed");
#endif
		return _matrix + j*_m;
	}

	inline T* operator[](const int j)
	{
#ifdef _DEBUG
		if (j>=_n)
			throw MatrixException("MatrixDyn::operator[]: Range check failed");
#endif
		return _matrix + j*_m;
	}


	/*template <int N2> inline MatrixT<T,N2> operator *(const MatrixDyn<T>& mat) const
	{
		MatrixT<T, _m> res;
		return res;
	}*/

	// Conversion a matriz cuadrada MatrixT<T,N> (si esta MatrixDyn es cuadrada)
	//template <int N> inline operator MatrixT<T,N> () const
	inline operator MatrixT<T> () const
	{
#ifdef _DEBUG
		if (_m!=_n)
			throw MatrixException(
				"MatrixDyn::operator MatrixT<T>: No se puede convertir, no es una matriz cuadrada" );
#endif
		//assert(_m==N && _n==N);

		return MatrixT<T>(_m, _matrix);
	}

	//template <int N> inline operator VectorT<T,N> () const
	inline operator VectorT<T> () const
	{
#ifdef _DEBUG
		if (_m!=1 && _n!=1)
			throw MatrixException(
				"MatrixDyn::operator VectorT<T>: No se puede convertir, no es matriz fila o columna" );
		// if ( ((_m==1)?_n:_m) != N )
		// 	throw MatrixException(
		// 		"MatrixDyn::operator VectorT<T,N>: No se puede convertir, dimension incorrecta" );
#endif
		
		return VectorT<T>((_m==1)?_n:_m, _matrix);
	}

	// Multiplicacion por otra matriz dinamica
	inline MatrixDyn<T> operator *(const MatrixDyn<T>& mat) const
	{
#ifdef _DEBUG
		if (_n!=mat._m)
			throw MatrixException("MatrixDyn::operator *(MatrixDyn): No se puede multiplicar cols1!=filas2");
#endif

		MatrixDyn<T>	res(_m, mat._n);

		for(int i=0; i<_m; ++i)
			for(int j=0; j<mat._n; ++j)
				for(int k=0; k<_n; ++k)
					res(i,j) += (*this)(i,k) * mat(k,j);

		return res;
	}

	// Multiplicacion por matriz cuadrada
	inline MatrixDyn<T> operator *(const MatrixT<T>& mat) const
	{
#ifdef _DEBUG
		if (_n!=mat.N)
			throw MatrixException("MatrixDyn::operator *(MatrixN): No se puede multiplicar cols1!=filas2");
#endif

		MatrixDyn<T>	res(_m, mat.N);

		for(int i=0; i<_m; ++i)
			for(int j=0; j<mat.N; ++j)
				for(int k=0; k<mat.N; ++k)
					res(i,j) += (*this)(i,k) * mat(k,j);

		return res;
	}

	void print(const char* pre=0) const
	{
		if (pre) printf("%s\n", pre);
		for(int i=0; i<_m; ++i)
		{
			for(int j=0; j<_n; ++j)
				printf(PRINTF_STR, (float)((*this)(i,j)));

			printf("\n");
		}
		if (pre) printf("\n");
	}

private:
	MatrixDyn() : _m(0), _n(0) {}

	T*	_matrix;
	const int _m, _n;

};


template <typename T> class BBox
{
public:
	BBox(const T& x, const T& y) : m_maxX(x), m_maxY(y), m_minX(x), m_minY(y)
	{ 
	}

	~BBox() 
	{ 
	}

	inline void	addPoint(const T& x, const T& y)
	{
		if (x>m_maxX) m_maxX = x;
		else
		if (x<m_minX) m_minX = x;
		
		if (y>m_maxY) m_maxY = y;
		else
		if (y<m_minY) m_minY = y;
	}

	inline bool	isInside(const T& x, const T& y) const
	{
		return x>=m_minX && x<=m_maxX && y>=m_minY && y<=m_maxY; 
	}

	inline bool	isInsideX(const T& x) const
	{
		return x>=m_minX && x<=m_maxX; 
	}

	inline bool	isInsideY(const T& y) const
	{
		return y>=m_minY && y<=m_maxY; 
	}

	inline T	minX() const { return m_minX; }
	inline T	maxX() const { return m_maxX; }
	inline T	minY() const { return m_minY; }
	inline T	maxY() const { return m_maxY; }

protected:
	T	m_maxX, m_maxY, m_minX, m_minY;
};



/////////////////////////////////////////////////////////////////////////////////

// Matriz
typedef MatrixDyn<double>	MatrixD;
typedef MatrixDyn<float>	MatrixDf;

// Matriz cuadrada
typedef MatrixT<double>	Matrix;

// Vector
typedef VectorT<double>	Vector;
