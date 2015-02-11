#pragma once

#include "dependente\corona\corona.h"
#include <fstream>
#include <iostream>
#include <string>


namespace lab
{
		

	void saveImage(unsigned char* data,const std::string &filename,unsigned int width,unsigned int height)
	{
		CString str;
		str.Format(_T("%d %d \n"), width,height);
		TRACE(str);
			

		corona::Image *myImage = corona::CreateImage(width,height,corona::PF_R8G8B8,data);
		corona::SaveImage(filename.c_str(),corona::FF_PNG,myImage);
		

		delete myImage;
		

	}

	unsigned char* loadImage(const std::string &filename, unsigned int &width, unsigned int &height){


		
		unsigned char *imageData;
	

		//deschide fisierul BMP cu corona
		corona::Image* coronaImg = corona::OpenImage(filename.c_str(), corona::PF_R8G8B8);
		if (!coronaImg) 
		{
			std::cout<<" eroare la deschiderea fisierului "<<filename.c_str()<<std::endl;
			system("PAUSE");
			exit(-1);
		}	

		

		width= coronaImg->getWidth();
		height= coronaImg->getHeight();

		std::cout<<"width="<<width<<" height="<<height<<std::endl;



		//aloca memorie pentru imagine
		imageData =(unsigned char*)malloc(sizeof(unsigned char)*width*height*3);



	
		//obtine pixelii
		unsigned char* pixeliCorona = (unsigned char*)coronaImg->getPixels();


		long pointer;
		unsigned char r, g, b;

		//salveaza pixelii in imagine
		for (unsigned int i = 0; i < height; i++)
		{
			for (unsigned int j = 0; j < width; j++)
			{
				r = *pixeliCorona++;
				g = *pixeliCorona++;
				b = *pixeliCorona++;
			
				pointer=(i*width+j)*3;
			
				imageData[pointer]=r; 
				imageData[pointer+1]=g; 
				imageData[pointer+2]=b;
			}
		}
		
		delete coronaImg;

		//intoarce imaginea
		return imageData;
	}

}