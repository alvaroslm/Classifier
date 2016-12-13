/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion del clasificador HKNN
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 *
 * $Id: HKNNClassifier.h 301 2008-03-19 13:05:13Z Alvaro $
 */

#pragma once

#include "KnnClassifier.h"

// Clasificador K-local hyperplane nearest neighbours (HKNN) modificado 
class HKNNClassifier : public KnnClassifier
{
public:
	// Constructor
	HKNNClassifier(int dim, bool bAdjHu=true, bool bMahal=true) : KnnClassifier(dim, bAdjHu), _bMahal(bMahal)
	{ }

	// Destructor
	virtual ~HKNNClassifier();

	// Busca el tipo que corresponde a un vector v
	virtual int find(const vector<double>& v) const;

	// Busca el tipo que corresponde a un vector v, maximo n vectores mas cercanos a una distancia maxima maxdist, distancia doble voto ddist, sets include y exclude de varios tipos para restringir
	int find(const vector<double>& v0, int K, double lambda, const set<int>& includeTypes=set<int>(), const set<int>& excludeTypes=set<int>()) const;

	// Busca el tipo que corresponde a un vector v, maximo n vectores mas cercanos a una distancia maxima maxdist, distancia doble voto ddist, sets include y exclude de varios tipos para restringir y devuelve las distancias a las clases
	multimap<double,int> find_dists(const vector<double>& v0, int K, double lambda, const set<int>& includeTypes=set<int>(), const set<int>& excludeTypes=set<int>()) const;

protected:
	double distancia(const vector<double>& u, const vector<double>& v) const;

	//typedef Vector<double,Dim> VectorN;
	//typedef Matrix<double,Dim> MatrixN;

	bool _bMahal;
};
