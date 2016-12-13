/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion de la clase KnnClassifier
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007. All rights reserved.
 *
 * $Id: KnnClassifier.h 22 2007-12-13 23:41:39Z Alvaro $
 */

#pragma once

#include "Classifier.h"

// Clasificador K-nearest neighbours modificado
class KnnClassifier : public Classifier<vector<double>, int>
{
public:
	const int Dim;

	KnnClassifier(int dim, bool bAdjHu=true);

	// Entrena el clasificador con el vector v que corresponde al tipo id
	virtual void train(const vector<double>& v0, const int& id);

	// Hace los calculos que sean necesarios cuando se ha terminado de proporcionar vectores de entrenamiento
	virtual void end_train();

	// Busca el tipo que corresponde a un vector v
	virtual int find(const vector<double>& v) const;

	// Busca el tipo que corresponde a un vector v, maximo n vectores mas cercanos a una distancia maxima maxdist, distancia doble voto ddist, sets include y exclude de varios tipos para restringir
	virtual int find(const vector<double>& v0, int n, double maxdist=1.E10, double ddist=0., const set<int>& includeTypes=set<int>(), const set<int>& excludeTypes=set<int>()) const;
	
	// Saca por pantalla informacion de los momentos, etc
	virtual void print();

	// Guarda en un archivo el entrenamiento conseguido
	virtual bool store(const char* fname) const;

	// Carga de un archivo entrenamiento previo
	virtual bool load(const char* fname);

	// Elimina el entrenamiento adquirido
	virtual void clear();

	// Destructor
	virtual ~KnnClassifier();

protected:
//	void adjHu();
	//const vector<double>& adjHu(vector<double>& v) const;
	vector<double> adjHu(const vector<double>& v0) const;
	double distancia(const vector<double>& u, const vector<double>& v) const;

	vector<double>	_min;
	vector<double>	_max;
	int				_maxid;
	bool			_bAdjHu;

	Vector			_media;
	Matrix			_covarianza;
	Matrix			_covarianza_inv;


private:
	KnnClassifier() : Dim(0) {}
/*	// no usar
	void normalize(double factor=100.);*/

};
