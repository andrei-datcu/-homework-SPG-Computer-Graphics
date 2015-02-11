
// ImageProcessingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImageProcessing.h"
#include "ImageProcessingDlg.h"
#include "afxdialogex.h"
#include "ManipulateImage.h"
#include "Image.h"
#include "Gaussian.h"
#include "Canny.h"

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
#include <iostream>

//asista la debug pentru leak-uri de memorie
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


int shader_based;
int do_blur;
int do_grayscale;
unsigned char *ImageData;
unsigned char *newImageData;
unsigned char *pixelData;

Image *inImg;
Image *step1Img, *step2Img;

std::string filename;
std::string filenameSaveAs;
int ext,extSaveAs;
bool saveas;
bool image_not_saved;
int prev_width;
int prev_height;

//texturi
	unsigned int texture_color;
	

unsigned int width, height;

class Laborator : public lab::glut::WindowListener{

//variabile
private:
	unsigned int program_shader;														//id-ul de opengl al obiectului de tip program shader

	//ecran
	unsigned int screen_width, screen_height;

	
	unsigned int m_vbo, m_ibo, m_vao, m_num_indices;							//geometrie suport pentru render-to-texture 

//metode
public:
	
	//constructor .. e apelat cand e instantiata clasa
	Laborator(unsigned char *data){

		
		//setari pentru desenare, clear color seteaza culoarea de clear pentru ecran (format R,G,B,A)
		glClearColor(0.5,0.5,0.5,1);
		glClearDepth(1);			//clear depth si depth test (nu le studiem momentan, dar avem nevoie de ele!)
		glEnable(GL_DEPTH_TEST);	//sunt folosite pentru a determina obiectele cele mai apropiate de camera (la curs: algoritmul pictorului, algoritmul zbuffer)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		
		
		texture_color = lab::loadTexture(data,width,height);


		//incarca shader
		program_shader = lab::loadShader("shadere\\shader_vertex.glsl", "shadere\\shader_fragment.glsl");

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
		vertices.push_back(MyVertex(glm::vec3(-1,1,0),glm::vec2(0,0)));
		vertices.push_back(MyVertex(glm::vec3(1,1,0),glm::vec2(1,0)));
		vertices.push_back(MyVertex(glm::vec3(1,-1,0),glm::vec2(1,1)));
		vertices.push_back(MyVertex(glm::vec3(-1,-1,0),glm::vec2(0,1)));
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
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(program_shader);

			glActiveTexture(GL_TEXTURE0+1);
			glBindTexture(GL_TEXTURE_2D, texture_color);
			glUniform1i( glGetUniformLocation(program_shader, "textura_color"), 1 );
			glUniform1i (glGetUniformLocation(program_shader, "do_blur"),do_blur);
			glUniform1i (glGetUniformLocation(program_shader, "do_grayscale"),do_grayscale);
			glUniform1i (glGetUniformLocation(program_shader, "shader_based"),shader_based);
			glUniform1i( glGetUniformLocation(program_shader, "screen_width"), screen_width );
			glUniform1i( glGetUniformLocation(program_shader, "screen_height"), screen_height );

			glBindVertexArray(m_vao);
			glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
		

	}
	//functie chemata dupa ce am terminat cadrul de desenare (poate fi folosita pt modelare/simulare)
	void notifyEndFrame(){
		if (image_not_saved)
		{
			//se apeleaza o singura data, dupa ce s-a apasat butonul Save As,
			//s-au citit pixelii cu glReadPixels in rezolutia initiala a imaginii
			//si s-a facut din nou reshape la rezolutia dinainte de apasarea butonului Save As
			image_not_saved = false;
			int i,j;
			long pointer1,pointer2;


			//se inverseaza coordonata y
			for (i = 0; i < height; i++)
			{
				for (j = 0; j < width; j++)
				{
					pointer1 = ((height - 1 - i) * width + j) * 3;
					pointer2 = (i * width + j) * 3;
					newImageData[pointer1] = pixelData[pointer2];
					newImageData[pointer1+1] = pixelData[pointer2+1];
					newImageData[pointer1+2] = pixelData[pointer2+2];
				}
			}
			lab::saveImage(newImageData,filenameSaveAs,width,height);
			free(pixelData);
		}
		if (saveas) //se executa o singura data, dupa ce s-a apasat
			//butonul de Save As, si s-a facut reshape la fereastra
			//cu rezolutia initiala a imaginii
		{
			
			saveas = false;
			//sa aloca memorie
			pixelData = (unsigned char*) malloc(sizeof(unsigned char) * width * height * 3);
			//se copiaza pixelii din fereastra
			glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,pixelData);
			//se face reshape la fereastra tinand cont de dimensiunile dinainte de 
			//apasarea butonului Save As
			lab::glut::_reshapeWindow(prev_width,prev_height);
			lab::glut::_reshapeCallback(prev_width,prev_height);
			//salvarea efectiva a imaginii se face in frame-ul urmator
			//ca sa nu stam prea mult cu frame-ul cu rezolutia initiala a imaginii deschis
			image_not_saved=true;
		}
			

	
	}
	//functei care e chemata cand se schimba dimensiunea ferestrei initiale
	void notifyReshape(int width, int height, int previous_width, int previous_height){
		
		//reshape
		if(height==0) height=1;
		
		screen_height = height;		screen_width = width;
		glViewport(0,0,width, height);
		float aspect = (float)width / (float)height;
		
		
	}


	//--------------------------------------------------------------------------------------------
	//functii de input output --------------------------------------------------------------------
	
	//tasta apasata
	void notifyKeyPressed(unsigned char key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == 27) lab::glut::close();	//ESC inchide glut si 
		if(key_pressed == 32) {
			//SPACE reincarca shaderele
		
			glDeleteProgram(program_shader);
			program_shader = lab::loadShader("shadere\\shader_vertex.glsl", "shadere\\shader_fragment.glsl");
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
	ON_BN_CLICKED(IDSAVEAS, &CImageProcessingDlg::OnBnClickedSaveas)
	ON_BN_CLICKED(IDBLUR, &CImageProcessingDlg::OnBnClickedBlur)
	ON_BN_CLICKED(IDGRAYSCALE, &CImageProcessingDlg::OnBnClickedGrayscale)
	ON_BN_CLICKED(IDOPENFILE, &CImageProcessingDlg::OnBnClickedOpenfile)
    ON_BN_CLICKED(IDGRAYSCALE2, &CImageProcessingDlg::OnBnClickedGrayscale2)
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

	
    AllocConsole();
    *stdout = *_tfdopen(_open_osfhandle((intptr_t) GetStdHandle(STD_OUTPUT_HANDLE), _O_APPEND), _T("a"));

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
	
	if (ImageData)
		free(ImageData);
	if (newImageData)
		free(newImageData);
	if (lab::glut::_initialized)
		lab::glut::close();
	CDialogEx::OnOK();
}


