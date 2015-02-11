//-------------------------------------------------------------------------------------------------
// Descriere: fisier main
//
// Autor: Andrei Datcu
// Data: 13.12.2014
//-------------------------------------------------------------------------------------------------

#define _USE_MATH_DEFINES

#include "lab_mesh_loader.hpp"
#include "lab_geometry.hpp"
#include "lab_shader_loader.hpp"
#include "lab_glut.hpp"
#include "lab_texture_loader.hpp"
#include "lab_camera.hpp"
#include <ctime>
#include <cmath>
#include "ParticleSystem.h"
#include "DynamicCubemap.h"

static void print_error()
{
    GLenum err = glGetError();
    switch (err) 
    {
    case GL_NO_ERROR:
        std::cerr<<"GL_NO_ERROR" << std::endl;
        break;
    case GL_INVALID_ENUM:
        std::cerr << "GL_INVALID_ENUM" << std::endl;
        break;
    case GL_INVALID_VALUE:
        std::cerr << "GL_INVALID_VALUE" << std::endl;
        break;
    case GL_INVALID_OPERATION:
        std::cerr << "GL_INVALID_OPERATION" << std::endl;
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
        break;
    case GL_OUT_OF_MEMORY:
        std::cerr << "GL_OUT_OF_MEMORY" << std::endl;
        break;
    case GL_STACK_UNDERFLOW:
        std::cerr << "GL_STACK_UNDERFLOW" << std::endl;
        break;
    case GL_STACK_OVERFLOW:
        std::cerr << "GL_STACK_OVERFLOW" << std::endl;
        break;        
    }
}


class Laborator : public lab::glut::WindowListener{

//variabile
private:
    lab::Camera camera;
	glm::mat4 projection_matrix;											//matrici 4x4 pt modelare vizualizare proiectie
	unsigned int shader_cubemap, shader_normal;											//id-ul de opengl al obiectului de tip program shader
    unsigned int shader_building;
	//meshe
	lab::Mesh mesh_cubemap;	
    lab::Mesh mesh_building;
    lab::Mesh mesh_windows;
    DynamicCubemap *dc;
	
	//texturi
	unsigned int texture_cubemap, aux_cubemap;
    ParticleSystem particles;
    bool fullscreen;

//metode
public:
	
