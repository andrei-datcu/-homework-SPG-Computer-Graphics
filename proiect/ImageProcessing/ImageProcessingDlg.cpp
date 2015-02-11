
// ImageProcessingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VideoManager.h"
#include "ImageProcessing.h"
#include "ImageProcessingDlg.h"
#include "afxdialogex.h"

#include "dependente\glm\glm.hpp"
//incarcator de shadere
#include "lab_shader_loader.hpp"
//interfata cu glut, ne ofera fereastra, input, context opengl
#include "lab_glut.hpp"
//texturi
#include "lab_texture_loader.hpp"
//time
#include <ctime>
#include <fcntl.h>
#include <io.h>
#include <chrono>

#include "Object3D.h"
#include "lab_camera.hpp"
#include <forward_list>


//asista la debug pentru leak-uri de memorie
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


std::string filename;
//texturi
	unsigned int texture_color;
	
class Laborator : public lab::glut::WindowListener{

//variabile
private:
	unsigned int program_shader, obj_shader;
    //id-ul de opengl al obiectului de tip program shader

	//ecran
	unsigned int screen_width, screen_height;

	
	unsigned int m_vbo, m_ibo, m_vao, m_num_indices;
    //geometrie suport pentru render-to-texture 

    VideoManager vm;

    lab::Camera cam;

    std::forward_list<Object3D> objects;
    glm::mat4 projection_matrix;
//metode
public:
	
