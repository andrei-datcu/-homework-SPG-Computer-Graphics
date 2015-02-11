//-------------------------------------------------------------------------------------------------
// Descriere: fisier main
//
// Autor: student
// Data: today
//-------------------------------------------------------------------------------------------------

//interfata cu glut, ne ofera fereastra, input, context opengl
#include <glew/glew.h>
#include "lab_glut.hpp"
#include "MainWindow.h"
#include <iostream>

//time
#include <ctime>

int main(){
	//initializeaza GLUT (fereastra + input + context OpenGL)
	lab::glut::WindowInfo window(std::string("Tema1 - Datcu Andrei"),
		800, 600, 100, 100,true);
	lab::glut::ContextInfo context(3,3,false);
	lab::glut::FramebufferInfo framebuffer(true,true,true,true);
	lab::glut::init(window,context, framebuffer);

	//initializeaza GLEW (ne incarca functiile openGL)
	glewExperimental = GL_TRUE;
	glewInit();

	std::cout<<"GLEW:initializare"<<std::endl;

	//creem clasa noastra si o punem sa asculte evenimentele de la GLUT
	MainWindow mw;
	lab::glut::setListener(&mw);

	//run
	lab::glut::run();

	return 0;
}