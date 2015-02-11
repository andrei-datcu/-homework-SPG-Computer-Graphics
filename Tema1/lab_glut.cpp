#include "lab_glut.hpp"
#include <iostream>

lab::glut::WindowInfo lab::glut::_window_information;					//aici sunt tinute informatii despre fereastra
lab::glut::FramebufferInfo lab::glut::_framebuffer_information;		//aici sunt tinute informatii despre framebuffer
lab::glut::ContextInfo lab::glut::_context_information;				//aici sunt tinute informatii despre context
bool lab::glut::_initialized=false;							//e initializat glut?
lab::glut::WindowListener* lab::glut::_listener=NULL;	


static void lab::glut::_idleCallback(){
	//cand GLUT nu face nimic (idle) se apeleaza aceasta functi -> ii spun deseneze din nou
	glutPostRedisplay();
}

static void lab::glut::_displayCallback(){
	//notifica ascultatorul de faptul ca a inceput un nou cadru 
	if(_listener) _listener->notifyBeginFrame();

	//notifica ascultatorul de faptul ca un nou cadru se deseneaza 
	if(_listener) _listener->notifyDisplayFrame();

	//swap buffers
	glutSwapBuffers();

	//cadrul s-a terminat, notifica ascultator
	if(_listener) _listener->notifyEndFrame();
}

void lab::glut::_reshapeCallback(int width, int height){
	if(_window_information.is_reshapable){

		//notifica ascultator
		if(_listener) _listener->notifyReshape(width, height, _window_information.width, _window_information.height);

		//tine minte noile dimensiuni
		_window_information.width = width;	
		_window_information.height= height;
	}
}

//defintie forward pt aceasta functie, e implementata la linia 193
void lab::glut::close();

void lab::glut::_closeCallback(){
	//daca apasati pe buton "X" din dreapta sus a ecranului dorim sa iesim elegant nu cu std::terminate = exit
	close();
}

void lab::glut::_keyboardDownCallback(unsigned char key_pressed, int mouse_x, int mouse_y){
	//notifica ascultator
	if(_listener) _listener->notifyKeyPressed(key_pressed,mouse_x,mouse_y);
}

void lab::glut::_keyboardUpCallback(unsigned char key_released, int mouse_x, int mouse_y){
	//notifica ascultator
	if(_listener) _listener->notifyKeyReleased(key_released,mouse_x,mouse_y);
}

void lab::glut::_specialDownCallback(int key_pressed, int mouse_x, int mouse_y){
	//notifica ascultator
	if(_listener) _listener->notifySpecialKeyPressed(key_pressed,mouse_x,mouse_y);
}

void lab::glut::_specialUpCallback(int key_released, int mouse_x, int mouse_y){
	//notifica ascultator
	if(_listener) _listener->notifySpecialKeyReleased(key_released,mouse_x,mouse_y);
}

void lab::glut::_motionCallback(int mouse_x, int mouse_y){
	//notifica ascultator
	if(_listener) _listener->notifyMouseDrag(mouse_x, mouse_y);
}

void lab::glut::_passiveMotionCallback(int mouse_x, int mouse_y){
	//notifica ascultator
	if(_listener) _listener->notifyMouseMove(mouse_x, mouse_y);
}

void lab::glut::_mouseCallback(int button, int state, int mouse_x, int mouse_y){
	//notifica ascultator
	if(_listener) _listener->notifyMouseClick(button, state, mouse_x, mouse_y);
}

void lab::glut::_mouseWheelCallback(int wheel, int direction, int mouse_x, int mouse_y){
	//notifica ascultator
	if(_listener) _listener->notifyMouseScroll(wheel,direction, mouse_x, mouse_y);
}