//daca se apeleaza butonul "Save As"
void CImageProcessingDlg::OnBnClickedSaveas()
{
	if (lab::glut::_initialized)
	{
		//se pot scrie numai fisiere png
		wchar_t*  lpszFilter = _T("PNG Files (*.png)|*.png|");

		CFileDialog dlgFile = CFileDialog(false,0,0,OFN_ENABLESIZING | OFN_HIDEREADONLY,lpszFilter,0,0,true);
	
		if (dlgFile.DoModal() == IDCANCEL) return;

		CString name = dlgFile.GetPathName();
	
		CT2CA pszConvertedAnsiString(name);
		// construct a std::string using the LPCSTR input
		std::string strStd (pszConvertedAnsiString);
		filenameSaveAs = strStd;

		
		if (!shader_based)
		{
			//se salveaza imaginea finala in fisierul filenameSaveAs
			lab::saveImage(newImageData,filenameSaveAs,width,height);
		}
		else
		{
			
			prev_width = lab::glut::_getWindowWidth();
			prev_height = lab::glut::_getWindowHeight();
			lab::glut::_reshapeWindow( width, height);
			lab::glut::_reshapeCallback(width,height);
			saveas = true;
			image_not_saved = false;
			
		}
		
	}
}

//se apeleaza la apasarea butonului "Blur"
void CImageProcessingDlg::OnBnClickedBlur()
{
	if (inImg)
	{
        if (!step1Img)
            step1Img = new Image(fill_main_obj(*inImg));

        step1Img->to_corona(newImageData);
        texture_color = lab::loadTexture(newImageData,width,height);   
	}
}



