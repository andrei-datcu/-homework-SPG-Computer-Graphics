//-------------------------------------------------------------------------------------------------
// Descriere: header in care sunt implementate functii pentru incarcare de texturi din fisiere BMP
//
// Nota:
//		loadTextureBMP - functia pe care o veti folosi pentru a incarca texturi.
//						 lucreaza cu functii OpenGL pentru a crea o textura (mai mult decat o imagine!)
//                       este functia care trebuie folosita
//---
//		_loadImageBMP  - incarca imaginea in memorie, independenta de OpenGL
//                       rol de functie de suport
//
// Nota2:
//		sunteti incurajati sa va scrieti parsele proprii pentru alte formaturi. Format sugerat: ppm, tga
//
// Autor: Lucian Petrescu
// Data: 28 Sep 2013
//-------------------------------------------------------------------------------------------------

#pragma once
#include "dependente\glew\glew.h"
#include <fstream>
#include <iostream>
#include <string>

namespace lab{

	//definitie forward
	//unsigned char* _loadBMPFile(const std::string &filename, unsigned int &width, unsigned int &height);


	//incarca o imagine BMP si creeaza cu ea o textura
	//aceasta este functia pe care o veti apela
	//returneaza id-ul texturii
	unsigned int loadTextureBMP(const std::string &filename);


    //incarca 6 imagini BMP si creeaza cu ele un cubemap
    unsigned int loadTextureCubemapBMP(const std::string &posx, const std::string &posy, const std::string &posz, const std::string& negx, const std::string& negy, const std::string& negz);



	//nu suporta compresie! 
	//incarca un fisier BMP intr-un array unsigned char
	//returneaza un pointer la un array ce contine datele texturii si valori in argumentele trimise prin referinta width si height
	unsigned char* _loadBMPFile(const std::string &filename, unsigned int &width, unsigned int &height);
}