	//constructor .. e apelat cand e instantiata clasa
    Laborator(): particles(600000), fullscreen(false){
		glClearColor(0.5,0.5,0.5,1);
		glClearDepth(1);
		glEnable(GL_DEPTH_TEST);
		
        shader_normal = lab::loadShader("shadere/normal_vertex.glsl", "shadere/normal_fragment.glsl");
        shader_cubemap = lab::loadShader("shadere/cubemap_vertex.glsl", "shadere/cubemap_fragment.glsl");
        shader_building = lab::loadShader("shadere/building_vertex.glsl", "shadere/building_fragment.glsl");

		lab::loadObj("resurse\\box.obj",mesh_cubemap);	
        lab::loadObj("resurse\\outer_building.obj", mesh_building);
        lab::loadObj("resurse\\windows.obj", mesh_windows);
 
        dc = new DynamicCubemap(glm::vec3(0, 0, 0), 1024, 1024);
        texture_cubemap = lab::loadTextureCubemapBMP("resurse/posx.bmp", "resurse/posy.bmp", "resurse/posz.bmp", "resurse/negx.bmp", "resurse/negy.bmp", "resurse/negz.bmp");
       
        //matrici de modelare si vizualizare
		camera.set(glm::vec3(0,40,60), glm::vec3(0,40,0), glm::vec3(0,1,0));
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//destructor .. e apelat cand e distrusa clasa
	~Laborator(){
		//distruge shadere
		glDeleteProgram(shader_cubemap);
        glDeleteProgram(shader_normal);

		//distruge obiecte
		glDeleteTextures(1, &texture_cubemap);
        delete(dc);
	}
		


	//--------------------------------------------------------------------------------------------
	//functii de cadru ---------------------------------------------------------------------------

	//functie chemata inainte de a incepe cadrul de desenare, o folosim ca sa updatam situatia scenei ( modelam/simulam scena)
    void notifyBeginFrame(){
    }
	//functia de afisare (lucram cu banda grafica)

    void screenshot (std::string filename,int x, int y)
    {// get the image data
        long imageSize = x * y * 3;
        unsigned char *data = new unsigned char[imageSize];
        glReadPixels(0,0,x,y, GL_BGR,GL_UNSIGNED_BYTE,data);// split x and y sizes into bytes
        int xa= x % 256;
        int xb= (x-xa)/256;int ya= y % 256;
        int yb= (y-ya)/256;//assemble the header
        unsigned char header[18]={0,0,2,0,0,0,0,0,0,0,0,0,(char)xa,(char)xb,(char)ya,(char)yb,24,0};

        // write header and data to file
        std::fstream File(filename, std::ios::out | std::ios::binary);
        File.write (reinterpret_cast<char *>(header), sizeof (char)*18);
        File.write (reinterpret_cast<char *>(data), sizeof (char)*imageSize);
        File.close();

        delete[] data;
        data=NULL;
    }

	void notifyDisplayFrame(){
		
		//pe tot ecranul
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        particles.update_positions();

        //Cream un cubemap din framebuffer pe care proiectam particulele
        dc->bind_framebuffer();
        int prevw = glutGet(GLUT_WINDOW_WIDTH), prevh = glutGet(GLUT_WINDOW_HEIGHT);
        glutReshapeWindow(1024,1024);
        glViewport(0, 0, 1024, 1024);
        for (int face = 0; face < 6; ++face) {
            //deseneaza obiectul cubemap
            glm::mat4 d_mm(1), d_pm, d_vm;
            dc->bind_face(face, d_vm, d_pm);
            glUseProgram(shader_normal);
            glUniformMatrix4fv(glGetUniformLocation(shader_normal, "view_matrix"), 1, false, glm::value_ptr(d_vm));
            glUniformMatrix4fv(glGetUniformLocation(shader_normal, "projection_matrix"), 1, false, glm::value_ptr(d_pm));
            glUniformMatrix4fv(glGetUniformLocation(shader_normal, "model_matrix"), 1, false, glm::value_ptr(d_mm));
   
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture_cubemap);
            glUniform1i(glGetUniformLocation(shader_normal, "textura_cubemap"), 0);
            mesh_cubemap.Bind();
            mesh_cubemap.Draw();
   
            particles.bind();
            particles.set_render_shader_vars(d_pm, d_vm);
            particles.draw();
        }

        glutReshapeWindow(prevw,prevh);
        glViewport(0, 0, prevw, prevh);
        
        //Acum desenam in framebufferul default
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//deseneaza obiectul cubemap
		glUseProgram(shader_normal);
        glUniformMatrix4fv(glGetUniformLocation(shader_normal, "view_matrix"), 1, false, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader_normal, "projection_matrix"), 1, false, glm::value_ptr(projection_matrix));
        glUniformMatrix4fv(glGetUniformLocation(shader_normal, "model_matrix"), 1, false, glm::value_ptr(glm::mat4(1)));
    	glActiveTexture(GL_TEXTURE0);
    	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_cubemap);
    
        glUniform1i(glGetUniformLocation(shader_normal, "textura_cubemap"), 0);
        mesh_cubemap.Bind();
        mesh_cubemap.Draw();
        
        //Desenam particulele
        particles.bind();
        particles.set_render_shader_vars(projection_matrix, camera.getViewMatrix());
        particles.draw();

        //Desenam partea din cladire care nu reflecta
        static glm::mat4 building_mm = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(1.0f, 0, 0));
        glUseProgram(shader_building);
        glUniformMatrix4fv(glGetUniformLocation(shader_building, "view_matrix"), 1, false, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader_building, "projection_matrix"), 1, false, glm::value_ptr(projection_matrix));
        glUniformMatrix4fv(glGetUniformLocation(shader_building, "model_matrix"), 1, false, glm::value_ptr(building_mm));
        glUniform3f(glGetUniformLocation(shader_building, "eye_position"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
        mesh_building.Bind();
        mesh_building.Draw();
        
        //Desenam geamurile folosind cubemapul cu particulele proiectate (cel generat la inceput)
        glUseProgram(shader_cubemap);
        glUniformMatrix4fv(glGetUniformLocation(shader_cubemap, "view_matrix"), 1, false, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader_cubemap, "projection_matrix"), 1, false, glm::value_ptr(projection_matrix));
        glUniformMatrix4fv(glGetUniformLocation(shader_cubemap, "model_matrix"), 1, false, glm::value_ptr(building_mm));
        
        dc->bind_texture();
        glUniform1i(glGetUniformLocation(shader_cubemap, "textura_cubemap"), 0);
        glUniform3f(glGetUniformLocation(shader_cubemap, "camera_position"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
        mesh_windows.Bind();
        mesh_windows.Draw();
        

	}
	//functie chemata dupa ce am terminat cadrul de desenare (poate fi folosita pt modelare/simulare)
	void notifyEndFrame(){}
	//functei care e chemata cand se schimba dimensiunea ferestrei initiale
	void notifyReshape(int width, int height, int previos_width, int previous_height){
		//reshape
		if(height==0) height=1;
		float aspect = (float)width / (float)height;
		glViewport(0,0,width,height);
		projection_matrix = glm::perspective(75.0f, aspect,0.1f, 10000.0f);
	}


	//--------------------------------------------------------------------------------------------
	//functii de input output --------------------------------------------------------------------
	
	//tasta apasata
	void notifyKeyPressed(unsigned char key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == 27) lab::glut::close();	//ESC inchide glut si 

        if (key_pressed == 'w') camera.translateForward(1.0f);
        if (key_pressed == 'a') camera.translateRight(-1.0f);
        if (key_pressed == 's') camera.translateForward(-1.0f);
        if (key_pressed == 'd') camera.translateRight(1.0f);
        if (key_pressed == 'q') camera.rotateFPSoY(1.0f);
        if (key_pressed == 'e') camera.rotateFPSoY(-1.0f);
        if (key_pressed == 'r') camera.translateUpword(1.0f);
        if (key_pressed == 'f') camera.translateUpword(-1.0f);
	}
	//tasta ridicata
	void notifyKeyReleased(unsigned char key_released, int mouse_x, int mouse_y){	}
	//tasta speciala (up/down/F1/F2..) apasata
	void notifySpecialKeyPressed(int key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == GLUT_KEY_F1){
            lab::glut::enterFullscreen();
            fullscreen = true;
        }
		if(key_pressed == GLUT_KEY_F2){
            lab::glut::exitFullscreen();
            fullscreen = false;
        }
	}
	//tasta speciala ridicata
	void notifySpecialKeyReleased(int key_released, int mouse_x, int mouse_y){}
	//drag cu mouse-ul
	void notifyMouseDrag(int mouse_x, int mouse_y){ }
	//am miscat mouseul (fara sa apas vreun buton)
	void notifyMouseMove(int mouse_x, int mouse_y){ }
	//am apasat pe un boton
	void notifyMouseClick(int button, int state, int mouse_x, int mouse_y){ }
	//scroll cu mouse-ul
	void notifyMouseScroll(int wheel, int direction, int mouse_x, int mouse_y){ }

};

int main(){
	//initializeaza GLUT (fereastra + input + context OpenGL)
	lab::glut::WindowInfo window(std::string("Tema2"),800,600,100,100,true);
	lab::glut::ContextInfo context(3,3,false);
	lab::glut::FramebufferInfo framebuffer(true,true,true,true);
	lab::glut::init(window,context, framebuffer);

	//initializeaza GLEW (ne incarca functiile openGL, altfel ar trebui sa facem asta manual!)
	glewExperimental = true;
	glewInit();
	std::cout<<"GLEW:initializare"<<std::endl;

	//creem clasa noastra si o punem sa asculte evenimentele de la GLUT
	//DUPA GLEW!!! ca sa avem functiile de OpenGL incarcate inainte sa ii fie apelat constructorul (care creeaza obiecte OpenGL)
	Laborator mylab;
	lab::glut::setListener(&mylab);

	//taste
    std::cout << "Taste:" << std::endl << "\tESC ... iesire" << std::endl << "\tSPACE ... reincarca shadere" << std::endl;

	//run
	lab::glut::run();

	return 0;
}