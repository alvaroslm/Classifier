/*
 * Modulo: Vision artificial
 *
 * Descripcion: Programa principal de test y evaluacion en consola para el clasificador de imagenes
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 *
 * $Id: main.cpp 410 2008-04-07 16:35:43Z Alvaro $
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

#include "evaluation.h"

const char* TrainFile("test_train");


#define EVALUATION_MAIN 1


#ifdef EVALUATION_MAIN


//ev MAIN
int main(int argc, char** argv)
{
	vector<string> files;

	int evaluaciones_bien[5] = {0,0,0,0,0};
	int evaluaciones_mal[5] = {0,0,0,0,0};
	int evaluaciones_noid[5] = {0,0,0,0,0};

	int evaluaciones_bienH[5] = {0,0,0,0,0};
	int evaluaciones_bienF[5] = {0,0,0,0,0};
	int evaluaciones_malHF[5] = {0,0,0,0,0};

	int evaluaciones_totales[5] = {0,0,0,0,0};

	bool bEntrenar = true;

	printf("\n");
	printf("\nPulsa una tecla para empezar (1 para coger eval_training enter para imagenes)...\n");
	if (getchar()=='1')
		bEntrenar = false;


	// Cogido de VisionProcessor, siempre usa Otsu
	Procesador::Config cfg = Procesador::CFG_DEFAULT;
	cfg.wsh_steps = 5; //M era 12
	cfg.wsh_step = 2;
	cfg.wsh_softness = 11; //25;
	cfg.wsh_umbral = 40;
	cfg.th_adjOtsu = 10.; // -5 M era -15.0
	cfg.th_smooth = 25; //M era 29
	cfg.th_threshold = -1; //(max_v>128) ? -1 : max_sxv; //max_v cuando thresholding normal, max_s con th sat


	/////////////////
	
	
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile("images_in\\*.jpg", &findFileData);

	if (hFind==INVALID_HANDLE_VALUE)
	{
		printf("No he podido leer los archivos del directorio images\n");
		return 0;
	}

	do 
	{
		files.insert(files.end(), string(findFileData.cFileName));
	} 
	while(FindNextFile(hFind, &findFileData)!=0);

	////////////////////////

	map<string, vector<double> >	vectores;
	for(vector<string>::const_iterator I = files.begin();
		I != files.end();
		++I)
	{
		vector<double> vect = 
			eval_vector(((string)"images_in\\" + (*I)).c_str(), cfg);

		vectores[*I] = vect;
	}

	// ajustes de los metodos  --> meterlos al find x defecto de combinedclassifier si se optimizan
	int KHu = 3;
	double lambdaHu = 10.;
	int KFourier = 2;
	double lambdaFourier = 0.;
	double dist21 = 1.25;

	for(int eval=0; eval<(signed)files.size(); ++eval)
	{
		CombinedClassifier*	pClas = new CombinedClassifier();

		int j = 0;
		string fname_evaluar;
		for(vector<string>::const_iterator I = files.begin();
			I != files.end();
			++I, ++j)
		{
			if (j==eval)
			{
				fname_evaluar = *I;
				continue;
			}

			if (bEntrenar)	
				eval_train(pClas, vectores[*I], (*I)[0]-'0');
		}
		
		if (bEntrenar)
			pClas->end_train();
		else
		{
			pClas->clear();
			pClas->load("eval_training");
		}

		printf("ENTRENADO; EVALUANDO %s\n", fname_evaluar.c_str());
		int tipo = fname_evaluar[0]-'0';

		////
		vector<double> vHu, vF, v = vectores[fname_evaluar];

		vHu.insert(vHu.begin(), v.begin(), v.begin()+8); //los 8 primeros (7+tamaño)
		vF.insert(vF.begin(), v.begin()+8, v.end()); // los restantes

		int find = pClas->find(v, KHu, lambdaHu, KFourier, lambdaFourier, dist21);
		int findF = pClas->getFourierClas()->find(vF, KFourier, lambdaFourier);
		int findH = pClas->getHuClas()->find(vHu, KHu, lambdaHu);
		////

		if (find==tipo)
		{
			evaluaciones_bien[tipo]++;
			evaluaciones_bien[0]++;
		}
		else
		{
			if (find!=0)
			{
				evaluaciones_mal[tipo]++;
				evaluaciones_mal[0]++;
			}
			else
			{
				evaluaciones_noid[tipo]++;
				evaluaciones_noid[0]++;
			}
		}

		if (findH==tipo)
		{
			evaluaciones_bienH[tipo]++;
			evaluaciones_bienH[0]++;
		}
		
		if (findF==tipo)
		{
			evaluaciones_bienF[tipo]++;
			evaluaciones_bienF[0]++;
		}

		if (tipo!=findF && tipo!=findH)
		{
			evaluaciones_malHF[tipo]++;
			evaluaciones_malHF[0]++;
		}

		evaluaciones_totales[tipo]++;
		evaluaciones_totales[0]++;

		delete pClas;
	}
	
	printf("\n\n****************************************************\n");
	
	printf("KHu = %d;\nlambdaHu = %.1f;\nKFourier = %d;\nlambdaFourier = %.1f;\ndist21 = %.2f\n\n",
			KHu, lambdaHu, KFourier, lambdaFourier, dist21);

	for(int i=0; i<=4; i++)
	{
		int total = evaluaciones_totales[i];
		if (i==0)
			printf("\n\nGLOBAL:\n", i);
		else
			printf("\n\nTIPO %d:\n", i);
		printf("Bien: %.2f (%d/%d)\n", 100.f*evaluaciones_bien[i]/total, evaluaciones_bien[i], total);
		printf("Mal: %.2f (%d/%d)\n", 100.f*evaluaciones_mal[i]/total, evaluaciones_mal[i], total);
		printf("noid: %.2f (%d/%d)\n", 100.f*evaluaciones_noid[i]/total, evaluaciones_noid[i], total);
		printf("\n             BienH: %.2f (%d/%d)\n", 100.f*evaluaciones_bienH[i]/total, evaluaciones_bienH[i], total);
		printf(  "             BienF: %.2f (%d/%d)\n", 100.f*evaluaciones_bienF[i]/total, evaluaciones_bienF[i], total);
		printf(  "                 MalHF: %.2f (%d/%d)\n", 100.f*evaluaciones_malHF[i]/total, evaluaciones_malHF[i], total);
	}
	
	printf("\n\nPulsa 1 tecla para salir...");
	getchar();

	return 0;
}

#else


// MAIN
int main(int argc, char** argv)
{
	Camara* camara = new Camara();
	IplImage* imagenCamara;
	int ident = 0;
	printf("\n");

	CombinedClassifier hkclas_combi;

	if (!hkclas_combi.load(TrainFile))
		printf("No se ha encontrado el training, empezamos de vacio\n");

	printf("\nPulsa una tecla para capturar...\n");
	cvWaitKey(0);


	bool bReleaseImagen = false;

	do
	{
		imagenCamara = camara->queryFrame();
		
		if (!imagenCamara)
		{
			imagenCamara = cvLoadImage("c.jpg", 1);
			bReleaseImagen = true;
		}

		if (!imagenCamara)
		{
			fprintf(stderr, "No se puede abrir la imagen\n");
			return -1;
		}

		DWORD t0 = timeGetTime();

		// Cogido de VisionProcessor, siempre usa Otsu
		Procesador::Config cfg = Procesador::CFG_DEFAULT;
		cfg.wsh_steps = 5; //M era 12
		cfg.wsh_step = 2;
		cfg.wsh_softness = 11; //25;
		cfg.wsh_umbral = 40;
		cfg.th_adjOtsu = 10.; // -5 M era -15.0
		cfg.th_smooth = 25; //M era 29
		cfg.th_threshold = -1; //(max_v>128) ? -1 : max_sxv; //max_v cuando thresholding normal, max_s con th sat

															   // v - SEGMENTACION watershed DESACTIVADA
		Procesador *pbuscar = new Procesador(imagenCamara, cfg, false); 

		//pbuscar = new Procesador(imagenCamara, cfg); 
		pbuscar->display();

		DWORD tBuscar = 0;

		for(int jj=0; pbuscar->objeto(jj)!=NULL; jj++)
		{
			const Objeto* p_obj = pbuscar->objeto(jj);
			p_obj->display(730+20*jj/2+ (jj&1)*25, 310+40*jj/2+ (jj&1)*200);


			printf("\n** tipos( 1=metal, 2=plastico, 3=papel, 4=vidrio**\n\n");
			
			DWORD tBuscar0 = timeGetTime();
			//int find2 = hkclas_fourier.find(p_obj->getDesc_Fourier()->getVector(), 3, 10.); 
			//int find2 = kclas2.find(p_obj->getDesc_HuMoments()->getVector(), 5, 0.);
			int find2 = hkclas_combi.find(p_obj->getCombinedVector());
			
			tBuscar = timeGetTime() - tBuscar0;
			
			printf("find2(pbuscar)=%d\n", find2);

			if (find2>0)
			{
				char* stipo;
				switch(find2) {
					case 1: stipo = "METAL"; break;
					case 2: stipo = "PLASTICO"; break;
					case 3: stipo = "PAPEL"; break;
					case 4: stipo = "VIDRIO"; break;
					default: stipo = "???"; break;
				}
				printf("'%s': IDENTIFICADO OBJETO DE TIPO: %s\n", p_obj->getName(), stipo);
			}
			else
				printf("'%s': NO SE HA IDENTIFICADO EL OBJETO\n", p_obj->getName());

		}

		printf("  Tiempo en buscar: %dms\n", (int)tBuscar);
		printf("- TIEMPO TOTAL: %dms\n", (int)(timeGetTime()-t0));

		printf("\nSi quieres entrenar con el objeto 0 actual, pulsa el numero correspondiente a su tipo\n");
		ident = cvWaitKey(0)-'0';
		printf("x\n");
		if (ident>=1 && ident<=9)
		{
			printf("\nGuardando entrenamiento de un objeto tipo %d\n", ident);
			
			const Objeto* obj = pbuscar->objeto(0);

			if (obj)
			{
				hkclas_combi.train(obj->getCombinedVector(), ident);
				hkclas_combi.end_train();
				hkclas_combi.store(TrainFile);
				
				static char fname[MAX_PATH];
				static int iNombre = 0;
				sprintf(fname, "images\\%d-%d.jpg", ident, iNombre++);
				printf("Guardando imagen del objeto como %s\n", fname);
				cvSaveImage(fname, obj->getImg());
			}
		}

#		ifdef DBG_IMG_INST
		printf("ImagenBn::Instances = %d\n", ImagenBn::Instances);
#		endif
		
		delete pbuscar;

#		ifdef DBG_IMG_INST
		printf("ImagenBn::Instances = %d\n", ImagenBn::Instances);
#		endif

		if (bReleaseImagen)
			cvReleaseImage(&imagenCamara);
	} 
	while((ident+'0')!=27);

	delete camara;

	return 0;
}
#endif