	//constructor .. e apelat cand e instantiata clasa
	Laborator(const std::string &filename) : vm(filename){

		
		//setari pentru desenare, clear color seteaza culoarea de clear pentru ecran (format R,G,B,A)
		glClearColor(0.5,0.5,0.5,1);
		glClearDepth(1);			//clear depth si depth test (nu le studiem momentan, dar avem nevoie de ele!)
		glEnable(GL_DEPTH_TEST);	//sunt folosite pentru a determina obiectele cele mai apropiate de camera (la curs: algoritmul pictorului, algoritmul zbuffer)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
	    //glEnable(GL_MULTISAMPLE);
		
		
        //incarca shader
		program_shader = lab::loadShader("shadere\\shader_vertex.glsl", "shadere\\shader_fragment.glsl");
        obj_shader = lab::loadShader("shadere\\shader_vertex_obj.glsl",
		"shadere\\shader_fragment_obj.glsl");
        glUniformBlockBinding(obj_shader, glGetUniformBlockIndex(obj_shader,"Material"),
		    Mesh::shaderMaterialUniformLocation);

        unsigned int location_model_matrix = glGetUniformLocation(obj_shader,
		    "model_matrix");
        objects.emplace_front(location_model_matrix, glm::mat4(1),
		"resurse\\room_good.obj");
        objects.emplace_front(location_model_matrix, glm::mat4(1),
		"resurse\\walls.obj");
        cam = lab::Camera(glm::vec3(0, 65,-2), glm::vec3(0,65,20), glm::vec3(0,1,0));
		//pentru afisarea imaginii trebuie sa desenam un quad cat tot ecranul cu coordonate de texturare, pe care sa mapam
		//textura imaginea incarcata
		
		glGenVertexArrays(1,&m_vao);
		glBindVertexArray(m_vao);
		struct MyVertex{ 
			glm::vec3 position;
			glm::vec2 texcoord;
			MyVertex(const glm::vec3 &pos, const glm::vec2 &tc){ position = pos; texcoord = tc; }
		};
		std::vector<MyVertex> vertices; //4 vertecsi in coordonate NDC
        float xoff = -93.3f, yoff=86.4f, width = 122.7f, height = 68.2f, zpos = 99.6f;//100;
		vertices.push_back(MyVertex(glm::vec3(xoff,yoff + height,zpos),glm::vec2(1,0)));
		vertices.push_back(MyVertex(glm::vec3(xoff + width,yoff + height,zpos),glm::vec2(0,0)));
		vertices.push_back(MyVertex(glm::vec3(xoff + width,yoff,zpos),glm::vec2(0,1)));
		vertices.push_back(MyVertex(glm::vec3(xoff,yoff,zpos),glm::vec2(1,1)));
		std::vector<unsigned int> indices; indices.push_back(0);indices.push_back(1);indices.push_back(2);
		indices.push_back(2);indices.push_back(3);indices.push_back(0);	//6 indecsi formeaza 2 triunghiuri
		glGenBuffers(1,&m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(MyVertex)*4, &vertices[0], GL_STATIC_DRAW);
		glGenBuffers(1,&m_ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*6, &indices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void*)0);
		glEnableVertexAttribArray(2);glVertexAttribPointer(2,2,GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void*)sizeof(glm::vec3));
		m_num_indices=6;
        glBindVertexArray(0);
	}

	//destructor .. e apelat cand e distrusa clasa
	~Laborator(){
		
		//distruge geometrie suport pentru afisarea imaginii
		glDeleteBuffers(1,&m_vbo);	glDeleteBuffers(1,&m_ibo);	glDeleteVertexArrays(1,&m_vao);

		//distruge shader
		glDeleteProgram(program_shader);

	}


	
	//--------------------------------------------------------------------------------------------
	//functii de cadru ---------------------------------------------------------------------------

	//functie chemata inainte de a incepe cadrul de desenare, o folosim ca sa updatam situatia scenei ( modelam/simulam scena)
	void notifyBeginFrame(){}
	//functia de afisare (lucram cu banda grafica)
	void notifyDisplayFrame(){
	
		//desenam un quad in NDC texturat cu textura imaginea incarcata
		
			//clear screen
			glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

            glUseProgram(obj_shader);

            glUniformMatrix4fv(glGetUniformLocation(obj_shader, "projection_matrix"),
                1, false, glm::value_ptr(projection_matrix));

            glUniformMatrix4fv(glGetUniformLocation(obj_shader, "view_matrix"),
                1, false, glm::value_ptr(cam.getViewMatrix()));

            for (Object3D &o : objects)
		        o.renderObject();
                
			glUseProgram(program_shader);

			glActiveTexture(GL_TEXTURE0);
            //auto start = std::chrono::system_clock::now();
            glBindTexture(GL_TEXTURE_2D + 0, vm.get_next_frame_tex());
            //auto end = std::chrono::system_clock::now();
            //std::cout << "Dur : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
            glUniform1i( glGetUniformLocation(program_shader, "textura_color"), 0 );
            glUniformMatrix4fv(glGetUniformLocation(program_shader, "projection_matrix"),
                1, false, glm::value_ptr(projection_matrix));

            static int fn = 0, cv = 1;
            fn++;
            if (fn % 100 == 0)
                cv ^=1;
            glUniformMatrix4fv(glGetUniformLocation(program_shader, "view_matrix"),
                1, false, glm::value_ptr(cam.getViewMatrix()));
			glBindVertexArray(m_vao);
			glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
            glBindVertexArray(0);
		

	}
	//functie chemata dupa ce am terminat cadrul de desenare (poate fi folosita pt modelare/simulare)
	void notifyEndFrame(){	}
	//functei care e chemata cand se schimba dimensiunea ferestrei initiale
	void notifyReshape(int width, int height, int previous_width, int previous_height){
		
		//reshape
		if(height==0) height=1;
		
		screen_height = height;		screen_width = width;
		glViewport(0,0,width, height);
		float aspect = (float)width / (float)height;
		
		projection_matrix = glm::perspective(90.0f, (float)width / (float)height,
		    0.1f, 10000.0f);
	}


	//--------------------------------------------------------------------------------------------
	//functii de input output --------------------------------------------------------------------
	
	//tasta apasata
	void notifyKeyPressed(unsigned char key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == 27) lab::glut::close();	//ESC inchide glut si 

        static const float dx = 5.0f;
        static const float dangle = 0.05f;

        switch (toupper(key_pressed)) {
        case 'W':
            cam.translateForward(dx);
            break;

        case 'S':
            cam.translateForward(-dx);
            break;

        case 'D':
            cam.translateRight(dx);
            break;

        case 'A':
            cam.translateRight(-dx);
            break;

        case 'O':
            cam.translateUpword(dx);
            break;

        case 'L':
            cam.translateUpword(-dx);
            break;

        case '[':
            cam.rotateFPSoX(-dangle);
            break;

        case ']':
            cam.rotateFPSoX(dangle);
            break;

        case ';':
            cam.rotateFPSoY(-dangle);
            break;

        case '\'':
            cam.rotateFPSoY(dangle);
            break;

        case '.':
            cam.rotateFPSoZ(-dangle);
            break;

        case '/':
            cam.rotateFPSoZ(dangle);
            break;
        }

	}
	//tasta ridicata
	void notifyKeyReleased(unsigned char key_released, int mouse_x, int mouse_y){	}
	//tasta speciala (up/down/F1/F2..) apasata
	void notifySpecialKeyPressed(int key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == GLUT_KEY_F1) lab::glut::enterFullscreen();
		if(key_pressed == GLUT_KEY_F2) lab::glut::exitFullscreen();
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



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CImageProcessingDlg dialog



