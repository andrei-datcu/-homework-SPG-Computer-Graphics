
#define _VARIADIC_MAX 8 //damn MSVC
#include "MainWindow.h"
#include "lab_shader_loader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Mesh.h"

#include <ctime>
#include <fstream>

MainWindow::MainWindow(){

	//setari pentru desenare opengl
	glClearColor(0.5,0.5,0.5,1);
	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);

    gl_track_shader = lab::loadShader("shadere\\track_vertex.glsl",
        "shadere\\track_geometry.glsl", "shadere\\track_fragment.glsl");
    RaceTrack::loc_num_points = glGetUniformLocation(gl_track_shader,
        "num_points");

    glUniform1d(glGetUniformLocation(gl_track_shader,"texUnit"), 0);

    topCamera.set(glm::vec3(0, 4000, 0), rt.get_start_pos(),
			glm::vec3(0, 1, 0));
	
	//incarca un shaderele principale
	gl_program_shader = lab::loadShader("shadere\\shader_vertex.glsl",
		"shadere\\shader_fragment.glsl");

	location_model_matrix = glGetUniformLocation(gl_program_shader,
		"model_matrix");
	location_view_matrix = glGetUniformLocation(gl_program_shader,
		"view_matrix");
	location_projection_matrix = glGetUniformLocation(gl_program_shader,
		"projection_matrix");

	//Bloc uniform prin care trimit proprietatiile de material
	glUniformBlockBinding(gl_program_shader,
		glGetUniformBlockIndex(gl_program_shader,"Material"),
		Mesh::shaderMaterialUniformLocation);

	location_texUnit = glGetUniformLocation(gl_program_shader,"texUnit");


	//matrici de modelare si vizualizare

	original_model_matrix = glm::mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
	view_matrix = glm::lookAt(glm::vec3(310, 20, 350), glm::vec3(0,0,0),
		glm::vec3(0,1,0));
	glUniform1d(location_texUnit,0);

	//adaug 2 steaguri
	flags.reserve(2);
	flags.emplace_back(location_model_matrix, original_model_matrix,
		"resurse\\flag5.obj");
	flags.emplace_back(location_model_matrix, original_model_matrix,
		"resurse\\flag5.obj");

    flags[0].translate(rt.get_start_pos() + glm::vec3(-150, 0, 0));
	flags[1].translate(rt.get_start_pos() + glm::vec3(170, 0, 0));

    car = new Car(location_model_matrix, "resurse\\maserati_GTS_sc.3ds", rt);
    currentCamera = &(car->tps_back_camera);
}


MainWindow::~MainWindow(){

	glDeleteProgram(gl_program_shader);
	glDeleteProgram(gl_flag_shader);

	
	for (auto &p : Mesh::texturesId)
		if (p.second > 0)
			glDeleteTextures(1, &p.second);
}


//functie chemata inainte de a incepe cadrul de desenare
void MainWindow::notifyBeginFrame(){
	
	static float dangle = 0.04f; //viteza unghiulara a playerului 0


	//Verific care taste sunt apasate si nu au fost inca ridicate

	for (int key = 0; key < 4; ++key)
		if (specialKeys.test(key))
			switch (key + GLUT_KEY_LEFT){
			case GLUT_KEY_LEFT:
				car->steer(32.0f*dangle);
				break;

			case GLUT_KEY_RIGHT:
				car->steer(-32.0f*dangle);
				break;

			case GLUT_KEY_UP:
				car->accelerate(5.0f * dangle);
				break;

            case GLUT_KEY_DOWN:
                car->accelerate(-25.0f *dangle);
                break;
			}
}

