/*
 * Modulo: Vision artificial
 *
 * Descripcion: Implementacion de las clases de vectores y matrices
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 *
 * $Id: Matrix.cpp 408 2008-04-07 14:23:09Z Alvaro $
 */

#include "stdafx.h"
#include "Matrix.h"

template class MatrixDyn<double>;
template class MatrixDyn<float>;
// forzamos que se evaluen varias clases que necesitamos usar

template class MatrixT<double>;
template class MatrixT<float>;

template class VectorT<double>;
template class VectorT<float>;


template<class T> inline bool MatrixT<T>::factor_cholesky(bool bNoupper)
{
	int i,j,k;
	T sum;

	MatrixT<T>	&A = *this;

	for(i=0; i<N; ++i) 
	{
		for(j=i; j<N; ++j) 
		{
			sum = A(i,j);
			
			for(k=i-1; k>=0; --k) 
			{
				sum = sum - A(i,k)*A(j,k);
			}
			
			if (i==j) 
			{
				if (sum<=0.) 
#			ifdef _DEBUG
					//dbg throw MatrixException("MatrixT<T,N>::invertir_cholesky(): Raiz de numero negativo");
					return false;
#			else
					return false;
#			endif

				A(i,i) = sqrt(sum);
			} 
			else 
				A(j,i) = sum / A(i,i);
		}
	}

	// si aplicamos este bucle, nos da simplemente la descomposicion de cholesky (la matriz L) con ceros arriba
	if (!bNoupper)
	{
		for(j=1; j<N; ++j) // pone a cero la mitad superior a la diagonal de la matriz
			for(i=0; i<j; ++i)
				A(i,j) = 0.; 
	}

	return true;
}




template<class T> inline bool MatrixT<T>::invertir_cholesky()
{
	// invertir con factorizacion de Cholesky (mas rapida) si es posible (simetrica, definida positiva)
	if (N==1)
	{
		if (_matrix[0]!=0.0)
		{
			_matrix[0] = (T)1.0/_matrix[0];
			return true;
		}
		else
			return false;
	}

	assert(N>=2);

#ifndef _DEBUG	
	if (N<2)
		return false;
#endif

	int i, j, k;
	T sum;
	
	MatrixT<T>	A = *this;

	if (!A.factor_cholesky(true))
		return false;

	// aqui almacenamos el resultado
	clear();

	// tmp,diag=L
	for(j=0; j<N; ++j) 
	{
		// columna j del resultado
		T* b = (*this)[j];
		b[j] = 1.0;

		// resolvemos
		// L * y = b
		for(i=0; i<N; ++i)
		{
			sum = b[i];
			
			for(k=i-1; k>=0; --k) 
				sum = sum - A(i,k)*b[k];

			b[i] = sum / A(i,i);
		}

		// Ltrans * x = y
  		for(i=N-1; i>=0; --i)
		{
			sum = b[i];

			for(k=i+1; k<N; ++k) 
				sum = sum - A(k,i)*b[k];

			b[i] = sum / A(i,i);
		}
	}
	
	return true;
}



template<class T> inline bool MatrixT<T>::factor_LU()
{
	int i, j;

    // no se puede hacer la factorizacion LU  si algun elemento de la diagonal principal es 0 y de todas
	// formas, proximos a 0 el problema puede estar mal condicionado -> implementar fact. PA=LU pivotando
	for (i=0; i<N; ++i) 
	{
		if (_matrix[i*N+i]==0.)
#	ifdef _DEBUG
			//dbg throw MatrixException("MatrixT<T>::invertir(): Algun elemento de la diagonal principal es 0");
			return false;
#	else
			return false;
#	endif
	}

	for (i=1; i<N; ++i) 
		_matrix[i*N] /= _matrix[0]; // normalizar primera fila


	for (i=1; i<N; ++i)  
	{ 
		for (j=i; j<N; ++j)   // una columna de L
		{
			T sum = 0.0;
			
			for (int k=0; k<i; ++k)  
				sum += _matrix[k*N+j] * _matrix[i*N+k];

			_matrix[i*N+j] -= sum;
		}

		// if (i==N-1) continue; // prescindible, el bucle que sigue comprueba lo mismo al empezar

		for (j=i+1; j<N; ++j)  // una fila de U
		{
			T sum = 0.0;
			
			for (int k=0; k<i; ++k)
				sum += _matrix[k*N+i]*_matrix[j*N+k];

			_matrix[j*N+i] = (_matrix[j*N+i]-sum) / _matrix[i*N+i];
		}
	}

	return true;	
}


