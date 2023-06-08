/*---------------------------------------------------------*/
/* ----------------  Proyecto Final -----------*/
/*-----------------    2023-2   ---------------------------*/
/*------------- Alumno: Milner Ushuaia Flores Perez , Ochoa Fernández Francine ---------------*/
/*-------------Se corrio y programo este codigo usando Visual Studio 2022---------------*/
#include <Windows.h>

#include <glad/glad.h>
#include <glfw3.h>	//main
#include <stdlib.h>		
#include <glm/glm.hpp>	//camera y model
#include <glm/gtc/matrix_transform.hpp>	//camera y model
#include <glm/gtc/type_ptr.hpp>
#include <time.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>	//Texture

#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>

#include <shader_m.h>
#include <camera.h>
#include <modelAnim.h>
#include <model.h>
#include <Skybox.h>
#include <iostream>
#include <fstream>
#include <string>

//#pragma comment(lib, "winmm.lib")

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void my_input(GLFWwindow *window);
void my_input(GLFWwindow* window, int key, int scancode, int action, int mods);
void animate(void);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWmonitor *monitors;

void getResolution(void);

// camera
Camera camera(glm::vec3(0.0f, 25.0f, 1600.0f));
float MovementSpeed = 0.1f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
const int FPS = 60;
const int LOOP_TIME = 1000 / FPS; // = 16 milisec // 1000 millisec == 1 sec
double	deltaTime = 0.0f,
		lastFrame = 0.0f;

//Lighting
glm::vec3 lightPosition(0.0f, 4.0f, -10.0f); //posicion de la luz puntual inicial
glm::vec3 lightDirection(0.0f, -1.0f, -1.0f);//rayos de luz llegaran en esta direccion

//Variables de color para luz posicional estatica pointLight[1]
glm::vec3 varColorAmbient(0.3f, 0.3f, 0.0f);
glm::vec3 varColorDiffuse(1.0f, 1.0f, 1.0f);

// posiciones
float	movAuto_x = 0.0f,
		movAuto_z = 0.0f,
		movAuto_y = 0.0f,
		orienta = 0.0f;
bool	animacion = false,
		recorrido1 = true,
		recorrido2 = false,
		recorrido3 = false,
		recorrido4 = false;

bool	avanza = true,
		arriba = false;

float myVariable = 0.0f;

//Keyframes (Manipulación y dibujo)
float	posX = 0.0f,
		posY = 0.0f,
		posZ = 0.0f,
		mov_patas_caminar = 0.0f,
		giroMonito = 0.0f,
		movCabeza = 0.0f;

float	incX = 0.0f,
		incY = 0.0f,
		incZ = 0.0f,
		mov_patas_caminar_Inc = 0.0f,
		giroMonitoInc = 0.0f,
		movCabezaInc = 0.0f;



#define MAX_FRAMES 50
int i_max_steps = 60;
int i_curr_steps = 0;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float posX;		//Variable para PosicionX
	float posY;		//Variable para PosicionY
	float posZ;		//Variable para PosicionZ
	float mov_patas_caminar;
	float giroMonito;
	float movCabeza = 0.0f;

}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;			//introducir número en caso de tener Key guardados
bool play = false;
int playIndex = 0;

std::ofstream archivo("animacion.txt");

void saveFrame(void)
{
	//printf("frameindex %d\n", FrameIndex);
	std::cout << "Frame Index = " << FrameIndex << std::endl;

	KeyFrame[FrameIndex].posX = posX;
	KeyFrame[FrameIndex].posY = posY;
	KeyFrame[FrameIndex].posZ = posZ;

	KeyFrame[FrameIndex].mov_patas_caminar = mov_patas_caminar;
	KeyFrame[FrameIndex].giroMonito = giroMonito;
	KeyFrame[FrameIndex].movCabeza = movCabeza;

	FrameIndex++;

	//Orden de guardado sera posX, posY, posZ, rotRodIzq, giroMonito, movBrazoDer, movBrazoIzq, movCabeza
	if (archivo.is_open())
	{
		archivo << posX << " " << posY << " " << posZ << " " << mov_patas_caminar << " " << giroMonito << " " << movCabeza << std::endl;
	}
}