CImageProcessingDlg::CImageProcessingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CImageProcessingDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CImageProcessingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CImageProcessingDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CImageProcessingDlg::OnBnClickedExit)
	ON_BN_CLICKED(IDOPENFILE, &CImageProcessingDlg::OnBnClickedOpenfile)
END_MESSAGE_MAP()


// CImageProcessingDlg message handlers

BOOL CImageProcessingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

    AllocConsole();
    *stdout = *_tfdopen(_open_osfhandle((intptr_t) GetStdHandle(STD_OUTPUT_HANDLE), _O_APPEND), _T("a"));

    VideoManager::registerall();


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CImageProcessingDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CImageProcessingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CImageProcessingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//la apasarea butonului "Exit"
void CImageProcessingDlg::OnBnClickedExit()
{
	
	if (lab::glut::_initialized)
		lab::glut::close();
	CDialogEx::OnOK();
}




//apelata la apasarea butonului Open
void CImageProcessingDlg::OnBnClickedOpenfile()
{
	
	if (!lab::glut::_initialized)
	{

		//se pot citi numai imagini jpg,png sau bmp
		wchar_t*  lpszFilter = _T("Video Files (*.wmv;*.mkv)|*.wmv;*.mkv");//_T("PNG Files (*.png)|*.png|")_T("Bitmap Files (*.bmp)|*.bmp|");

		CFileDialog dlgFile = CFileDialog(true,0,0,OFN_ENABLESIZING | OFN_HIDEREADONLY,lpszFilter,0,0,true);
	
		if (dlgFile.DoModal() == IDCANCEL) return;

		CString name = dlgFile.GetPathName();
		CT2CA pszConvertedAnsiString(name);
		std::string strStd (pszConvertedAnsiString);
		
        int swidth = 1280, sheight = 720;
		
		//initializeaza GLUT (fereastra + input + context OpenGL)
		lab::glut::WindowInfo window(std::string("Laborator procesare imagini"),swidth,sheight,100,100,true);
		lab::glut::ContextInfo context(3,3,false);
		lab::glut::FramebufferInfo framebuffer(true,true,true,true);
		lab::glut::init(window,context, framebuffer);

		
		//initializeaza GLEW (ne incarca functiile openGL, altfel ar trebui sa facem asta manual!)
		glewExperimental = true;
		glewInit();
		std::cout<<"GLEW:initializare"<<std::endl;
	
		//creem clasa noastra si o punem sa asculte evenimentele de la GLUT
		//DUPA GLEW!!! ca sa avem functiile de OpenGL incarcate inainte sa ii fie apelat constructorul (care creeaza obiecte OpenGL)
		
		//Laborator mylab;
		Laborator mylab(strStd);

		lab::glut::setListener(&mylab);
		lab::glut::run();
		
	}
	else
	{
		MessageBox(_T("Inchideti fereastra glut inainte de a deschide o noua imagine"), _T("Error"), MB_ICONERROR | MB_OK);
	}
}