template<class T> inline bool MatrixT<T>::invertir()
{
	int i,j;

	if (N==1)
	{
		if (_matrix[0]!=0.0)
		{
			_matrix[0] = (T)1.0/_matrix[0];
			return true;
		}
		else
			return false;
	}

	assert(N>=2);


	// factorizamos
	if (!factor_LU())
		return false;

	///////
	// y resolvemos

    for (i=0; i<N; ++i)  
	{
		for (j=i; j<N; ++j)  
		{
			T x = 1.0;
			if (i!=j) 
			{
				x = 0.0;
				for (int k=i; k<j; ++k) 
					x -= _matrix[k*N+j]*_matrix[i*N+k];
			}
			_matrix[i*N+j] = x / _matrix[j*N+j];
		}
	}

	for (i=0; i<N; ++i)  
	{
		for (j=i; j<N; ++j)  
		{
			if (i==j) continue;
			
			T sum = 0.0;
			
			for (int k=i; k<j; ++k)
				sum += _matrix[j*N+k]*( (i==k) ? T(1.0) : _matrix[k*N+i] ); /**/
			
			/*T sum = _matrix[j][i];  metodo alternativo
			for (int k=i+1; k<j; ++k)
				sum += _matrix[j][k] * _matrix[k][i]; */
			
			_matrix[j*N+i] = -sum;
		}
	}

	for (i=0; i<N; ++i)  
	{
		for (j=0; j<N; ++j)  
		{
			T sum = 0.0;

			for (int k=((i>j)?i:j); k<N; ++k)  
				sum += (j==k) ? _matrix[i*N+k] : (_matrix[k*N+j]*_matrix[i*N+k]);

			_matrix[i*N+j] = sum;
        }
	}

	return true;
}

/*template<> double MatrixT<double,1>::det() const
{
	return _matrix[0][0];
}

template<> double MatrixT<double,2>::det() const
{
	return _matrix[0][0]*_matrix[1][1] - _matrix[0][1]*_matrix[1][0];
}

template<> double MatrixT<double,3>::det() const
{
	return	_matrix[0][0] * _matrix[1][1] * _matrix[2][2] +
			_matrix[1][0] * _matrix[2][1] * _matrix[0][2] +
			_matrix[0][1] * _matrix[1][2] * _matrix[2][0] -
			_matrix[2][0] * _matrix[1][1] * _matrix[0][2] -
			_matrix[1][0] * _matrix[0][1] * _matrix[2][2] -
			_matrix[2][1] * _matrix[1][2] * _matrix[0][0];
}*/

template<typename T> T MatrixT<T>::det() const
{
	if (N<3)
	{
		if (N==2)
			return _matrix[0]*_matrix[3] - _matrix[1]*_matrix[2];
		else
			return _matrix[0];
	}

	MatrixT<T> fact = *this;
	fact.factor_LU();

	T res = 1.;
	
	// det(A) = det(L)*det(U) = det(L)*1 = det(diagL)
	for (int i=0; i<N; ++i)
		res *= fact[i][i];

	return res;
}


/***** test de invertir matrices:

double	m[3][3] = {
						{ 5., 2., 3. },		// columna 1
						{ 1., -1., 2. },	// columna 2
						{ -3., -2., 1. }	// columna 3
						};

	Matrix3 mat((double*)m);

	mat.print();
	printf("\n\ninvertida:\n");

	if (!mat.invertir_cholesky())
	{
		printf("Cholesky ha fallado, usando LU\n");
		if (!mat.invertir())
			printf("LU ha fallado\n");
	}
	Matrix3 mat_inv = mat;
	mat.print();
	printf("\n\ninvertida otra vez:\n");

	mat.invertir();
	mat.print();
	printf("\n\nmat * mat_inv:\n");

	(mat * mat_inv).print();
	printf("\n\n");

	return 0;

	////////////////////////////////////////////////////////

	//  test de invertir con cholesky
double	m[3][3] = {
						{ 1., 2., 0. },		// columna 1
						{ 2., 8., 4. },	// columna 2
						{ 0., 4., 13. }	// columna 3
					};

						{ 0.42, -0.15, -0.32 },		// columna 1
						{ -0.15, 0.21, 0.54 },	// columna 2
						{ -0.32, 0.54, 2.32 }	// columna 3

	//  test de invertir con cholesky
double	m[3][3] = {
						{ 0.31, -0.06, -0.01 },		// columna 1
						{ -0.06, 0.18, -0.13 },	// columna 2
						{ -0.01, -0.13, 0.93 }	// columna 3
					};

	Matrix3 mat_LU((double*)m);
	Matrix3 mat_Ch((double*)m);
	Matrix3 mat_orig((double*)m);

	mat_orig.print();
	printf("\n\ninv cholesky:\n");

	if (mat_Ch.invertir_cholesky())
		mat_Ch.print();
	else
		printf("Cholesky ha fallado\n");

	printf("\nA*inv(A)ch=\n");
	(mat_orig*mat_Ch).print();

	printf("\n\ninv LU:\n");
	if (mat_LU.invertir())
		mat_LU.print();
	else
		printf("LU ha fallado\n");

	printf("\nA*inv(A)=\n");
	(mat_orig*mat_LU).print();

	return 0; 
*/

