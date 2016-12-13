/*
 * Modulo: Vision artificial
 *
 * Descripcion: Funciones de evaluacion de metodos de clasificacion en consola
 *
 * Autor: Alvaro Salmador
 *
 * (C) Alvaro Salmador 2007-2008. All rights reserved.
 *
 * $Id: evaluation.cpp 400 2008-04-05 23:54:09Z Alvaro $
 */

#include "evaluation.h"




void eval_train(Classifier<vector<double>,int>* pClas, const vector<double>&vector, int tipo_real)
{
	pClas->train(vector, tipo_real);
}


vector<double> eval_vector(const char* fname, Procesador::Config cfg)
{
	IplImage* imagenCamara;
	vector<double> resultado;

	imagenCamara = cvLoadImage(fname, 1);

	if (!imagenCamara)
	{
		fprintf(stderr, "No se puede abrir la imagen\n");
		return resultado;
	}

														   // v - SEGMENTACION watershed DESACTIVADA
	Procesador *pbuscar = new Procesador(imagenCamara, cfg, false); 
	//pbuscar->display();

	const Objeto* obj = pbuscar->objeto(0);

	if (obj)
	{
		resultado = obj->getCombinedVector(); //VECTOR
		//pClas->train(obj->getCombinedVector(), tipo_real); // VECTOR
		//pClas->end_train(); se hjace fuera
	}

	cvReleaseImage(&imagenCamara);

	return resultado;
}



int		eval_buscar(Classifier<vector<double>,int>* pClas, const char* fname, Procesador::Config cfg)
{
	IplImage* imagenCamara;

	imagenCamara = cvLoadImage(fname, 1);

	if (!imagenCamara)
	{
		fprintf(stderr, "No se puede abrir la imagen\n");
		return -1;
	}

														   // v - SEGMENTACION watershed DESACTIVADA
	Procesador *pbuscar = new Procesador(imagenCamara, cfg, false); 
	//pbuscar->display();


	const Objeto* p_obj = pbuscar->objeto(0);
		
	//p_obj->display(730+20*jj/2+ (jj&1)*25, 310+40*jj/2+ (jj&1)*200);

	int find2 = pClas->find(p_obj->getCombinedVector()); // VECTOR
		
		/*printf("find2(pbuscar)=%d\n", find2);

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
		*/

	cvReleaseImage(&imagenCamara);

	return find2;
}