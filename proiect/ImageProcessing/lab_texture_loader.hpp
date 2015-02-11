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



namespace lab{



	//incarca o imagine i creeaza cu ea o textura
	//aceasta este functia pe care o veti apela
	//returneaza id-ul texturii
	unsigned int loadTexture(unsigned char* data,unsigned int width, unsigned height){

		

		//creeaza textura OpenGL
		unsigned int gl_texture_object;
		glGenTextures(1, &gl_texture_object);
		glBindTexture(GL_TEXTURE_2D, gl_texture_object);

		//filtrare
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		float maxAnisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

		//cand lucram cu texturi cu dimensiuni non multiple de 4 trebuie sa facem cititorul de randuri
		//ce incarca texturile in OpenGL sa lucreze cu memorie aliniata la 1 (default este la 4)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		//genereaza textura
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

		//creaza ierarhia de mipmapuri
		glGenerateMipmap(GL_TEXTURE_2D);

		//returneaza obiectul textura
		return gl_texture_object;
	}



	

	

}