//-----------------------------------------------------------------------------------------
//initializare glut, functia necesita 3 parametri
// -window, de tip WindowInfo, descrie fereastra ce va fi oferita
// -context, de tip ContextInfo, ce descrie contextul pe care il cerem de la driver
// -framebuffer, de tip FramebufferInfo, ce descrie formatul datelor din framebufferul oferit de glut
void lab::glut::init(const WindowInfo &window,const ContextInfo &context, const FramebufferInfo &framebuffer){
	//initializam de 2 ori?
	if(_initialized){
		std::cout<<"EROARE! GLUT - nu ma poti initializa de 2 ori, ar rezulta crash/eroare"<<std::endl;
		return;
	}

	//copiaza informatie pentru fereastra,context,framebuffer
	_context_information = context;
	_window_information = window;
	_framebuffer_information = framebuffer;

	//cerem glut un context OpenGL
	glutInitContextVersion(context.major_version, context.minor_version);
	glutInitContextFlags(GLUT_DEBUG);		
	if(context.core) glutInitContextProfile(GLUT_CORE_PROFILE);
	else glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	//argumente fake pentru ca nu folosim glut in linie de comanda
	int fakeargc = 1;		char *fakeargv[] = {"fake",NULL};
	glutInit(&fakeargc, fakeargv);
	glutInitDisplayMode(framebuffer.flags);	
	glutInitWindowPosition(window.start_position_x,window.start_position_y);
	glutInitWindowSize(window.width,window.height);
	glutCreateWindow(window.name.c_str());

	//leaga functiile locale la GLUT
	glutIdleFunc(_idleCallback);
	glutCloseFunc(_closeCallback);
	glutDisplayFunc(_displayCallback);
	glutReshapeFunc(_reshapeCallback);
	glutKeyboardFunc(_keyboardDownCallback);
	glutKeyboardUpFunc(_keyboardUpCallback);
	glutSpecialFunc(_specialDownCallback);
	glutSpecialUpFunc(_specialUpCallback);
	glutMotionFunc(_motionCallback);
	glutPassiveMotionFunc(_passiveMotionCallback);
	glutMouseFunc(_mouseCallback);
	glutMouseWheelFunc(_mouseWheelCallback);

	//scrie la consola diverse detalii utile
	const unsigned char* renderer = glGetString( GL_RENDERER );
	const unsigned char* vendor = glGetString( GL_VENDOR );
	const unsigned char* version = glGetString( GL_VERSION );
	std::cout<<"*******************************************************************************"<<std::endl;
	std::cout<<"GLUT:initializare"<<std::endl;
	std::cout<<"GLUT:\tVendor : "<<vendor<<std::endl;
	std::cout<<"GLUT:\tRenderer : "<<renderer<<std::endl;
	std::cout<<"GLUT:\tutilizez versiunea de OpenGl : "<<version<<std::endl;
	std::cout<<"GLUT:\tFereasta initiala se numeste `"<<window.name<<"`, are dimensiunile  ("<<window.width<<"X"<<window.height;
	std::cout<<") incepe de la coordonatele de ecran ("<<window.start_position_x<<"X"<<window.start_position_y;
	std::cout<<") si "<<((window.is_reshapable)?"este":"nu este")<<" redimensionabila"<<std::endl;
	std::cout<<"GLUT:\tFramebuffer initial contine buffere(duble) pentru"<<std::endl;
	if(glutGet(GLUT_WINDOW_RGBA)){
		int r_bits, g_bits, b_bits, a_bits;
		glGetIntegerv(GL_RED_BITS, &r_bits);	glGetIntegerv(GL_GREEN_BITS, &g_bits);
		glGetIntegerv(GL_BLUE_BITS, &b_bits);	glGetIntegerv(GL_ALPHA_BITS, &a_bits);
		std::cout<<"\tCuloare R"<<r_bits<<"G"<<g_bits<<"B"<<b_bits<<"A"<<a_bits<<std::endl;
	}
	if(_framebuffer_information.flags&GLUT_DEPTH){
		int d_bits;	glGetIntegerv(GL_DEPTH_BITS, &d_bits);
		std::cout<<"\tAdancime DEPTH"<<d_bits<<std::endl;
	}
	if(_framebuffer_information.flags&GLUT_STENCIL){
		int s_bits;	glGetIntegerv(GL_STENCIL_BITS, &s_bits);
		std::cout<<"\tStencil STENCIL"<<s_bits<<std::endl;
	}
	if(_framebuffer_information.flags&GLUT_MULTISAMPLE) std::cout<<"\tmultisampling cu 4 sample-uri per pixel"<<std::endl;
	std::cout<<"GLUT:\tContextul OpenGL este "<<_context_information.major_version<<"."<<_context_information.minor_version;
	std::cout<<" si profilul este de "<<((_context_information.core)?"core":"compatibilitate")<<std::endl;
	std::cout<<"*******************************************************************************"<<std::endl;

	//cand glut este inchis este returnat la main pentru oportunitatea de cleanup corect
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//am terminat cu initializarea!
	_initialized=true;
}

//-----------------------------------------------------------------------------------------
// GLUT incepe sa ruleze (cadru dupa cadru)
void lab::glut::run(){
	if(!_initialized){
		std::cout<<"EROARE! GLUT - nu pot rula daca nu sunt initializat"<<std::endl;							
		return;
	}
	std::cout<<"GLUT:\tRulez"<<std::endl;
	glutMainLoop();											//GLUT incepe un while(true) deseneaza cadru
}
//functia aceasta o apelati cand doriti sa inchideti glut
void lab::glut::close(){
	if(!_initialized) return;
	_initialized=false;
	std::cout<<"GLUT:\tTerminat"<<std::endl;
	glutLeaveMainLoop();
}

//-----------------------------------------------------------------------------------------
// detalii despre framebuffer
const lab::glut::FramebufferInfo& lab::glut::getInitialFramebufferInformation(){
	return _framebuffer_information;
}
//detalii despre fereastra
const lab::glut::WindowInfo& lab::glut::getInitialWindowInformation(){
	return _window_information;
}
//detalii despre context
const lab::glut::ContextInfo& lab::glut::getContextInfo(){
	return _context_information;
}

//-----------------------------------------------------------------------------------------
//fullscreen
void lab::glut::enterFullscreen(){
	if(!_initialized){
		std::cout<<"EROARE! GLUT - nu sunt initializat"<<std::endl;
		return;
	}
	glutFullScreen(); 
}
void lab::glut::exitFullscreen(){
	if(!_initialized){
		std::cout<<"EROARE! GLUT - nu sunt initializat"<<std::endl;
		return;
	}
	glutLeaveFullScreen();
}

//-----------------------------------------------------------------------------------------
//listener
void lab::glut::setListener(WindowListener* listener){
	if(listener) _listener = listener;
}
lab::glut::WindowListener* lab::glut::getListener(){
	return _listener;
}