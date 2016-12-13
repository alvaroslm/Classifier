/*
 * Modulo: Vision artificial
 *
 * Descripcion: Implementacion de la clase KnnClassifier
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 *
 * $Id: KnnClassifier.cpp 307 2008-03-20 02:16:17Z Alvaro $
 */

#include "stdafx.h"
#include <stdio.h>


#include "KnnClassifier.h"

#include "Matrix.h"



KnnClassifier::KnnClassifier(int dim, bool bAdjHu) : _maxid(0), _bAdjHu(bAdjHu), Dim(dim), _media(dim),
													 _covarianza(dim), _covarianza_inv(dim)
{
}

KnnClassifier::~KnnClassifier()
{
}

void KnnClassifier::train(const vector<double>& v0, const int& id)
{
	const vector<double>& v = _bAdjHu ? adjHu(vector<double>(v0)) : v0;

	unsigned i=0;

	_map.insert(pair<int,vector<double> >(id, v));

	for(vector<double>::const_iterator I=v.begin(); I!=v.end(); ++I, ++i)
	{
		if (_min.size()>i) 
		{
			if (*I<_min[i])
				_min[i] = *I;
		} 
		else 
			_min.push_back(*I);
			
		if (_max.size()>i) 
		{
			if (*I>_max[i])
				_max[i] = *I;
		} 
		else
			_max.push_back(*I);
	}

	if (id>_maxid)
		_maxid = id;
}



// Calcula el vector medio y la matriz de covarianzas, que se usara para calcular la distancia de Mahalanobis
void KnnClassifier::end_train()
{
	multimap<int, vector<double> >::iterator I;
	_media.clear();

	// Calculamos el vector medio
	int i, j, n_muestras=0;
	for(I=_map.begin(); I!=_map.end(); ++I)
	{
		_media += I->second;
		++n_muestras;
	}
	_media /= (double)n_muestras;

	// Calcula la matriz de covarianzas
	for(I=_map.begin(); I!=_map.end(); ++I)
	{
		Vector vec = (I->second);

		for(j=0; j<Dim; ++j)
			for(i=j; i<Dim; ++i) 
				_covarianza[j][i] += (vec[i]-_media[i])*(vec[j]-_media[j]);
	}

	// Dividimos por n_muestras y copiamos todo lo de por debajo de la diagonal inferior a la otra mitad
	// que no habiamos calculado porque la matriz sabemos que es simetrica
	for(j=0; j<Dim; ++j)
		for(i=j; i<Dim; ++i)
		{
			_covarianza[j][i] /= (double)n_muestras;
			_covarianza[i][j] = _covarianza[j][i];
		}

	// printf("\nCovarianza:\n");
	// _covarianza.print();

	_covarianza_inv = _covarianza;
	if (!_covarianza_inv.invertir_cholesky()) // invertimos la matriz con la factorizacion de Cholesky
	{
		printf("\nKnnClas::end_train: Ha fallado invertir con Cholesky (no deberia ocurrir), usando LU\n");
		if (!_covarianza_inv.invertir())
			printf("KnnClas::end_train: Ha fallado invertir con factorizacion LU\n");
	}

	// printf("\n\nCovarianza invertida:\n");
	// _covarianza_inv.print();

	// printf("\n\nCovarianza * Covarianza invertida:\n");
	// (_covarianza * _covarianza_inv).print();
}



vector<double> KnnClassifier::adjHu(const vector<double>& v0) const
{
	vector<double> v = v0;

	// Fisher

	v[0] = log(10.*v[0]);
	v[1] = log( (v[1]+0.0001) / 0.0101 );
	v[2] = log( (v[2]+0.0005) / 0.0025 );
	v[3] = log( (v[3]+0.0000001) / 0.0000101 );
		
	v[4] = ((v[4]<0.)?-1.:1.) * pow(abs(v[4]), 1./3.);
	v[5] = ((v[5]<0.)?-1.:1.) * pow(abs(v[5]), 1./3.);
	v[6] = ((v[6]<0.)?-1.:1.) * pow(abs(v[6]), 1./3.);

//	for(unsigned j=0; j<v.size(); ++j)
//		v[j] = (v[j]-_min[j]) * 2.0 / (_max[j]-_min[j]);

	v[0] += 4.;
	v[1] += 3.;
	v[2] += 3.;
	v[4] -= 3.;

	return v;
}

void KnnClassifier::print()
{
	for(multimap<int,vector<double> >::iterator I=_map.begin(); I!=_map.end(); ++I)
	{
		vector<double>& v = I->second;

		for(unsigned j=0; j<v.size(); ++j)
		{
			printf("%f ", v[j]);
			if ((j+1)%4==0) printf("\n");
		}
		printf("\n\n");
	}
}

double KnnClassifier::distancia(const vector<double>& u, const vector<double>& v) const
{
/*	distancia euclidea

	unsigned int i=0;
	double r = 0.;
	while(i<u.size() && i<v.size())
	{
		r += (v[i]-u[i])*(v[i]-u[i]);
		++i;
	}

	return sqrt(r);*/

	Vector vdist = Vector(u)-v;

	// Distancia de Mahalanobis

	return sqrt(vdist * _covarianza_inv * vdist); // vector fila * matriz 7x7 * vector columna
	// return sqrt(vdist* Matrix7::I() *vdist); // distancia euclidea
}

int KnnClassifier::find(const vector<double>& v) const
{
	// los 3 mas votados, dist max 0.02, dist doble voto 0.01

	return find(v, 5);
	//return find(v, 5, 10., 5.);

//	return find(v, 5, 10., 1.);
	//return find(v, 3, 0.5, 0.2);

/*	if (_bAdjHu)
		return find(v, 3, 1.5, 0.8);
		//return find(v, 3, 2.5, 1.0);
	else
		return find(v, 3, 0.025, 0.01);
		//return find(v, 3, 0.025, 0.01);
*/
}

