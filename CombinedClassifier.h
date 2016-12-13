/*
 * Modulo: Vision artificial
 *
 * Descripcion: Definicion de la clase CombinedClassifier
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 * 
 * $Id: CombinedClassifier.h 400 2008-04-05 23:54:09Z Alvaro $
 */

#pragma once

#include "Classifier.h"

class HKNNClassifier;


// Clasificador que combina varios metodos
class CombinedClassifier : public Classifier<vector<double>, int>
{
public:
	CombinedClassifier();

	// Entrena el clasificador con el vector v que corresponde al tipo id
	virtual void train(const vector<double>& v0, const int& id);

	// Hace los calculos que sean necesarios cuando se ha terminado de proporcionar vectores de entrenamiento
	virtual void end_train();

	// Busca el tipo que corresponde a un vector v
	virtual int find(const vector<double>& v) const;

	// Busca el tipo que corresponde a un vector v, con parametros de ajuste
	virtual int find(const vector<double>& v, int KHu, double lambdaHu, int KFourier, double lambdaFourier, double dist21) const;

	// Saca por pantalla informacion de la clasificacion, etc
	virtual void print();

	// Guarda en un archivo el entrenamiento conseguido
	virtual bool store(const char* fname) const;

	// Carga de un archivo entrenamiento previo
	virtual bool load(const char* fname);

	// Elimina el entrenamiento adquirido
	virtual void clear();

	// (Const) Clasificador para momentos
	inline const HKNNClassifier* getHuClas() const
	{
		return m_pHuClassifier;
	}

	// (Const) Clasificador para Fourier
	inline const HKNNClassifier* getFourierClas() const
	{
		return m_pFourierClassifier;
	}

	// Destructor
	virtual ~CombinedClassifier();

protected:
	HKNNClassifier* m_pHuClassifier;
	HKNNClassifier* m_pFourierClassifier;

};