void resetElements(void)
{
	std::cout << "resetElements: " << KeyFrame[0].posX << KeyFrame[0].posY
		<< KeyFrame[0].posZ << KeyFrame[0].mov_patas_caminar
		<< KeyFrame[0].giroMonito << KeyFrame[0].movCabeza << std::endl;

	posX = KeyFrame[0].posX;
	posY = KeyFrame[0].posY;
	posZ = KeyFrame[0].posZ;

	mov_patas_caminar = KeyFrame[0].mov_patas_caminar;
	giroMonito = KeyFrame[0].giroMonito;
	movCabeza= KeyFrame[0].movCabeza;
}

void interpolation(void)
{
	incX = (KeyFrame[playIndex + 1].posX - KeyFrame[playIndex].posX) / i_max_steps;
	incY = (KeyFrame[playIndex + 1].posY - KeyFrame[playIndex].posY) / i_max_steps;
	incZ = (KeyFrame[playIndex + 1].posZ - KeyFrame[playIndex].posZ) / i_max_steps;

	mov_patas_caminar_Inc = (KeyFrame[playIndex + 1].mov_patas_caminar - KeyFrame[playIndex].mov_patas_caminar) / i_max_steps;
	giroMonitoInc = (KeyFrame[playIndex + 1].giroMonito - KeyFrame[playIndex].giroMonito) / i_max_steps;
	movCabezaInc = (KeyFrame[playIndex + 1].movCabeza - KeyFrame[playIndex].movCabeza) / i_max_steps;

}

void animate(void)
{
	//La luz posicional se moveria de manera circular automaticamente
	//lightPosition.x = 250.0f * cos(myVariable);
	//lightPosition.z = 250.0f * sin(myVariable);
	//myVariable += 0.1f;

	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				std::cout << "Animation ended" << std::endl;
				//printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
								  //Interpolation
				interpolation();
			}
		}
		else
		{
			//Draw animation
			posX += incX;
			posY += incY;
			posZ += incZ;

			mov_patas_caminar += mov_patas_caminar_Inc;
			giroMonito += giroMonitoInc;
			movCabeza += movCabezaInc;

			i_curr_steps++;
		}
	}

	//Vehículo
	//No meter mas ciclos por que ya estamos  en un ciclo
	if (animacion)//booleano
	{
		if (arriba == false)
		{
			if (avanza)//va a avanzar hasta la posicion 100
			{
				movAuto_z += 3.0f;
				if (movAuto_z >= 100.0f)
					avanza = false;

			}
			else//Si ya llego a 100
			{
				movAuto_z -= 3.0f;
				if (movAuto_z <= 0.0f)
				{
					avanza = true;
					arriba = true;
				}

			}
		}
		else
		{

			if (avanza)//va a subir hasta la posicion 120
			{
				movAuto_y += 3.0f;
				if (movAuto_y >= 120.0f)
					avanza = false;

			}
			else//Si ya llego a 120
			{
				movAuto_y -= 3.0f;
				if (movAuto_y <= 0.0f)
				{
					avanza = true;
					arriba = false;
					animacion = false;
				}

			}

		
			
		}

	}



}

