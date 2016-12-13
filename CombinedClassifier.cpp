/*
 * Modulo: Vision artificial
 *
 * Descripcion: Implementación de la clase CombinedClassifier
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 *
 * $Id: CombinedClassifier.cpp 415 2008-04-08 17:32:31Z Alvaro $
 */

#include "stdafx.h"
#include <stdio.h>

#include "CombinedClassifier.h"
#include "HKNNClassifier.h"
#include "FourierShapeDesc.h"

#include <string>
using namespace std;


CombinedClassifier::CombinedClassifier()
{
	m_pHuClassifier = new HKNNClassifier(8, true, true); // 7 dims+largo, adjhu y mahal.
	m_pFourierClassifier = new HKNNClassifier(FourierShapeDesc::Nsamples-2, false, false);
}

CombinedClassifier::~CombinedClassifier()
{
	store("training");

	if (m_pHuClassifier)
		delete m_pHuClassifier;

	if (m_pFourierClassifier)
		delete m_pFourierClassifier;
}



void CombinedClassifier::train(const vector<double>& v0, const int& id)
{
	vector<double> vHu, vF;

	vHu.insert(vHu.begin(), v0.begin(), v0.begin()+8); //los 8 primeros (7+tamaño)
	vF.insert(vF.begin(), v0.begin()+8, v0.end()); // los restantes

	m_pHuClassifier->train(vHu, id);
	m_pFourierClassifier->train(vF, id);
}



void CombinedClassifier::end_train()
{
	m_pHuClassifier->end_train();
	m_pFourierClassifier->end_train();
}


void CombinedClassifier::print()
{
	m_pHuClassifier->print();
	m_pFourierClassifier->print();
}


int CombinedClassifier::find(const vector<double>& v) const
{
	//return find(v, 5, 0., 3, 10., 3.);
	// MEJORES VALORES AJUSTADOS
	return find(v, /*kHU=*/ 3, /*lambdaHU=*/ 10., /*KFourier=*/ 2, /*lambdaFourier=*/ 0., /*dist21=*/ 1.25);
}


int CombinedClassifier::find(const vector<double>& v, int KHu, double lambdaHu, int KFourier, double lambdaFourier, double dist21) const
{
	vector<double> vHu, vF;

	vHu.insert(vHu.begin(), v.begin(), v.begin()+8); //los 8 primeros (7+tamaño)
	vF.insert(vF.begin(), v.begin()+8, v.end()); // los restantes

	printf("_________________FIND Hu: _____________________________\n");
	multimap<double,int> mdists = m_pHuClassifier->find_dists(vHu, KHu, lambdaHu);
	multimap<double,int>::const_iterator I = mdists.begin();
	int findH = (I!=mdists.end()) ? I->second : 0;
	printf("_________________FIND Fourier: ________________________\n");
	multimap<double,int> mdistsF = m_pFourierClassifier->find_dists(vF, KFourier, lambdaFourier);
	multimap<double,int>::const_iterator J = mdistsF.begin();
	int findF = (J!=mdistsF.end()) ? J->second : 0;
	printf("_______________________________________________________\n");

	printf("CombinedClassifier::find: Hu da clase %d, Fourier da clase %d\n", findH, findF);
	
	int ganador = 0;

	double dist1 = (findH>0) ? I->first : 999999.;
	double dist1F = (findF>0) ? J->first : 999999.;
	if (I!=mdists.end()) I++;
	if (J!=mdistsF.end()) J++;
	double dist2 = (I!=mdists.end()) ? I->first : 999999.;
	double dist2F = (J!=mdistsF.end()) ? J->first : 999999.;

	if (findF==findH && findF!=0 && (dist1F<2.5 || dist1<1.2 || dist2F/dist1F > dist21 || dist2/dist1 > dist21))
		ganador = findF;
	else if (findF!=0 && dist1F<2.5 && dist2F/dist1F > dist21)
		ganador = findF;
	else if (findH!=0 && dist1<1.2 && dist2/dist1 > dist21)
		ganador = findH;
	else
	{
		if (findF>0 && dist1F<1.5)
			ganador = findF;
		else
			if (findH>0 && dist1<0.7)
				ganador = findH;

		ganador = 0;
	}

	// PARA DESHACER LOS ULTIMOS CAMBIOS AQUI (ENREVESADOS Y AÑADEN POCO), VOLVER A REVISION 399 en este file
/*	if (dist1F>6. && dist1>2.5)
		ganador = 0;
	else if (mdists.size()<=1) // si solo hay una clase
		ganador = findH; // el ganador es la unica clase esa
	else if (findH!=findF && findH!=0 && findF!=0) // si discrepan los dos clasificadores
	{
		I++; J++;
		double dist2 = (I!=mdists.end()) ? I->first : 999999.;
		double dist2F = (J!=mdistsF.end()) ? J->first : 999999.;
		int findH2 = (I!=mdists.end()) ? I->second : 0;
		int findF2 = (J!=mdistsF.end()) ? J->second : 0;

		if (dist2/dist1 < dist21 || (dist1>dist1F && dist1>1.5)) // la distancia de la segunda clase es menos del triple de la distancia de la primera mas cercana
		{
			if (dist1F<8. && (findF==findH2 || dist1>4.0)) // si ademas de ser la distancia proxima, fourier dice q la buena es la segunda, le hacemos caso
				ganador = findF;
			else
			{
				if (dist1<4.0)
					ganador = findH; 
				else
					ganador = 0; // en caso contrario, decimos que no sabemos
			}
		}
		else
			ganador = findH; // la distancia a la segunda clase es grande, la primera debe ser la solucion		
	}
	else
		ganador = findH; // los dos clasificadores estan de acuerdo (ó alguno da 0), devolvemos su solucion
*/
	printf("    CombinedClassifier::find: resultado final: clase %d\n", ganador);

	return ganador;
}


bool CombinedClassifier::load(const char* fname)
{
	string Hname = (string)fname + ".htn";
	string Fname = (string)fname + ".ftn";
	return m_pHuClassifier->load(Hname.c_str()) && m_pFourierClassifier->load(Fname.c_str());
}


bool CombinedClassifier::store(const char* fname) const
{
	string Hname = (string)fname + ".htn";
	string Fname = (string)fname + ".ftn";
	return m_pHuClassifier->store(Hname.c_str()) && m_pFourierClassifier->store(Fname.c_str());
}


void CombinedClassifier::clear()
{
	m_pHuClassifier->clear();
	m_pFourierClassifier->clear();
}