int KnnClassifier::find(const vector<double>& v0, int n, double maxdist, double ddist, const set<int>& includeTypes, const set<int>& excludeTypes) const
{
	const vector<double> v = _bAdjHu ? adjHu(v0) : v0;

	// recorremos todos los vectores, almacenamos (distancia, tipo)
	multimap<double,int> m;
	for(multimap<int,vector<double> >::const_iterator I=_map.begin(); I!=_map.end(); ++I)
	{
		// excluimos los tipos que nos digan
		if (excludeTypes.find(I->first)!=excludeTypes.end())
			continue;
		
		// tambien excluimos los que no nos han dicho que incluyamos, si hay conjunto includeTypes
		if (!includeTypes.empty() && includeTypes.find(I->first)==includeTypes.end())
			continue;

		m.insert(pair<double,int>(distancia(v,I->second), I->first));
	}

	// recorremos las n distancias mas cercanas calculadas en orden de distancia hasta maxdist
	map<int, int> votos;
	multimap<double,int>::const_iterator J;
	for(J=m.begin(); J!=m.end() && J->first<=maxdist; ++J)
	{
		if (n--==0)
			break;
		printf("dist de %d = %f\n", J->second, J->first);
		votos[J->second] += J->first<ddist ? 2 : 1; // ponderamos los votos, si la distancia es <ddist valen doble
	}
	printf("___maxdist o nmax___\n");
	for(; J!=m.end(); ++J)
	{
		printf("dist de %d = %f\n", J->second, J->first);
	} /**/

	// recorremos los votos para determinar el ganador y para sacarlos por pantalla
	int masvotado = 0;
	int maxvotos = 0;
	for(map<int,int>::iterator K=votos.begin(); K!=votos.end(); ++K)
	{
		if (K->second>maxvotos) 
		{
			masvotado = K->first;
			maxvotos = K->second;
		}

		printf("tipo %d: %d votos\n", K->first, K->second);
	}
		
	return masvotado;
}


bool KnnClassifier::store(const char* fname) const
{
	FILE* f = fopen(fname, "wt");
	if (!f) return false;

	for(multimap<int,vector<double> >::const_iterator I=_map.begin(); I!=_map.end(); ++I)
	{
		fprintf(f, "%d ", I->first);
		
		for(unsigned j=0; j<I->second.size(); ++j)
			fprintf(f, "%LG ", (long double)I->second[j]);
		
		fprintf(f, "\n");
	}

	fclose(f);
	return true;
}

bool KnnClassifier::load(const char* fname)
{
	FILE* f = fopen(fname, "rt");
	if (!f) return false;

	clear();

	char buffer[1503];
	char seps[]   = " ,\t\n\r";
	int id = 0;
	
	bool bAdjHu = _bAdjHu;
	_bAdjHu = false;

	while(!feof(f) && fgets(buffer, 1500, f))
	{
		vector<double> v;
		
		char* token = strtok(buffer, seps);
		if (!token) break;
		sscanf(token, "%d", &id);
		token = strtok(NULL, seps);
		
		while(token!=NULL)
		{
			if (strlen(token)!=0)
			{
				long double val = 0.;
				// printf("%s*", token);
				sscanf(token, "%LG", &val);
				//printf("--%LG--", val);
				v.push_back((double)val);
			}
			token = strtok(NULL, seps);
		}

		train(v, id);
	}
	_bAdjHu = bAdjHu;

	fclose(f);

	end_train();

	return true;
}

void KnnClassifier::clear()
{
	_max.clear();
	_min.clear();
	_map.clear();
	_maxid = 0;
}



///////////////////////////////////////////////////////////////////////

/*void KnnClassifier::adjHu()
{
	for(multimap<int,vector<double> >::iterator I=_map.begin(); I!=_map.end(); ++I)
	{
		__adjHu(I->second);
	}

	__adjHu(_min);
	__adjHu(_max);

//	normalize(2.0);

	for(multimap<int,vector<double> >::iterator I=_map.begin(); I!=_map.end(); ++I)
	{
		vector<double>& v = I->second;

		v[0] += 4.;
		v[1] += 3.;
		v[2] += 3.;
		v[4] -= 3.;
	}
}*/

/*void KnnClassifier::normalize(double factor)
{
	for(multimap<int,vector<double> >::iterator I=_map.begin(); I!=_map.end(); ++I)
	{
		vector<double>& v = I->second;

		for(unsigned j=0; j<v.size(); ++j)
			v[j] = (v[j]-_min[j]) * factor / (_max[j]-_min[j]);
	}

	//for(unsigned j=0; j<_min.size() && j<_max.size(); ++j)
	//{
	//	_min[j] = 0.;
	//	_max[j] = factor;
	//}
}*/

/*void __adjHu(vector<double>& v)
{
	v[0] = log(10.*v[0]);
	v[1] = log( (v[1]+0.0001) / 0.0101 );
	v[2] = log( (v[2]+0.0005) / 0.0025 );
	v[3] = log( (v[3]+0.0000001) / 0.0000101 );
		
	v[4] = ((v[4]<0)?-1.:1.) * pow(abs(v[4]), 1./3.);
	v[5] = ((v[5]<0)?-1.:1.) * pow(abs(v[5]), 1./3.);
	v[6] = ((v[6]<0)?-1.:1.) * pow(abs(v[6]), 1./3.);
}
*/