//functia de afisare (lucram cu banda grafica)
void MainWindow::notifyDisplayFrame(){
	//bufferele din framebuffer sunt aduse la valorile initiale
	//adica se sterge ecranul si se pune culoare (si alte propietati) initiala
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    car->update_position();

	glUseProgram(gl_program_shader);
	//trimite variabile uniforme la shader
	glUniformMatrix4fv(location_view_matrix, 1, false,
		glm::value_ptr(currentCamera->getViewMatrix()));
	glUniformMatrix4fv(location_projection_matrix, 1, false,
		glm::value_ptr(projection_matrix));
	glUniform3f(glGetUniformLocation(gl_program_shader, "eye_position"),
		currentCamera->position.x, currentCamera->position.y,
		currentCamera->position.z);
    car->renderObject();

    for (auto &flag : flags)
        flag.renderObject();


    glUseProgram(gl_track_shader);
    glUniformMatrix4fv(glGetUniformLocation(gl_track_shader, "model_matrix"),
        1, false, glm::value_ptr(original_model_matrix));
    glUniformMatrix4fv(glGetUniformLocation(gl_track_shader, "view_matrix"),
        1, false, glm::value_ptr(currentCamera->getViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(gl_track_shader, "projection_matrix"),
        1, false, glm::value_ptr(projection_matrix));
    auto hl = car->headlight_pos();
    glm::vec3 hdir = car->headlight_dir();
    glUniform3f(glGetUniformLocation(gl_track_shader, "light_position1"),
		hl.first.x, hl.first.y, hl.first.z);
    glUniform3f(glGetUniformLocation(gl_track_shader, "light_position2"),
		hl.second.x, hl.second.y, hl.second.z);
	glUniform3f(glGetUniformLocation(gl_track_shader, "light_direction"),
		hdir.x, hdir.y, hdir.z);
    rt.render();

    glUseProgram(0);

    printText("Turul: ", -0.9, 0.9);
    printText(std::to_string(car->laps_completed), -0.7, 0.9);

    printText("Timpul curent: ", -0.9, 0.8);
    printText("Record: ", -0.9, 0.7);

    static auto last_time = std::chrono::high_resolution_clock::now();
    static int last_lap = 0;
    static std::string best_str;
    auto now = std::chrono::high_resolution_clock::now();
    unsigned int  msecs =
        std::chrono::duration_cast<std::chrono::milliseconds>
        (now - last_time).count();

    printText(std::to_string(msecs/60000) + ":" +std::to_string((msecs % 60000)/1000) + "." + std::to_string(msecs % 1000), -0.7, 0.8);
    if (last_lap != car->laps_completed) {
        last_lap = car->laps_completed;
        last_time = now;
        best_str = std::to_string(msecs/60000) + ":" +std::to_string((msecs % 60000)/1000) + "." + std::to_string(msecs % 1000);
    }
    printText(best_str, -0.7, 0.7);

}

//functie chemata dupa ce am terminat cadrul de desenare
void MainWindow::notifyEndFrame(){
}

//functei care e chemata cand se schimba dimensiunea ferestrei initiale
void MainWindow::notifyReshape(int width, int height, int previos_width,
							   int previous_height){
	//reshape
	if(height == 0)
		height=1;
	glViewport(0, 0, width, height);
	projection_matrix = glm::perspective(90.0f, (float)width / (float)height,
		0.1f, 10000.0f);
}


//-----------------------------------------------------------------------------
//functii de input output -----------------------------------------------------

//tasta apasata
void MainWindow::notifyKeyPressed(unsigned char key_pressed, int mouse_x,
								  int mouse_y){

	if(key_pressed == 27)
		lab::glut::close();	//ESC inchide glut si 


	//if (!gameOn || !gameStarted)
	//	return;

	static float dx = 1.1f, dangle = 0.055f;
	//vitezele pentru camera fps de deasupra stadionului

	switch (toupper(key_pressed)){

    case 'W': {
        static bool wf = true;
        if (wf)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        wf = !wf;
        break;
    }

    case 'C':
        currentCamera = &car->tps_back_camera;
        break;

	case 'T':
		//currentCamera = new lab::Camera(*currentCamera);
        currentCamera = &topCamera;
		break;

	case 'I':
		currentCamera->translateForward(dx);
		break;

	case 'K':
		currentCamera->translateForward(-dx);
		break;

	case 'L':
		currentCamera->translateRight(dx);
		break;

	case 'J':
		currentCamera->translateRight(-dx);
		break;

	case 'O':
		currentCamera->translateUpword(dx);
		break;

	case 'P':
		currentCamera->translateUpword(-dx);
		break;

	case '[':
		currentCamera->rotateFPSoX(dangle);
		break;

	case ']':
		currentCamera->rotateFPSoX(-dangle);
		break;

	case ';':
		currentCamera->rotateFPSoY(-dangle);
		break;

	case '\'':
		currentCamera->rotateFPSoY(dangle);
		break;

	case '.':
		currentCamera->rotateFPSoZ(-dangle);
		break;

	case '/':
		currentCamera->rotateFPSoZ(dangle);
		break;
	}
}

//tasta ridicata
void MainWindow::notifyKeyReleased(unsigned char key_released, int mouse_x,
								   int mouse_y){
}


void MainWindow::notifySpecialKeyPressed(int key_pressed, int mouse_x,
										 int mouse_y){

	if(key_pressed == GLUT_KEY_F1) lab::glut::enterFullscreen();
	if(key_pressed == GLUT_KEY_F2) lab::glut::exitFullscreen();

	switch (key_pressed){
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN:	
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
		specialKeys.set(key_pressed - GLUT_KEY_LEFT);
		break;
	}
}
//tasta speciala ridicata
void MainWindow::notifySpecialKeyReleased(int key_released, int mouse_x,
										  int mouse_y){
	switch (key_released){
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN:	
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
		specialKeys.reset(key_released - GLUT_KEY_LEFT);
		break;
	}
}
//drag cu mouse-ul
void MainWindow::notifyMouseDrag(int mouse_x, int mouse_y){ }
//am miscat mouseul (fara sa apas vreun buton)
void MainWindow::notifyMouseMove(int mouse_x, int mouse_y){ }
//am apasat pe un boton
void MainWindow::notifyMouseClick(int button, int state, int mouse_x,
								  int mouse_y){ }

//scroll cu mouse-ul
void MainWindow::notifyMouseScroll(int wheel, int direction, int mouse_x,
								   int mouse_y){
}

void MainWindow::printText(std::string text, float x, float y){

	glColor3f(1.0f, 0.0f, 0.0f);
    glRasterPos2f(x, y);
	for (char c : text)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
}