void getResolution()
{
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	SCR_WIDTH = mode->width;
	SCR_HEIGHT = (mode->height) - 80;
}


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	// --------------------
	monitors = glfwGetPrimaryMonitor();
	getResolution();

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CGeIHC", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 0, 30);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, my_input);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader staticShader("Shaders/shader_Lights.vs", "Shaders/shader_Lights_mod.fs");
	Shader skyboxShader("Shaders/skybox.vs", "Shaders/skybox.fs");
	Shader animShader("Shaders/anim.vs", "Shaders/anim.fs");

	vector<std::string> faces
	{
		"resources/skybox/right.jpg",
		"resources/skybox/left.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg"
	};

	Skybox skybox = Skybox(faces);

	// Shader configuration
	// --------------------
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// load models
	// -----------

	//*********************************************************** Hábitats y Mundo ***********************************************************
	Model piso("resources/objects/piso_nuevo/piso_nuevo.obj");
	Model pasillo("resources/objects/piso_concreto/tramo_concreto.obj");

	Model habitat_pinguinos("resources/objects/Habitat_Pinguino_Opt/HabitatPinguinoOpt.obj");
	Model habitat_leones("resources/objects/Habitat_Leon3.0/habitat.obj");
	Model habitat_osos("resources/objects/Habitat_Oso3.0/escenaOso.obj");
	Model zona_juegos("resources/objects/zona_juegos/escenaJuegos.obj");
	Model zona_comida("resources/objects/zona_comida/zona_comida.obj");
	Model reja("resources/objects/Reja/Reja.obj");
	Model zonaAfuera("resources/objects/ZonaAfuera/ZonaAfuera.obj");

	//*********************************************************** Objetos ***********************************************************
	Model helados("resources/objects/Helados/helados.obj");
	Model fuente("resources/objects/fuente/fuente.obj");
	Model letrero("resources/objects/Letrero/Letrero.obj");
	Model cobra("resources/objects/Cobra/Cobra.obj");

	//*********************************************************** León Keyframes ***********************************************************
	Model cabeza("resources/objects/leon_descuartizado2.0/cabeza.obj");
	Model torso("resources/objects/leon_descuartizado2.0/torso.obj");
	Model brazoDer("resources/objects/leon_descuartizado2.0/pata_delante_derecha.obj");
	Model brazoIzq("resources/objects/leon_descuartizado2.0/pata_delante_izquierda.obj");
	Model piernaDer("resources/objects/leon_descuartizado2.0/pata_trasera_derecha.obj");
	Model piernaIzq("resources/objects/leon_descuartizado2.0/pata_trasera_izquierda.obj");


	////*********************************************************** Carga de animaciones //***********************************************************
	ModelAnim pinguinoVolador("resources/objects/AnimacionPinguinoVolador/PinguinoVolador.fbx");
	pinguinoVolador.initShaders(animShader.ID);
	
	ModelAnim osoSaludo("resources/objects/AnimacionOso/OsoSaludando.fbx");
	osoSaludo.initShaders(animShader.ID);

	ModelAnim amyDance("resources/objects/Amy/Amy_Dancing.dae");
	amyDance.initShaders(animShader.ID);


	ModelAnim policiaVigilando("resources/objects/AnimacionPolicia/PoliciaVigilando.fbx");
	policiaVigilando.initShaders(animShader.ID);

	ModelAnim leonRugido("resources/objects/AnimacionLeon/LeonRugido.fbx");
	leonRugido.initShaders(animShader.ID);

	//Inicialización de KeyFrames
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].posX = 0;
		KeyFrame[i].posY = 0;
		KeyFrame[i].posZ = 0;
		KeyFrame[i].mov_patas_caminar = 0;
		KeyFrame[i].giroMonito = 0;
		KeyFrame[i].movCabeza = 0;
	}

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		skyboxShader.setInt("skybox", 0);
		
		// per-frame time logic
		// --------------------
		lastFrame = SDL_GetTicks();

		// input
		// -----
		//my_input(window);
		animate();

		// render
		// ------
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		staticShader.use();
		//Setup Advanced Lights
		staticShader.setVec3("viewPos", camera.Position);
		staticShader.setVec3("dirLight.direction", lightDirection); //lightDirection vector que indica direccion en que llegan rayos de luz
		staticShader.setVec3("dirLight.ambient", glm::vec3(0.6f, 0.6f, 0.6f));//dirLight.ambient le da iluminacion al ambiente(yo no la controlo) 
																			  //todas las caras se ven iluminadas con la misma intensidad. 
																			  //(siento que solo es como hacer que no se vean grises(o el color que se este usando) los colores)
		staticShader.setVec3("dirLight.diffuse", glm::vec3(0.0f, 0.0f, 0.0f));//dirLight.diffuse afecta dada una fuente de luz dependiento las componentes
																		      //en (0.0,0.0,0.0) se ve negro todo en el escenario de iluminacion
		staticShader.setVec3("dirLight.specular", glm::vec3(0.0f, 0.0f, 0.0f));//dirLight.specular imaginar que se imprime todo como una foto, 
																		       //la foto se vera diferente dependiendo del tipo de papel que se use
																			   // es como el reflejo que se ve en las fotos impresas de papel brilloso
		staticShader.setVec3("pointLight[0].position", lightPosition);//lightPosition vector que indica posicion de la luz puntual inicial
		staticShader.setVec3("pointLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[0].constant", 0.08f);
		staticShader.setFloat("pointLight[0].linear", 0.009f); //ayudan a que los  rayos de luz vijen una mayor ditancia.
		staticShader.setFloat("pointLight[0].quadratic", 0.00032f); //ayudan a que los  rayos de luz vijen una mayor ditancia. Es un cambio mas severo
																  //Entre mas pequeño el valor mas pequeña la atenuacion

		staticShader.setVec3("pointLight[1].position", glm::vec3(80.0, 0.0f, 0.0f));//posicion fija puntual
		staticShader.setVec3("pointLight[1].ambient", varColorAmbient); //varColorAmbient
		staticShader.setVec3("pointLight[1].diffuse", varColorDiffuse);
		staticShader.setVec3("pointLight[1].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[1].constant", 1.0f);
		staticShader.setFloat("pointLight[1].linear", 0.009f);
		staticShader.setFloat("pointLight[1].quadratic", 0.000032f);

		//Fuente de luz de reflector
		staticShader.setVec3("spotLight[0].position", glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z));
		staticShader.setVec3("spotLight[0].direction", glm::vec3(camera.Front.x, camera.Front.y, camera.Front.z));
		staticShader.setVec3("spotLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("spotLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("spotLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("spotLight[0].cutOff", glm::cos(glm::radians(10.0f)));//para modificar tamaño del circulo reflector
		staticShader.setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(10.0f)));//tamaño de dispersion de la luz (basandose en el cuttoff)
		staticShader.setFloat("spotLight[0].constant", 0.1f);
		staticShader.setFloat("spotLight[0].linear", 0.0009f);
		staticShader.setFloat("spotLight[0].quadratic", 0.0005f);

		staticShader.setFloat("material_shininess", 32.0f);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 tmp = glm::mat4(1.0f);
		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		//// Light
		glm::vec3 lightColor = glm::vec3(0.6f);
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.75f);
		

		// -------------------------------------------------------------------------------------------------------------------------
		// Animaciones en FBX
		// -------------------------------------------------------------------------------------------------------------------------
		//Remember to activate the shader with the animation
		animShader.use();
		animShader.setMat4("projection", projection);
		animShader.setMat4("view", view);
	
		animShader.setVec3("material.specular", glm::vec3(0.5f));
		animShader.setFloat("material.shininess", 32.0f);
		animShader.setVec3("light.ambient", lightColor); //	originalmente:	animShader.setVec3("light.ambient", ambientColor);
		animShader.setVec3("light.diffuse", diffuseColor);
		animShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		animShader.setVec3("light.direction", lightDirection);
		animShader.setVec3("viewPos", camera.Position);

		//Animación Pingüino
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-450.0f, -18.5f, -25.0f));
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.003f));
		animShader.setMat4("model", model);
		pinguinoVolador.Draw(animShader);

		//Animación Oso
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -20.75f, -515.0f));	//model = glm::translate(temporalPosicion, glm::vec3(0, 0, 0)); || model = glm::translate(glm::mat4(1.0f), glm::vec3(0, -20.0f, 0));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.0085f));
		animShader.setMat4("model", model);
		osoSaludo.Draw(animShader);

		//Amy Animation
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-590.0f, -20.75f, -603.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.30f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		amyDance.Draw(animShader);

		//Animación Policía
		model = glm::translate(glm::mat4(1.0f), glm::vec3(280.0, -20.75f, -275.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.30f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		policiaVigilando.Draw(animShader);

		//Animación León
		model = glm::translate(glm::mat4(1.0f), glm::vec3(485.0, -20.75f, -130.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(75.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.0097f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		leonRugido.Draw(animShader);

		// -------------------------------------------------------------------------------------------------------------------------
		// Escenario
		// -------------------------------------------------------------------------------------------------------------------------
		staticShader.use();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		//Piso
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -20.75f, 0.0f));
		model = glm::scale(model, glm::vec3(0.12f));
		staticShader.setMat4("model", model);
		piso.Draw(staticShader);

		//Reja
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -20.75f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		reja.Draw(staticShader);

		//Zona de afuera
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -20.75f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		zonaAfuera.Draw(staticShader);

		//Letrero
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -20.75f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		letrero.Draw(staticShader);

		//Serpiente
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -20.75f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		cobra.Draw(staticShader);


		//Pasillo 1 leones exteremo
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(450.0f, -21.0f, -320.0f));
		model = glm::scale(model, glm::vec3(1.9f));
		staticShader.setMat4("model", model);
		pasillo.Draw(staticShader);

		//Pasillo 2 entre comida y leones
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(450.0f, -20.8f, 320.0f));
		model = glm::scale(model, glm::vec3(1.9f));
		staticShader.setMat4("model", model);
		pasillo.Draw(staticShader);

		//Pasillo 3 entre juegos y pinguinos
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-545.0f, -20.9f, -376.0f));
		model = glm::scale(model, glm::vec3(1.9f));
		staticShader.setMat4("model", model);
		pasillo.Draw(staticShader);

		//Pasillo 4 entre juegos y osos
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-335.0f, -21.2f, -540.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.9f));
		staticShader.setMat4("model", model);
		pasillo.Draw(staticShader);

		//Pasillo 5 pinguino exteremo
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-542.0f, -21.3f, 325.0f));
		model = glm::scale(model, glm::vec3(1.9f));
		staticShader.setMat4("model", model);
		pasillo.Draw(staticShader);

		//Pasillo 6 entre osos y comida
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(335.0f, -21.2f, -540.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.9f));
		staticShader.setMat4("model", model);
		pasillo.Draw(staticShader);

		//Pasillo 7 extremo de comida
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(455.0f, -20.95f, -700.0f));
		model = glm::scale(model, glm::vec3(1.9f));
		staticShader.setMat4("model", model);
		pasillo.Draw(staticShader);



		//Habitat Leones
		model = glm::translate(glm::mat4(1.0f), glm::vec3(370.0f, -20.75f, -20.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 90.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.04f));//Haciendola pequena
		staticShader.setMat4("model", model);
		habitat_leones.Draw(staticShader);

		//Habitat Pinguinos
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-585.0f, -20.75f, 8.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 90.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.05f));//Haciendola pequena
		staticShader.setMat4("model", model);
		habitat_pinguinos.Draw(staticShader);

		//Zona juegos
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-590.0f, -20.75f, -603.0f));
		model = glm::scale(model, glm::vec3(0.80f));
		staticShader.setMat4("model", model);
		zona_juegos.Draw(staticShader);

		//Habitat Osos
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -20.75f, -580.0f));
		model = glm::scale(model, glm::vec3(20.0f));//Haciendola grande
		staticShader.setMat4("model", model);
		habitat_osos.Draw(staticShader);

		//Zona comida
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(498.0f, -20.75f, -520.0f));
		model = glm::scale(model, glm::vec3(0.90f));
		staticShader.setMat4("model", model);
		zona_comida.Draw(staticShader);

		//Carrito de helados 1
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-200.0f,15.0f, -80.0f));//Ubicacion 
		model = glm::scale(model, glm::vec3(0.3f));//Haciendola un poco mas grande
		staticShader.setMat4("model", model);
		helados.Draw(staticShader);

		//Carrito de helados 2
		model = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 15.0f, 70.0f));//Ubicacion 
		model = glm::scale(model, glm::vec3(0.3f));//Haciendola un poco mas grande
		staticShader.setMat4("model", model);
		helados.Draw(staticShader);

		//Fuente
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -20.75f, 20.0f));//Ubicacion 
		model = glm::scale(model, glm::vec3(0.04f));//Haciendola pquena
		staticShader.setMat4("model", model);
		fuente.Draw(staticShader);


		// -------------------------------------------------------------------------------------------------------------------------
		// Animacion Leon por Keyframes
		// -------------------------------------------------------------------------------------------------------------------------
		model = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, -20.75f, 0.0f));//Posicionar leon
		model = glm::scale(model, glm::vec3(0.4f));
		model = glm::translate(model, glm::vec3(posX, posY, posZ));
		tmp = model = glm::rotate(model, glm::radians(giroMonito), glm::vec3(0.0f, 1.0f, 0.0));
		staticShader.setMat4("model", model);
		torso.Draw(staticShader);

		////Pierna Der 2
		model= glm::translate(tmp, glm::vec3(11.7f,72.2f,-42.2f));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0));
		model = glm::rotate(model, glm::radians(-mov_patas_caminar), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		piernaDer.Draw(staticShader);

		////Pierna Izq 1
		model = glm::translate(tmp, glm::vec3(-9.8f,65.7f,-41.8f));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(mov_patas_caminar), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		piernaIzq.Draw(staticShader);

		////Brazo derecho 1
		model = glm::translate(tmp, glm::vec3(11.70f, 62.60f, 45.20f));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(mov_patas_caminar), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		brazoDer.Draw(staticShader);

		////Brazo izquierdo 2
		model = glm::translate(tmp, glm::vec3(-11.2f, 61.60f, 42.4f));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-mov_patas_caminar), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		brazoIzq.Draw(staticShader);

		////Cabeza
		model = glm::translate(tmp, glm::vec3(0.0f, 89.5f, 84.8f));
		model = glm::rotate(model, glm::radians(movCabeza), glm::vec3(0.0f, 0.0f, 1.0));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0));
		staticShader.setMat4("model", model);
		cabeza.Draw(staticShader);

		// -------------------------------------------------------------------------------------------------------------------------
		// Termina Escenario
		// -------------------------------------------------------------------------------------------------------------------------

		//-------------------------------------------------------------------------------------
		// draw skybox as last
		// -------------------
		skyboxShader.use();
		skybox.Draw(skyboxShader, view, projection, camera);

		// Limitar el framerate a 60
		deltaTime = SDL_GetTicks() - lastFrame; // time for full 1 loop
		//std::cout <<"frame time = " << frameTime << " milli sec"<< std::endl;
		if (deltaTime < LOOP_TIME)
		{
			SDL_Delay((int)(LOOP_TIME - deltaTime));
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	skybox.Terminate();

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void my_input(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, (float)deltaTime);
	//To Configure Model
	//______________________________________________________
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) //Mover hacia adelante
		posZ += 0.5f;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		posZ -= 0.5f;
	//______________________________________________________
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		posX--;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		posX++;
	//______________________________________________________
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)//Movimiento piernas 1
		mov_patas_caminar--;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)//Movimiento piernas 2
		mov_patas_caminar++;
	//______________________________________________________
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)//Giro en su propio eje
		giroMonito--;
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		giroMonito++;

	//Car animation
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		animacion ^= true;


	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		movCabeza += 10.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		movCabeza -= 10.0f;
	}

	//To play KeyFrame animation 
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if (play == false && (FrameIndex > 1))
		{
			std::cout << "Play animation" << std::endl;
			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
			std::cout << "Not enough Key Frames" << std::endl;
		}
	}

	//To Save a KeyFrame
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}
	}


	//To play KeyFrame animation 
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{	
		int count=0;
		string line;

		ifstream archivo_cuenta("animacion_leon.txt");
		while (archivo_cuenta.peek() != EOF)
		{
			getline(archivo_cuenta, line);
			count++;
		}
		
		std::cout << count << std::endl;

		ifstream archivo_leido("animacion_leon.txt");

		for (int contador_frame = 0; contador_frame < count; contador_frame++)
		{
			archivo_leido >> KeyFrame[contador_frame].posX >> KeyFrame[contador_frame].posY >> KeyFrame[contador_frame].posZ
				>> KeyFrame[contador_frame].mov_patas_caminar >> KeyFrame[contador_frame].giroMonito
				>> KeyFrame[contador_frame].movCabeza;

			std::cout << "Coordenadas leidas del archivo: " << KeyFrame[contador_frame].posX << KeyFrame[contador_frame].posY
				<< KeyFrame[contador_frame].posZ << KeyFrame[contador_frame].mov_patas_caminar
				<< KeyFrame[contador_frame].giroMonito << KeyFrame[contador_frame].movCabeza << std::endl;

		}

		FrameIndex = count;

		if (play == false)
		{
			std::cout << "Play animation" << std::endl;
			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
			std::cout << "Not enough Key Frames" << std::endl;
		}
	}


}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}