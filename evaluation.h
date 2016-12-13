/*
 * Modulo: Vision artificial
 *
 * Descripcion: Funciones de evaluacion de metodos de clasificacion en consola
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 * 
 * $Id: evaluation.h 400 2008-04-05 23:54:09Z Alvaro $
 */

#include "stdafx.h"

#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

#pragma warning( disable : 4996 )
#include "cv.h"
#include "cxcore.h"
#include "cvaux.h"
#include "highgui.h"
#pragma warning( disable :  )

#include "KnnClassifier.h"
#include "HKNNClassifier.h"
#include "CombinedClassifier.h"
#include "Procesador.h"
#include "Moments.h"
#include "Camara.h"
#include "Objeto.h"


#include "Matrix.h"


// obtiene un vector
vector<double>	eval_vector(const char* fname, 
							Procesador::Config cfg);

// entrena con un vector
void			eval_train(Classifier<vector<double>,int>* pClas, 
						   const vector<double>&vector, 
						   int tipo_real);

// obsoleta
int				eval_buscar(Classifier<vector<double>,int>* pClas, 
							const char* fname, 
							Procesador::Config cfg);