//se apeleaza la apasarea butonului "Grayscale"
void CImageProcessingDlg::OnBnClickedGrayscale()
{
    if (inImg) {
        Image *src;
        if (step1Img)
            src = step1Img;
        else
            src = inImg;

        if (!step2Img) {
            step2Img = new Image(*src);
            step2Img->thin();
        }

        step2Img->to_corona(newImageData);
        texture_color = lab::loadTexture(newImageData,width,height);
	}
}

//apelata la apasarea butonului Open
void CImageProcessingDlg::OnBnClickedOpenfile()
{
	
	if (!lab::glut::_initialized)
	{
		//daca e deschisa o alta imagine, se dezaloca memorie pentru imaginea initiala si cea finala
		if (ImageData)
			free(ImageData);
		if (newImageData)
			free(newImageData);

        if (inImg) {
            delete inImg;
            inImg = NULL;
        }

        if (step1Img) {
            delete step1Img;
            step1Img = NULL;
        }

        if (step2Img) {
            delete step2Img;
            step2Img = NULL;
        }


		//se pot citi numai imagini jpg,png sau bmp
		wchar_t*  lpszFilter = _T("PNG Files (*.png)|*.png|")_T("JPEG Files (*.jpg)|*.jpg|")_T("Bitmap Files (*.bmp)|*.bmp|");

		CFileDialog dlgFile = CFileDialog(true,0,0,OFN_ENABLESIZING | OFN_HIDEREADONLY,lpszFilter,0,0,true);
	
		if (dlgFile.DoModal() == IDCANCEL) return;

		CString name = dlgFile.GetPathName();
		CT2CA pszConvertedAnsiString(name);
		std::string strStd (pszConvertedAnsiString);
		filename = strStd;
		
		//imaginea initiala si imaginea noua sunt incarcate din fisierul ales
		ImageData = lab::loadImage(filename,width,height);
		newImageData = lab::loadImage(filename,width,height);

        inImg = new Image(ImageData, width, height);

		//se stabileste dimensiunea ecranului
		int swidth,sheight;
		int max_dim = width; int min_dim = height;
		if (height>max_dim) 
		{
			max_dim = height;
			min_dim = width;
		}
		if (max_dim < 600)
		{
			swidth = width;
			sheight = height;
		}
		else
		{
			if (max_dim == width)
			{
				swidth = 600;
				sheight = 600 * height/width;
			}
			else
			{
				sheight = 600;
				swidth = 600 * width/height;
			}
		}
		
		
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
		Laborator mylab(ImageData);
		shader_based = 0;
		do_blur = 0;
		do_grayscale = 0;
	
		lab::glut::setListener(&mylab);
		lab::glut::run();
		
	}
	else
	{
		MessageBox(_T("Inchideti fereastra glut inainte de a deschide o noua imagine"), _T("Error"), MB_ICONERROR | MB_OK);
	}
}


void CImageProcessingDlg::OnBnClickedGrayscale2()
{
    if (inImg) {

        Image *src = step2Img ? step2Img : inImg;

        std::vector<bool> visited(width * height, false);
        Image step3 = src->generate_distance_image(visited);
        step3.normalize();

        step3.to_corona_red(newImageData, visited);
        texture_color = lab::loadTexture(newImageData,width,height);
	}
}
