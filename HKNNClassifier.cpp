/*
 * Modulo: Vision artificial
 *
 * Descripcion: Implementacion de la clase HKNNClassifier
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007. All rights reserved.
 *
 * $Id: HKNNClassifier.cpp 402 2008-04-06 11:22:44Z Alvaro $
 */

#include "stdafx.h"
#include "HKNNClassifier.h"

/////////// HKNN classifier ///////////////////////////////////////////

HKNNClassifier::~HKNNClassifier()
{
}

double HKNNClassifier::distancia(const vector<double>& u, const vector<double>& v) const
{
	Vector vdist = Vector(u)-v;

	if (_bMahal)
	{
		// Distancia de Mahalanobis
		return sqrt(vdist*_covarianza_inv*vdist);
	}
	else
	{
		// distancia euclidea
		return sqrt(vdist * Matrix::I(Dim) * vdist); 
	}
}

int HKNNClassifier::find(const vector<double>& v) const
{
	try 
	{
		return find(v, 3, 10.);
	}
	catch(MatrixException me)
	{
		printf("EXCEPCION: %s\n", me.getMessage());
	}

	return 0;
}

multimap<double,int> HKNNClassifier::find_dists(const vector<double>& v0, int K, double lambda, const set<int>& includeTypes, const set<int>& excludeTypes) const
{
	const vector<double> vX = _bAdjHu ? adjHu(v0) : v0;
	const Vector X(vX);

	//assert(K<=6);
	assert(K>1); // hay vectores y matrices con dimension K-1

	multimap<double,int> mdists;

	int k=-1;
	for(int clase=1; k!=0; ++clase)
	{
		// excluimos los tipos que nos digan
		if (excludeTypes.find(clase)!=excludeTypes.end())
			continue;
		
		// tambien excluimos los que no nos han dicho que incluyamos, si hay conjunto includeTypes
		if (!includeTypes.empty() && includeTypes.find(clase)==includeTypes.end())
			continue;

		k = 0;
		// recorremos todos los vectores de la clase, almacenamos en S (distancia, Vector)
		multimap<double,Vector> S;
		for(multimap<int,vector<double> >::const_iterator I=_map.begin(); I!=_map.end(); ++I)
		{
			if (I->first==clase)
			{
				//printf("insertando en S vector de clase %d a distancia %.4f\n", clase, (float)distancia(vX, I->second));
				S.insert(pair<double,Vector>(distancia(vX, I->second), Vector(I->second)));
			}
		}

		Vector vmedio(Dim);
		multimap<double,Vector>::iterator J;
		for(J=S.begin(), k=0;   J!=S.end() && k<K;   ++J,++k)
		{
			// este bucle recorre los vectores de cada punto del conjunto de los K mas cercanos de la clase
			vmedio += J->second;
		}

		//printf("k=%d para clase %d; K=%d\n", k, clase, K);

		if (k!=K)
			continue;

		vmedio /= (double)K;

		MatrixD V(Dim, K-1);
		for(J=S.begin(), k=0;   J!=S.end() && k<K-1;   ++k)
		{
			// este bucle recorre los vectores de cada punto del conjunto de los K-1 mas cercanos de la clase
			// quitamos 1 (estamos usando K-1) para que el sistema de ecuaciones no sea degenerado
			// y de solucion unica (de las infinitas soluciones nos da igual cual coger, lo que queremos
			// es hallar la distancia al hiperplano)

			//printf("  recorriendo vector%d\n", k);
			const Vector &v = (J->second -= vmedio); // ajustamos coordenadas respecto a vmedio (esto hace que las columnas sean linealmente dependientes si no kitamos 1 vector)

			//v.print("v");

			for(int i=0; i<Dim; ++i)
				V[k][i] = v[i];

			++J;
		}
		
		// V.print("V");

		// Ahora ya tenemos en V los vectores 1..k-1 como columnas y en Vtrans V transpuesta
		MatrixD Vtrans = V.trans();

		// Vtrans.print("Vtrans");

		//////

		// vmedio.print("vmedio");
		// X.print("X");

		// Hallamos b multiplicando Vt * (X-vmedio)
		Vector B(K-1, true);
		if (_bMahal)
			B = Vtrans * _covarianza_inv * MatrixD(X-vmedio);
		else
			B = Vtrans * MatrixD(X-vmedio);

		//(X-vmedio).print("X-vmedio");
		//MatrixD(X-vmedio).print("MatrixD(X-vmedio)");
		// B.print("B");

		// Resolvemos y hallamos alpha
		Matrix VtV(K-1, true);
		if (_bMahal)
			VtV = (Vtrans*_covarianza_inv*V);
		else
			VtV = (Vtrans*V);
		VtV += Matrix::I(K-1, lambda); // esta linea quitarla para no usar lambda o hacer lambda=0
		Matrix VtV_inv = VtV;
		
		if (!VtV_inv.invertir_cholesky())
		{
			if (K>2)
				printf("***HKNN-find Ha fallado invertir con fact. Cholesky, usando LU\n");

			if (!VtV_inv.invertir())
				printf("***err** HKNN-find Ha fallado invertir con fact. LU\n");
			else
				if (K>2)
					printf("La factorizacion LU ha funcionado pero probablemente el problema estaba mal condicionado o directamente el determinante deberia ser 0 y no lo era por errores acumulados\n");	
		}

		// VtV.print("VtV");
		// printf("det(VtV)=%f\n\n", (float)VtV.det());
		// VtV_inv.print("VtV_inv");
		// printf("det(VtV_inv)=%f\n\n", (float)VtV_inv.det());

		// (VtV*VtV_inv).print("(VtV*VtV_inv)");

		Vector alpha = VtV_inv * B;
		
		// B.print("B (Vtrans*(X-vmedio))");
		// alpha.print("\nalpha (VtV_inv*B)");
		// (VtV*alpha).print("\nVtV*alpha (=B)");

		// Ahora ya podemos calcular la distancia al hiperplano local de la clase
		Vector vsuma(Dim);
		double penalizacionLambda = 0.;
		for(J=S.begin(), k=0;   J!=S.end() && k<K-1;   ++k)
		{
			penalizacionLambda += alpha[k]*alpha[k]; // quitar esta linea si no se quiere usar lambda
			vsuma += (J->second) * alpha[k];  // escalar_alpha_k * V_k
			++J;
		}        		
		penalizacionLambda *= lambda;

		//vsuma.print("\nvsuma");
		Vector vdist = X - vmedio - vsuma;

		// printf("penalizacionLambda=%f\n", (float)penalizacionLambda);

		// Elegir una de las 2 distancias y usar la misma en la funcion distancia()
		double dist;
		if (_bMahal)
			dist = sqrt(vdist*_covarianza_inv*vdist + penalizacionLambda); // dist. Mahalanobis 
		else
			dist = sqrt(vdist.norma2() + penalizacionLambda); // distancia euclidea
		// double dist = sqrt(Vector(vdist)*_covarianza_inv*Vector(vdist) * (1+(penalizacionLambda/vdist.norma2()))); // para pruebas si dim(vdist)!=7
		//xx	double dist = sqrt(vdist*_covarianza_inv*vdist * (1+(penalizacionLambda/vdist.norma2()))); // dist. Mahalanobis <- mejor
		
        // Guardamos esta distancia y su clase para despues ver que clase tiene el espacio mas cercano
		mdists.insert(pair<double,int>(dist, clase));

		//printf("\nnorma de dist=%f\n\n", (float)dist.norma());
	}

	multimap<double,int>::const_iterator I = mdists.begin();
	for(; I!=mdists.end(); ++I)
	{
		printf("Clase %d, distancia %f\n", I->second, (float)I->first);
	}

	return mdists;
}


int HKNNClassifier::find(const vector<double>& v0, int K, double lambda, const set<int>& includeTypes, const set<int>& excludeTypes) const
{
	multimap<double,int> mdists = find_dists(v0, K, lambda, includeTypes, excludeTypes);

	return (mdists.size()!=0) ? mdists.begin()->second : 0;
}

