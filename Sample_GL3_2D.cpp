/*Author : Bakhtiyar Syed */



#define _GLIBCXX_USE_CXX11_ABI 0

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <fstream>
//#include <libstdc++>
#include <vector>
#include <map>
//#include <bits/stdc++.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

//Define ALT key
//#define _GLIBCXX_USE_CXX11_ABI 0

//#define GLFW_MOD_ALT 0x0004



using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

//Structs for drawing colors and base object

struct color{
	GLfloat r;
	GLfloat g;
	GLfloat b;
};

struct Base {
	string name;
	color rgb_color;
	GLfloat x,y;
	VAO* object;
	GLint status;
	GLfloat height,width;
	GLfloat x_speed,y_speed;
	GLfloat angle; //Current Angle (Actual rotated angle of the object)
	GLint inAir;
	GLfloat radius;
	GLint fixed;
	GLfloat friction; //Value from 0 to 1
	GLint health;
	GLint isRotating;
	GLint direction; //0 for clockwise and 1 for anticlockwise for animation
	GLfloat remAngle; //the remaining angle to finish animation
	GLint isMovingAnim;
	GLint dx;
	GLint dy;
	int col_type;
	GLfloat weight;
};


//Grouping the objects for different visualisation , eg. cannon, bricks, buckets

std::map <string, Base> cannonobj; //Only have the cannon objects here
std::map <string, Base> bucketobj; //Only for bucket objects
std::map <string, Base> brickobj; //Only for brick objects
std::map <string, Base> backgroundobj; //Only for the background objects
std::map <string, Base> laserobj; //Only for laser objects
std::map <string, Base> mirrorobj; //Only for mirror objects
std::map <string, Base> scoreboardobj; //Only for scoreboard objects

std::map<string, Base> randomobj; //For random objects that float around or are not defined above

//Keep track of laser count and other globals
GLint globalscore=0;

int laser_count=1;
GLfloat zoom_camera = 1;
float x_change = 0; //For the camera pan
float y_change = 0; //For the camera pan
GLdouble prev_laser_time;
GLdouble new_laser_time;

GLint game_status = 1;



//Create function prototype, otherwise we'll have to place createCircle above
void createCircle (string name, GLfloat weight, color rgb_colorin, float x, float y, float r, int NoOfParts, string component, int fill);
void checkCollision(string key, string check);
void checkLaserCollision(string a);
void createRectangle (string name, GLfloat weight, color A, color B, color C, color D, GLfloat x, GLfloat y, GLfloat height, GLfloat width, string component );
void mousescroll(GLFWwindow* window, double xoffset, double yoffset);
void check_pan();




/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{

	glfwDestroyWindow(window);
	glfwTerminate();
  cout << "Your final score is " << globalscore << endl;
	//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/


//Check for Collision of Laser with the Falling Bricks only
void checkLaserCollision(string key)
{
	//struct Base laserobj[key] = laserobj[key];
	for( std::map<string,Base>::iterator it=brickobj.begin() ; it!=brickobj.end() ;  it++)
	{

		string currentobj = it->first;


		if(brickobj[currentobj].status==0)
			continue;


		float x = fabs(laserobj[key].x - brickobj[currentobj].x);
		float y = fabs(laserobj[key].y - brickobj[currentobj].y);

		/*  if (laserobj[key]Distance.x > (brickobj[currentobj].width/2 + laserobj[key].r)) { return false; }
		    if (laserobj[key]Distance.y > (brickobj[currentobj].height/2 + laserobj[key].r)) { return false; }*/

		if ((x - (brickobj[currentobj].width/2) <= 0.000001) && laserobj[key].status==1 &&brickobj[currentobj].status==1) { laserobj[key].status=0; brickobj[currentobj].status=0;}
		if ((y -(brickobj[currentobj].height/2) <= 0.000001) && laserobj[key].status==1 &&brickobj[currentobj].status==1) { laserobj[key].status=0; brickobj[currentobj].status=0; }

		float cornerDistance_sq = (x - brickobj[currentobj].width/2)*(x - brickobj[currentobj].width/2) +
			(y - brickobj[currentobj].height/2)*(y - brickobj[currentobj].height/2);

		if ((cornerDistance_sq - (laserobj[key].radius*laserobj[key].radius) <= 0.0000001)  && laserobj[key].status==1 &&brickobj[currentobj].status==1)
		{
			laserobj[key].status=0; brickobj[currentobj].status=0;
		}

	}


}






float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_X:
				// do something ..
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {

		//Bucket movement captures from the keyboard. glfwGetKey is the main function here
		if (glfwGetKey(window,GLFW_KEY_LEFT) && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) && (bucketobj["bucket1"].x- 0.5*bucketobj["bucket1"].width )>=-3.9)
			bucketobj["bucket1"].x-=0.1;
		if (glfwGetKey(window, GLFW_KEY_RIGHT) && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) && (bucketobj["bucket1"].x + 0.5*bucketobj["bucket1"].width)<= 3.9)
			bucketobj["bucket1"].x+=0.1;
		if (glfwGetKey(window,GLFW_KEY_LEFT ) && glfwGetKey(window, GLFW_KEY_LEFT_ALT)&& ( bucketobj["bucket2"].x- 0.5*bucketobj["bucket2"].width) >=-3.9)
			bucketobj["bucket2"].x-=0.1;
		if (glfwGetKey(window,GLFW_KEY_RIGHT ) && glfwGetKey(window, GLFW_KEY_LEFT_ALT)&& (bucketobj["bucket2"].x+ 0.5*bucketobj["bucket2"].width) <= 3.9)
			bucketobj["bucket2"].x+=0.1;


		//Translate the cannon front and rear objects up and down on keypress of S and F
		if(glfwGetKey(window,GLFW_KEY_S))
		{
			if(cannonobj["rear"].y <= 2.7 && cannonobj["rear"].y >=-3.3)
			{
				cannonobj["rear"].y+=0.2;
				cannonobj["front"].y+=0.2;
			}
		}

		if(glfwGetKey(window,GLFW_KEY_F))
		{
			if(cannonobj["rear"].y <= 3.2 && cannonobj["rear"].y >=-3.5)
			{
				cannonobj["rear"].y-=0.2;
				cannonobj["front"].y-=0.2;
			}
		}




		//Rotate the cannon front and rear objects on keypress of A and D
		if(glfwGetKey(window, GLFW_KEY_A))
		{
			if(cannonobj["front"].angle < 45)
			{
				cannonobj["front"].angle+=8;
			}

		}
		if(glfwGetKey(window, GLFW_KEY_D))
		{
			if(cannonobj["front"].angle > -45)
			{
				cannonobj["front"].angle-=8;
			}

		}

		//Shoot lasers on press of Spacebar
		if(glfwGetKey(window, GLFW_KEY_SPACE))
		{
			string n = "laser";
			n.append(std::to_string(laser_count++)); //Converting to string and incrementing the laser count by 1

      new_laser_time=glfwGetTime();

      if(n!="laser1" && new_laser_time - prev_laser_time > 1)
			   {
          createRectangle(n, 10000, {1.000, 0.941, 0.961},{1.000, 0.941, 0.961},{1.000, 0.941, 0.961},{1.000, 0.941, 0.961},cannonobj["front"].x+0.3, cannonobj["front"].y, 0.08, 0.2, "laserobj" );
         prev_laser_time=glfwGetTime();
       }

  	 }


		//Zoom in, zoom out
		if(glfwGetKey(window, GLFW_KEY_UP))
		{
			mousescroll(window,0,+1);
			check_pan();
		}
		if(glfwGetKey(window, GLFW_KEY_DOWN))
		{
			mousescroll(window,0,-1);
			check_pan();
		}

		//Pan the screen
		if(glfwGetKey(window, GLFW_KEY_RIGHT))
		{
			x_change+=10;
			check_pan();
		}
		if(glfwGetKey(window, GLFW_KEY_LEFT))
		{
			x_change-=10;
			check_pan();
		}


  /*  //Increase the speed of falling blocks
    if(glfwGetKey(window, GLFW_KEY_N))
    {
      for( map<string,Base>::iterator it=brickobj.begin() ; it!=brickobj.end() ;  )
    	{
    		string currentobj = it->first;
    		if(brickobj[currentobj].status==0)
    		{
    			it++;
    			continue;
    		}

        if(brickobj[currentobj].y_speed <=0.032)
          brickobj[currentobj].y_speed+=0.002;

      }
    }


    //Decrease the speed of falling blocks
    if(glfwGetKey(window, GLFW_KEY_M))
    {
      for( map<string,Base>::iterator it=brickobj.begin() ; it!=brickobj.end() ;  )
    	{
    		string currentobj = it->first;
    		if(brickobj[currentobj].status==0)
    		{
    			it++;
    			continue;
    		}

        if(brickobj[currentobj].y_speed >=0.008)
          brickobj[currentobj].y_speed-=0.002;

      }
    }*/


	}
}



void mousescroll(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset==-1) {
		zoom_camera /= 1.1; //make it bigger than current size
	}
	else if(yoffset==1){
		zoom_camera *= 1.1; //make it bigger than current size
	}
	if (zoom_camera<=1) {
		zoom_camera = 1;
	}
	if (zoom_camera>=4) {
		zoom_camera=4;
	}
	if(x_change-4.0f/zoom_camera<-4)
		x_change=-4+4.0f/zoom_camera;
	else if(x_change+4.0f/zoom_camera>4)
		x_change=4-4.0f/zoom_camera;
	if(y_change-4.0f/zoom_camera<-4)
		y_change=-4+4.0f/zoom_camera;
	else if(y_change+4.0f/zoom_camera>4)
		y_change=4-4.0f/zoom_camera;
	Matrices.projection = glm::ortho((float)(-4.0f/zoom_camera+x_change), (float)(4.0f/zoom_camera+x_change), (float)(-4.0f/zoom_camera+y_change), (float)(4.0f/zoom_camera+y_change), 0.1f, 500.0f);
}



//Ensure the panning does not go out of the map
void check_pan(){
	if(x_change-4.0f/zoom_camera<-4)
		x_change=-4+4.0f/zoom_camera;
	else if(x_change+4.0f/zoom_camera>4)
		x_change=4-4.0f/zoom_camera;
	if(y_change-4.0f/zoom_camera<-4)
		y_change=-4+4.0f/zoom_camera;
	else if(y_change+4.0f/zoom_camera>4)
		y_change=4-4.0f/zoom_camera;
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

GLdouble rightmouse_click=0;
GLdouble rightmouse_x;
GLdouble rightmouse_y;
GLdouble newrightmouse_x;
GLdouble newrightmouse_y;

GLdouble leftmouse_click=0;
GLdouble leftmouse_x;
GLdouble leftmouse_y;
GLdouble newleftmouse_x;
GLdouble newleftmouse_y;


/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE){
				//triangle_rot_dir *= -1;
				string n = "laser";
				n.append(std::to_string(laser_count++)); //Converting to string and incrementing the laser count by 1
        //Time gap between successive laser shots
        new_laser_time=glfwGetTime();
        if(n!="laser1" && new_laser_time - prev_laser_time > 1)
           {
            createRectangle(n, 10000, {1.000, 0.941, 0.961},{1.000, 0.941, 0.961},{1.000, 0.941, 0.961},{1.000, 0.941, 0.961},cannonobj["front"].x+0.3, cannonobj["front"].y, 0.08, 0.2, "laserobj" );
           prev_laser_time=glfwGetTime();
         }
				//createRectangle(n, 10000, {1.000, 0.941, 0.961},{1.000, 0.941, 0.961},{1.000, 0.941, 0.961},{1.000, 0.941, 0.961},cannonobj["front"].x+0.3, cannonobj["front"].y, 0.08, 0.2, "laserobj" );
				leftmouse_click=0;
			}
			if (action == GLFW_PRESS)
			{
				leftmouse_click=1;
				//Tricky, but nice, since angle of the laser will be dependent on angle of the cannon, so no need to update the cannon automatically.
				cannonobj["front"].angle = atan((newleftmouse_y - cannonobj["front"].y)/(newleftmouse_x - cannonobj["front"].x)) *180.0f/ M_PI;
			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
				rightmouse_click=0;
			}
			if (action == GLFW_PRESS)
			{
				rightmouse_click=1;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle;

// Creates the triangle object used in this sample code


/*void createTriangle ()
  {
/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

/* Define vertex array as used in glBegin (GL_TRIANGLES)
   static GLfloat vertex_buffer_data [] = {
   0, 1,0, // vertex 0
   -1,-1,0, // vertex 1
   1,-1,0, // vertex 2
   };

   static GLfloat color_buffer_data [] = {
   1,0,0, // color 0
   0,1,0, // color 1
   0,0,1, // color 2
   };

// create3DObject creates and returns a handle to a VAO that can be used later
triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);

}

// Creates the triangle object used in this sample code
void createTriangle (string name, GLfloat weight, color rgb, GLfloat x[], GLfloat y[], string component, GLint fill)
{
/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen

GLfloat xc=(x[0]+x[1]+x[2])/3;
GLfloat yc=(y[0]+y[1]+y[2])/3;


/* Define vertex array as used in glBegin (GL_TRIANGLES)
static GLfloat vertex_buffer_data [] = {
x[0]-xc,y[0]-yc,0, // vertex 0
x[1]-xc,y[1]-yc,0, // vertex 1
x[2]-xc,y[2]-yc,0 // vertex 2
};

static GLfloat color_buffer_data [] = {
rgb.r,rgb.g,rgb.b, // rgb 1
rgb.r,rgb.g,rgb.b, // rgb 2
rgb.r,rgb.g,rgb.b // rgb 3
};

// create3DObject creates and returns a handle to a VAO that can be used later
triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);

Base tempobj = {};
tempobj.rgb_color = rgb;
tempobj.name = name;
tempobj.object = triangle;
tempobj.x=(x[0]+x[1]+x[2])/3; //Position of the Base is the position of the centroid
tempobj.y=(y[0]+y[1]+y[2])/3;
tempobj.height=-1; //Height of the Base obj is undefined
tempobj.width=-1; //Width of the Base obj is undefined
tempobj.status=1;
tempobj.inAir=0;
tempobj.x_speed=0;
tempobj.y_speed=0;
tempobj.radius=-1; //The bounding circle radius is not defined.
tempobj.fixed=0;
tempobj.friction=0.4;
tempobj.health=100;
tempobj.weight=weight;


}*/


// Creates the rectangle object used in this sample code
void createRectangle (string name, GLfloat weight, color A, color B, color C, color D, GLfloat x, GLfloat y, GLfloat height, GLfloat width, string component )
{


	GLfloat w = 0.5*width;
	GLfloat h = 0.5*height;
	// GL3 accepts only Triangles. Quads are not supported
	GLfloat vertex_buffer_data [] = {
		-w,-h,0, // vertex 1
		-w, h,0, // vertex 2
		w, h, 0,// vertex 3

		w,h,0, // vertex 3
		w, -h, 0, // vertex 4
		-w, -h, 0  // vertex 1
	};

	GLfloat color_buffer_data [] = {
		A.r, A.g, A.b, // color 1
		B.r, B.g, B.b, // color 2
		C.r, C.g, C.b, // color 3

		C.r, C.g, C.b, // color 3
		D.r, D.g, D.b, // color 4
		A.r, A.g, A.b // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

	//Create the Base temporary object here corresponding to the name and this gets sorted

	Base tempobj = {};
	tempobj.rgb_color = A;
	tempobj.name = name;
	tempobj.object = rectangle;
	tempobj.x=x;
	tempobj.y=y;
	tempobj.height=height;
	tempobj.width=width;
	tempobj.status=1;
	tempobj.inAir=0;
	tempobj.angle=0;
	tempobj.x_speed=0;
	tempobj.y_speed=0;
	tempobj.fixed=0;
	tempobj.radius=(sqrt(height*height+width*width))/2;
	tempobj.friction=0.4;
	tempobj.health=100;
	tempobj.weight=weight;

	if(component=="cannonobj")
		cannonobj[name]=tempobj;
	else if(component=="brickobj")
		brickobj[name]=tempobj;
	else if(component=="bucketobj")
		bucketobj[name]=tempobj;
	else if(component=="backgroundobj")
		backgroundobj[name]=tempobj;
	else if(component=="laserobj")
	{
		laserobj[name]=tempobj;
		laserobj[name].angle=cannonobj["front"].angle;
	}
  else if(component=="scoreboardobj")
  {
    scoreboardobj[name]=tempobj;
    scoreboardobj[name].status=0;
  }

	else
		randomobj[name]=tempobj;


}

//Creates the circles used in the code
void createCircle (string name, GLfloat weight, color rgb_colorin, float x, float y, float r, int NoOfParts, string component, int fill)
{
	int parts;
	parts = NoOfParts;
	float radius;
	radius = r;
	GLfloat vertex_buffer_data[parts*9];
	GLfloat color_buffer_data[parts*9];
	int i,j;
	float angle=(2*M_PI/parts);
	float current_angle = 0;
	for(i=0;i<parts;i++){
		for(j=0;j<3;j++){
			color_buffer_data[i*9+j*3]=rgb_colorin.r;
			color_buffer_data[i*9+j*3+1]=rgb_colorin.g;
			color_buffer_data[i*9+j*3+2]=rgb_colorin.b;
		}
		vertex_buffer_data[i*9]=0;
		vertex_buffer_data[i*9+1]=0;
		vertex_buffer_data[i*9+2]=0;
		vertex_buffer_data[i*9+3]=radius*cos(current_angle);
		vertex_buffer_data[i*9+4]=radius*sin(current_angle);
		vertex_buffer_data[i*9+5]=0;
		vertex_buffer_data[i*9+6]=radius*cos(current_angle+angle);
		vertex_buffer_data[i*9+7]=radius*sin(current_angle+angle);
		vertex_buffer_data[i*9+8]=0;
		current_angle+=angle;
	}
	VAO* circle;
	if(fill==1)
		circle = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_FILL);
	else
		circle = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_LINE);
	Base tempobj = {};
	tempobj.rgb_color = rgb_colorin ;
	tempobj.name = name;
	tempobj.object = circle;
	tempobj.x=x;
	tempobj.y=y;
	tempobj.height=2*r; //Height of the sprite is 2*r
	tempobj.width=2*r; //Width of the sprite is 2*r
	tempobj.status=1;
	tempobj.inAir=0;
	tempobj.x_speed=0;
	tempobj.y_speed=0;
	tempobj.radius=r;
	tempobj.fixed=0;
	tempobj.friction=0.4;
	tempobj.health=100;
	tempobj.weight=weight;
	if(component=="laserobj")
	{
		laserobj[name]=tempobj;
		laserobj[name].angle=cannonobj["front"].angle;
	}
	if(component=="backgroundobj")
	{
		backgroundobj[name]=tempobj;
	}
	else
		randomobj[name]=tempobj;
}


//Creates the brick object used in this code
void createBrick (string name, GLfloat weight, color A, color B, color C, color D, GLfloat x, GLfloat y, GLfloat height, GLfloat width, string component, int color_type )
{


	GLfloat w = 0.5*width;
	GLfloat h = 0.5*height;
	// GL3 accepts only Triangles. Quads are not supported
	GLfloat vertex_buffer_data [] = {
		-w,-h,0, // vertex 1
		-w, h,0, // vertex 2
		w, h, 0,// vertex 3

		w,h,0, // vertex 3
		w, -h, 0, // vertex 4
		-w, -h, 0  // vertex 1
	};

	GLfloat color_buffer_data [] = {
		A.r, A.g, A.b, // color 1
		B.r, B.g, B.b, // color 2
		C.r, C.g, C.b, // color 3

		C.r, C.g, C.b, // color 3
		D.r, D.g, D.b, // color 4
		A.r, A.g, A.b // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

	//Create the Base temporary object here corresponding to the name and this gets sorted

	Base tempobj = {};
	tempobj.rgb_color = A;
	tempobj.name = name;
	tempobj.object = rectangle;
	tempobj.x=x;
	tempobj.y=y;
	tempobj.height=height;
	tempobj.width=width;
	tempobj.status=1;
	tempobj.col_type=color_type;
	tempobj.inAir=0;
	tempobj.x_speed=0;
	tempobj.y_speed=0.018;
	tempobj.fixed=0;
	tempobj.radius=(sqrt(height*height+width*width))/2;
	tempobj.friction=0.4;
	tempobj.health=100;
	tempobj.weight=weight;

	brickobj[name]=tempobj;

}
//Creates the mirror  object used in this code
void createMirror (string name, GLfloat weight, color A, color B, color C, color D, GLfloat x, GLfloat y, GLfloat height, GLfloat width, string component, int angle)
{


	GLfloat w = 0.5*width;
	GLfloat h = 0.5*height;
	// GL3 accepts only Triangles. Quads are not supported
	GLfloat vertex_buffer_data [] = {
		-w,-h,0, // vertex 1
		-w, h,0, // vertex 2
		w, h, 0,// vertex 3

		w,h,0, // vertex 3
		w, -h, 0, // vertex 4
		-w, -h, 0  // vertex 1
	};

	GLfloat color_buffer_data [] = {
		A.r, A.g, A.b, // color 1
		B.r, B.g, B.b, // color 2
		C.r, C.g, C.b, // color 3

		C.r, C.g, C.b, // color 3
		D.r, D.g, D.b, // color 4
		A.r, A.g, A.b // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

	//Create the Base temporary object here corresponding to the name and this gets sorted

	Base tempobj = {};
	tempobj.rgb_color = A;
	tempobj.name = name;
	tempobj.object = rectangle;
	tempobj.x=x;
	tempobj.y=y;
	tempobj.height=height;
	tempobj.width=width;
	tempobj.status=1;
	//tempobj.col_type=color_type;
	tempobj.inAir=0;
	tempobj.x_speed=0;
	tempobj.y_speed=0;
	tempobj.fixed=0;
	tempobj.radius=(sqrt(height*height+width*width))/2;
	tempobj.friction=0.4;
	tempobj.health=100;
	tempobj.weight=weight;

	mirrorobj[name]=tempobj;
	/*std::random_device rd; // obtain a random number from hardware
	  std::mt19937 eng(rd()); // seed the generator
	  std::uniform_int_distribution<> distr(90, 180); // define the range*/

	mirrorobj[name].angle= angle;





}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;


//Segment clearer
void segmentclear()
{
  scoreboardobj["one"].status =0;
  scoreboardobj["two"].status =0;
  scoreboardobj["three"].status =0;
  scoreboardobj["four"].status =0;
  scoreboardobj["five"].status =0;
  scoreboardobj["six"].status =0;
  scoreboardobj["seven"].status =0;
  scoreboardobj["eight"].status =0;
  scoreboardobj["nine"].status =0;
  scoreboardobj["ten"].status =0;
  scoreboardobj["eleven"].status =0;
  scoreboardobj["twelve"].status =0;
  scoreboardobj["thirteen"].status =0;
  scoreboardobj["fourteen"].status =0;
}

//Segment setter

void segmentsetter()
{

  if (globalscore < 0)
  {
    scoreboardobj["seven"].status=1;
    GLint newglobalscore = abs(globalscore);
    GLint newright = newglobalscore%10;
    if (newright==1)
    {
      scoreboardobj["ten"].status =1;
      scoreboardobj["eleven"].status =1;

    }

    else if (newright==2)
    {
      scoreboardobj["nine"].status =1;
      scoreboardobj["ten"].status =1;
      scoreboardobj["twelve"].status =1;
      scoreboardobj["thirteen"].status =1;
      scoreboardobj["fourteen"].status =1;



    }
    else if(newright==3)
    {
      scoreboardobj["nine"].status =1;
      scoreboardobj["ten"].status =1;
      scoreboardobj["eleven"].status =1;
      scoreboardobj["twelve"].status =1;
      scoreboardobj["fourteen"].status =1;
    }
     else if (newright == 4)
    {
      scoreboardobj["eight"].status =1;
      scoreboardobj["ten"].status =1;
      scoreboardobj["eleven"].status =1;
      scoreboardobj["fourteen"].status =1;
    }
    else if (newright==5)
    {
      scoreboardobj["eight"].status =1;
      scoreboardobj["nine"].status =1;
      scoreboardobj["fourteen"].status =1;
      scoreboardobj["eleven"].status =1;
      scoreboardobj["twelve"].status =1;
    }
    else if (newright==6)
    {
      scoreboardobj["eight"].status =1;
      scoreboardobj["nine"].status =1;
      scoreboardobj["fourteen"].status =1;
      scoreboardobj["eleven"].status =1;
      scoreboardobj["twelve"].status =1;
      scoreboardobj["thirteen"].status =1;
    }
    else if(newright==7)
    {
      scoreboardobj["nine"].status =1;
      scoreboardobj["ten"].status =1;
      scoreboardobj["eleven"].status =1;
    }
    else if(newright==8)
    {
      scoreboardobj["eight"].status =1;
      scoreboardobj["nine"].status =1;
      scoreboardobj["ten"].status =1;
      scoreboardobj["eleven"].status =1;
      scoreboardobj["twelve"].status =1;
      scoreboardobj["thirteen"].status =1;
      scoreboardobj["fourteen"].status =1;
    }
    else if(newright==9)
    {
      scoreboardobj["eight"].status =1;
      scoreboardobj["nine"].status =1;
      scoreboardobj["ten"].status =1;
      scoreboardobj["eleven"].status =1;
      scoreboardobj["twelve"].status =1;
      scoreboardobj["fourteen"].status =1;
    }
    return;
  }

  GLint left = globalscore/10;
  GLint right = globalscore%10;

  //cout << "left is " << left << " and right is " <<right << endl;

  //Set the status to 1 for the left digit
  if(left==0)
  {
    scoreboardobj["one"].status =1;
    scoreboardobj["two"].status =1;
    scoreboardobj["three"].status =1;
    scoreboardobj["four"].status =1;
    scoreboardobj["five"].status =1;
    scoreboardobj["six"].status =1;
    scoreboardobj["seven"].status =0;

  }
  else if (left==1)
  {
    scoreboardobj["three"].status =1;
    scoreboardobj["four"].status =1;

  }

  else if (left==2)
  {
    scoreboardobj["two"].status =1;
    scoreboardobj["three"].status =1;
    scoreboardobj["five"].status =1;
    scoreboardobj["six"].status =1;
    scoreboardobj["seven"].status =1;



  }
  else if(left==3)
  {
    scoreboardobj["two"].status =1;
    scoreboardobj["three"].status =1;
    scoreboardobj["four"].status =1;
    scoreboardobj["five"].status =1;
    scoreboardobj["seven"].status =1;
  }
   else if (left == 4)
  {
    scoreboardobj["one"].status =1;
    scoreboardobj["three"].status =1;
    scoreboardobj["four"].status =1;
    scoreboardobj["seven"].status =1;
  }
  else if (left==5)
  {
    scoreboardobj["one"].status =1;
    scoreboardobj["two"].status =1;
    scoreboardobj["seven"].status =1;
    scoreboardobj["four"].status =1;
    scoreboardobj["five"].status =1;
  }
  else if (left==6)
  {
    scoreboardobj["one"].status =1;
    scoreboardobj["two"].status =1;
    scoreboardobj["seven"].status =1;
    scoreboardobj["four"].status =1;
    scoreboardobj["five"].status =1;
    scoreboardobj["six"].status =1;
  }
  else if(left==7)
  {
    scoreboardobj["two"].status =1;
    scoreboardobj["three"].status =1;
    scoreboardobj["four"].status =1;
  }
  else if(left==8)
  {
    scoreboardobj["one"].status =1;
    scoreboardobj["two"].status =1;
    scoreboardobj["three"].status =1;
    scoreboardobj["four"].status =1;
    scoreboardobj["five"].status =1;
    scoreboardobj["six"].status =1;
    scoreboardobj["seven"].status =1;
  }
  else if (left==9)
  {
    scoreboardobj["one"].status =1;
    scoreboardobj["two"].status =1;
    scoreboardobj["three"].status =1;
    scoreboardobj["four"].status =1;
    scoreboardobj["five"].status =1;
    scoreboardobj["seven"].status =1;
  }

  //Set the status to 1 for the right digit

  if(right==0)
  {
    scoreboardobj["eight"].status =1;
    scoreboardobj["nine"].status =1;
    scoreboardobj["ten"].status =1;
    scoreboardobj["eleven"].status =1;
    scoreboardobj["twelve"].status =1;
    scoreboardobj["thirteen"].status =1;
  }
  else if (right==1)
  {
    scoreboardobj["ten"].status =1;
    scoreboardobj["eleven"].status =1;

  }

  else if (right==2)
  {
    scoreboardobj["nine"].status =1;
    scoreboardobj["ten"].status =1;
    scoreboardobj["twelve"].status =1;
    scoreboardobj["thirteen"].status =1;
    scoreboardobj["fourteen"].status =1;



  }
  else if(right==3)
  {
    scoreboardobj["nine"].status =1;
    scoreboardobj["ten"].status =1;
    scoreboardobj["eleven"].status =1;
    scoreboardobj["twelve"].status =1;
    scoreboardobj["fourteen"].status =1;
  }
   else if (right == 4)
  {
    scoreboardobj["eight"].status =1;
    scoreboardobj["ten"].status =1;
    scoreboardobj["eleven"].status =1;
    scoreboardobj["fourteen"].status =1;
  }
  else if (right==5)
  {
    scoreboardobj["eight"].status =1;
    scoreboardobj["nine"].status =1;
    scoreboardobj["fourteen"].status =1;
    scoreboardobj["eleven"].status =1;
    scoreboardobj["twelve"].status =1;
  }
  else if (right==6)
  {
    scoreboardobj["eight"].status =1;
    scoreboardobj["nine"].status =1;
    scoreboardobj["fourteen"].status =1;
    scoreboardobj["eleven"].status =1;
    scoreboardobj["twelve"].status =1;
    scoreboardobj["thirteen"].status =1;
  }
  else if(right==7)
  {
    scoreboardobj["nine"].status =1;
    scoreboardobj["ten"].status =1;
    scoreboardobj["eleven"].status =1;
  }
  else if(right==8)
  {
    scoreboardobj["eight"].status =1;
    scoreboardobj["nine"].status =1;
    scoreboardobj["ten"].status =1;
    scoreboardobj["eleven"].status =1;
    scoreboardobj["twelve"].status =1;
    scoreboardobj["thirteen"].status =1;
    scoreboardobj["fourteen"].status =1;
  }
  else if(right==9)
  {
    scoreboardobj["eight"].status =1;
    scoreboardobj["nine"].status =1;
    scoreboardobj["ten"].status =1;
    scoreboardobj["eleven"].status =1;
    scoreboardobj["twelve"].status =1;
    scoreboardobj["fourteen"].status =1;
  }
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window)
{

	//Matrices.projection = glm::ortho((float)(-400.0f), (float)(400.0f), (float)(-300.0f), (float)(300.0f), 0.1f, 500.0f);

	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);

	/* Render your scene */

	glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	//glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangleTransform =  /*rotateTriangle **/ translateTriangle ;
	Matrices.model *= triangleTransform;
	MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	//draw3DObject(triangle);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();

	/*
	   Matrices.model = glm::mat4(1.0f);

	   glm::mat4 translateRectangle = glm::translate (glm::vec3(0.8, 0, 0));        // glTranslatef
	   glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	   Matrices.model *= (translateRectangle/* * rotateRectangle);
	   MVP = VP * Matrices.model;
	   glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);*/


	// draw3DObject draws the VAO given to it using current MVP matrix


	//Mouse cursor position grab for both left and right clicks
	glfwGetCursorPos(window, &rightmouse_x, &rightmouse_y);
	glfwGetCursorPos(window, &leftmouse_x, &leftmouse_y);
	newrightmouse_x=(rightmouse_x - 300);
	newrightmouse_x*=4/300.0;
	newrightmouse_y=(rightmouse_y - 300);
	newrightmouse_y*=4/300.0;
	newrightmouse_y=-newrightmouse_y;

  //cout << newrightmouse_x << " , " <<newrightmouse_y << endl;


	newleftmouse_x=(leftmouse_x - 300);
	newleftmouse_x*=4/300.0;
	newleftmouse_y=(leftmouse_y - 300);
	newleftmouse_y*=4/300.0;
	newleftmouse_y=-newleftmouse_y;



	Matrices.projection = glm::ortho((float)(-4.0f/zoom_camera+x_change), (float)(4.0f/zoom_camera+x_change), (float)(-4.0f/zoom_camera+y_change), (float)(4.0f/zoom_camera+y_change), 0.1f, 500.0f);



  //Clear status variable of every scoreboard Objects
  segmentclear();



  //Segment Display for respective numbers

  segmentsetter();







	//For Background Objects
	for(map<string,Base>::iterator it=backgroundobj.begin();it!=backgroundobj.end();it++)
	{
		string currentobj = it->first;
		//cout << currentobj << endl;
		glm::mat4 MVP;
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateRectangle = glm::translate (glm::vec3(backgroundobj[currentobj].x, backgroundobj[currentobj].y, 0));        // glTranslatef
		glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle/* * rotateRectangle*/);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(backgroundobj[currentobj].object);
	}



	//For Bucket Objects
	for(map<string,Base>::iterator it=bucketobj.begin();it!=bucketobj.end();it++)
	{
		string currentobj = it->first;
		//cout << currentobj << endl;
		glm::mat4 MVP;
		Matrices.model = glm::mat4(1.0f);
		//bucketobj[currentobj].x+=0.01;
		glm::mat4 translateRectangle = glm::translate (glm::vec3(bucketobj[currentobj].x, bucketobj[currentobj].y, 0));        // glTranslatef
		glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle/* * rotateRectangle*/);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(bucketobj[currentobj].object);
		//checkCollision(currentobj);
	}

	//Bucket drag
	//Smooth drag added
	if ((rightmouse_click==1) && (newrightmouse_x>=-3.6 )&&(newrightmouse_x<=3.6)&&(newrightmouse_x<=(bucketobj["bucket1"].x+(0.75*bucketobj["bucket1"].width))) && (newrightmouse_x >= (bucketobj["bucket1"].x-(0.75*bucketobj["bucket1"].width))))
		bucketobj["bucket1"].x=newrightmouse_x;
	if ((rightmouse_click==1) &&  (newrightmouse_x>=-3.6 )&&(newrightmouse_x<=3.6)&& (newrightmouse_x<=(bucketobj["bucket2"].x+(0.75*bucketobj["bucket2"].width))) && (newrightmouse_x >= (bucketobj["bucket2"].x-(0.75*bucketobj["bucket2"].width))))
		bucketobj["bucket2"].x=newrightmouse_x;


	//For Cannon Objects
	for( map<string,Base>::iterator it=cannonobj.begin() ; it!=cannonobj.end() ;  it++)
	{   string currentobj = it->first;

		//cout << currentobj << endl;
		glm::mat4 MVP;
		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateRectangle = glm::translate (glm::vec3(cannonobj[currentobj].x, cannonobj[currentobj].y, 0));        // glTranslatef
		glm::mat4 rotateRectangle = glm::rotate((float)(cannonobj[currentobj].angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= ( translateRectangle * rotateRectangle);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(cannonobj[currentobj].object);
	}

	//Cannon drag via mouse
	if ((rightmouse_click==1) && (newrightmouse_y <= 2.7 ) &&(newrightmouse_y>=-3.5)&&(newrightmouse_x<=(cannonobj["front"].x+(0.5*cannonobj["front"].width))) && (newrightmouse_x >= (cannonobj["rear"].x-(0.5*bucketobj["bucket1"].width)))
			&& (newrightmouse_y<=cannonobj["rear"].y+(0.8*cannonobj["rear"].height)) && (newrightmouse_y>=cannonobj["rear"].y-(0.8*cannonobj["rear"].height)))
	{
		cannonobj["rear"].y=newrightmouse_y;
		cannonobj["front"].y=newleftmouse_y;
	}
	//For mouse-click of cannon front part, no need to place this inside the for loop of Cannon or Laser.
	if(leftmouse_click==1)
		cannonobj["front"].angle = atan((newleftmouse_y - cannonobj["front"].y)/(newleftmouse_x - cannonobj["front"].x)) *180.0f/ M_PI;


	GLint random_y=55;
	//For Brick Objects
	for( map<string,Base>::iterator it=brickobj.begin() ; it!=brickobj.end() ;  )
	{
		string currentobj = it->first;
		if(brickobj[currentobj].status==0)
		{
			it++;
			continue;
		}     //cout << "Status of current object is : " <<brickobj[currentobj].name << endl;

    //N and M for increasing and decreasing speeds within permissible limits
    if(glfwGetKey(window, GLFW_KEY_N))
    {
      if(brickobj[currentobj].y_speed <=0.032)
          brickobj[currentobj].y_speed+=0.002;
    }
    if(glfwGetKey(window, GLFW_KEY_M))
    {
      if(brickobj[currentobj].y_speed >=0.008)
          brickobj[currentobj].y_speed-=0.002;
    }

		glm::mat4 MVP;
		Matrices.model = glm::mat4(1.0f);
		//IF BRICK HAS FALLEN OUT OF THE SCREEN, REINVENT IT, SPEED supported
		if(brickobj[currentobj].status==1 && brickobj[currentobj].y<=-4 )
		{
			brickobj[currentobj].y=random_y;
			random_y--;
			if(random_y <= 4)
				random_y=55;
		}



		//Change falling speed of the bricks according to the y_speed parameter.
		brickobj[currentobj].y-=brickobj[currentobj].y_speed;

		glm::mat4 translateRectangle = glm::translate (glm::vec3(brickobj[currentobj].x, brickobj[currentobj].y, 0));        // glTranslatef
		glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle/* * rotateRectangle*/);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(brickobj[currentobj].object);

		//Check for collisions between bricks and buckets
		checkCollision(currentobj, "brickobj");
		it++;

	}

	//For Laser Objects, display them
	for( map<string,Base>::iterator it=laserobj.begin() ; it!=laserobj.end() ;  it++)
	{
		string currentobj = it->first;
		if(laserobj[currentobj].status==0)
			continue;
		if(laserobj[currentobj].x >=4 || laserobj[currentobj].x <=-4 || laserobj[currentobj].y>=4 || laserobj[currentobj].y<=-4)
		{
			laserobj[currentobj].status=0;
		}
		//cout << currentobj << endl;
		glm::mat4 MVP;
		Matrices.model = glm::mat4(1.0f);
		//  brickobj[currentobj].y-=brickobj[currentobj].y_speed;
		glm::mat4 translateRectangle = glm::translate (glm::vec3(laserobj[currentobj].x, laserobj[currentobj].y, 0));        // glTranslatef
		glm::mat4 rotateRectangle = glm::rotate((float)(laserobj[currentobj].angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle *rotateRectangle);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(laserobj[currentobj].object);

		laserobj[currentobj].x += 0.25*cos(laserobj[currentobj].angle*M_PI/180.0f);
		laserobj[currentobj].y += 0.25*sin(laserobj[currentobj].angle*M_PI/180.0f);

		checkCollision(currentobj, "laserobj");
	}

	//For Mirror Objects, display them
	for( map<string,Base>::iterator it=mirrorobj.begin() ; it!=mirrorobj.end() ;  it++)
	{
		string currentobj = it->first;
		//  cout << currentobj << endl;
		glm::mat4 MVP;
		Matrices.model = glm::mat4(1.0f);
		//  brickobj[currentobj].y-=brickobj[currentobj].y_speed;

		glm::mat4 translateRectangle = glm::translate (glm::vec3(mirrorobj[currentobj].x, mirrorobj[currentobj].y, 0));        // glTranslatef
		glm::mat4 rotateRectangle = glm::rotate((float)(mirrorobj[currentobj].angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle *rotateRectangle);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(mirrorobj[currentobj].object);

		//  laserobj[currentobj].x += 0.25*cos(laserobj[currentobj].angle*M_PI/180.0f);
		//  laserobj[currentobj].y += 0.25*sin(laserobj[currentobj].angle*M_PI/180.0f);
		checkCollision(currentobj, "mirrorobj");
	}


  //For Scoreboard Objects, display them
	for( map<string,Base>::iterator it=scoreboardobj.begin() ; it!=scoreboardobj.end() ;  it++)
	{
		string currentobj = it->first;
   if(scoreboardobj[currentobj].status==0)
      continue;
		//  cout << currentobj << endl;
		glm::mat4 MVP;
		Matrices.model = glm::mat4(1.0f);
		//  brickobj[currentobj].y-=brickobj[currentobj].y_speed;

		glm::mat4 translateRectangle = glm::translate (glm::vec3(scoreboardobj[currentobj].x, scoreboardobj[currentobj].y, 0));        // glTranslatef
	//	glm::mat4 rotateRectangle = glm::rotate((float)(mirrorobj[currentobj].angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateRectangle/* *rotateRectangle*/);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(scoreboardobj[currentobj].object);

		//  laserobj[currentobj].x += 0.25*cos(laserobj[currentobj].angle*M_PI/180.0f);
		//  laserobj[currentobj].y += 0.25*sin(laserobj[currentobj].angle*M_PI/180.0f);

	}

	// Increment angles
	float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		//        exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Laser Cannon Game", NULL, NULL);

	if (!window) {
		glfwTerminate();
		//        exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetScrollCallback(window, mousescroll); //enable mouse scrolling

	return window;
}


//Checking collision between different set of objects depending on the check variable
//0 = black
//1 = red
//2 = green
void checkCollision(string key, string check)
{

	if(check=="brickobj")
	{
		map<string, Base>::iterator iter = brickobj.find(key) ;
		if( brickobj[key].col_type==2 )
		{
			if(fabs(brickobj[key].x - bucketobj["bucket2"].x)<brickobj[key].width/2 + bucketobj["bucket2"].width/2
					&& fabs(brickobj[key].y - bucketobj["bucket2"].y )<brickobj[key].height/2 + bucketobj["bucket2"].height/2  && brickobj[key].status==1)
			{
				brickobj[key].status=0;
        globalscore+=2;
				//brickobj.erase(iter);  //Erasing the map gives a segmentation fault, use status, 2nd trial does not! DO NOT USE IT! :)
				//delete brickobj[key];
			}
		}

		if( brickobj[key].col_type == 1)
		{    //cout << brickobj[key].x;
			if(fabs(brickobj[key].x - bucketobj["bucket1"].x)<brickobj[key].width/2 + bucketobj["bucket1"].width/2
					&& fabs(brickobj[key].y - bucketobj["bucket1"].y )<brickobj[key].height/2 + bucketobj["bucket1"].height/2  && brickobj[key].status==1)
			{
				brickobj[key].status=0;
        globalscore+=2;
				//brickobj.erase(iter);
				//delete brickobj[key];
			}
		}

    //If black brick hits the bucket, game is over.
    if( brickobj[key].col_type == 0)
		{    //cout << brickobj[key].x;
			if((fabs(brickobj[key].x - bucketobj["bucket1"].x)<brickobj[key].width/2 + bucketobj["bucket1"].width/2
					&& fabs(brickobj[key].y - bucketobj["bucket1"].y )<brickobj[key].height/2 + bucketobj["bucket1"].height/2  && brickobj[key].status==1)
          || (fabs(brickobj[key].x - bucketobj["bucket2"].x)<brickobj[key].width/2 + bucketobj["bucket2"].width/2
              && fabs(brickobj[key].y - bucketobj["bucket2"].y )<brickobj[key].height/2 + bucketobj["bucket2"].height/2  && brickobj[key].status==1))
      {
				//quit(window);
        game_status=0;
				//brickobj.erase(iter);
				//delete brickobj[key];
			}
		}
	}


	else if (check=="laserobj")
	{
		//Collision of laser with brick
		for( std::map<string,Base>::iterator it=brickobj.begin() ; it!=brickobj.end() ;  it++)
		{
			string currentobj = it->first;

			if( brickobj[currentobj].col_type == 0 && laserobj[key].status==1) //Black bricks vanish
			{
				if(fabs(brickobj[currentobj].x - laserobj[key].x) < brickobj[currentobj].width/2 + laserobj[key].width/2
						&& fabs(brickobj[currentobj].y - laserobj[key].y )< brickobj[currentobj].height/2 + laserobj[key].height/2  && brickobj[currentobj].status==1)
				{
					brickobj[currentobj].status=0;
					laserobj[key].status=0;
          globalscore+=3;
				}
			}

			if( brickobj[currentobj].col_type == 1 && laserobj[key].status==1) //Red bricks vanish
			{
				if(fabs(brickobj[currentobj].x - laserobj[key].x) < brickobj[currentobj].width/2 + laserobj[key].width/2
						&& fabs(brickobj[currentobj].y - laserobj[key].y )< brickobj[currentobj].height/2 + laserobj[key].height/2  && brickobj[currentobj].status==1)
				{
					brickobj[currentobj].status=0;
					laserobj[key].status=0;
          globalscore-=1;
				}
			}

			if( brickobj[currentobj].col_type == 2 && laserobj[key].status==1) //Green bricks vanish
			{
				if(fabs(brickobj[currentobj].x - laserobj[key].x) < brickobj[currentobj].width/2 + laserobj[key].width/2
						&& fabs(brickobj[currentobj].y - laserobj[key].y )< brickobj[currentobj].height/2 + laserobj[key].height/2  && brickobj[currentobj].status==1)
				{
					brickobj[currentobj].status=0;
					laserobj[key].status=0;
          globalscore-=1;
				}
			}
		}
		//Collision of laser with mirror
		for( std::map<string,Base>::iterator it=mirrorobj.begin() ; it!=mirrorobj.end() ;  it++)
		{
			string currentobj = it->first;
			/*
			   float dis1,dis2;
			   dis2=laserobj[key].y+laserobj[key].width*0.25*sin(laserobj[key].angle*M_PI/180.0f);
			   dis1=laserobj[key].x+laserobj[key].width*0.25*cos(laserobj[key].angle*M_PI/180.0f);
			   float dis3=(dis1-mirrorobj[currentobj].x)/cos(mirrorobj[currentobj].angle*M_PI/180.0f);
			   float dis4=(dis2-mirrorobj[currentobj].y)/sin(mirrorobj[currentobj].angle*M_PI/18.0f);
			   if (dis3 > -1 && dis3 < 1 && dis4 > -1 && dis4<1 && abs(dis3-dis4)<=0.1)
			   {
			   laserobj[key].angle = 2*mirrorobj[currentobj].angle - laserobj[key].angle;
			   }


*/


			/*  GLfloat dis1;
			    GLfloat dis2;
			    dis2=laserobj[key].y+laserobj[key].width*sin(laserobj[key].angle*M_PI/180.0f);
			    dis1=laserobj[key].x+laserobj[key].width*cos(laserobj[key].angle*M_PI/180.0f);

			    GLfloat dis3=(dis1-mirrorobj[currentobj].x)/cos(mirrorobj[currentobj].angle*M_PI/180.0f);
			    GLfloat dis4=(dis2-mirrorobj[currentobj].y)/sin(mirrorobj[currentobj].angle*M_PI/18.0f);
			    if (laserobj[key].status==1 && dis3 > -laserobj[key].width/2 && dis3 < laserobj[key].width/2 && dis4 > -laserobj[key].height/2 && dis4<laserobj[key].height/2 && fabs(dis3-dis4)<=0.1)
			    {
			//cout << "collide mirror" << endl;
			laserobj[key].angle = 2*mirrorobj[currentobj].angle - laserobj[key].angle;
			}
			*/

			if( laserobj[key].status==1)
			{
				if(fabs( mirrorobj[currentobj].x- laserobj[key].x) < mirrorobj[currentobj].width/2 + laserobj[key].width/2+0.14
						&& fabs(mirrorobj[currentobj].y - laserobj[key].y )< mirrorobj[currentobj].height/2 + laserobj[key].height/2+0.14)
				{
					laserobj[key].angle=2*mirrorobj[currentobj].angle-laserobj[key].angle;
				}
			}
		}
	}

	//else if (check=="mirrorobj")

}


/*Create segments */
void createSegment(GLdouble pos)
{
  color cloudwhite = {0.690, 0.769, 0.871};
  createRectangle("one",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,pos,(-pos)-0.15,0.3,0.08,"scoreboardobj");
  createRectangle("two",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,pos+0.2,(-pos),0.08,0.3,"scoreboardobj");
  createRectangle("three",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,pos+0.4,(-pos)-0.15,0.3,0.08,"scoreboardobj");
  createRectangle("four",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,pos+0.4,(-pos)-0.45,0.3,0.08,"scoreboardobj");
  createRectangle("five",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,pos+0.2,(-pos)-0.6,0.08,0.3,"scoreboardobj");
  createRectangle("six",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,pos,(-pos)-0.45,0.3,0.08,"scoreboardobj");
  createRectangle("seven",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,pos+0.2,(-pos)-0.3,0.08,0.3,"scoreboardobj");

  //pos+=0.2;
  GLdouble newpos = pos + 0.55;
  createRectangle("eight",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,newpos,(-pos)-0.15,0.3,0.08,"scoreboardobj");
  createRectangle("nine",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,newpos+0.2,(-pos),0.08,0.3,"scoreboardobj");
  createRectangle("ten",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,newpos+0.4,(-pos)-0.15,0.3,0.08,"scoreboardobj");
  createRectangle("eleven",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,newpos+0.4,(-pos)-0.45,0.3,0.08,"scoreboardobj");
  createRectangle("twelve",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,newpos+0.2,(-pos)-0.6,0.08,0.3,"scoreboardobj");
  createRectangle("thirteen",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,newpos,(-pos)-0.45,0.3,0.08,"scoreboardobj");
  createRectangle("fourteen",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,newpos+0.2,(-pos)-0.3,0.08,0.3,"scoreboardobj");
}





/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/*Add colors */

	color grey = {168.0/255.0,168.0/255.0,168.0/255.0};
	color gold = {218.0/255.0,165.0/255.0,32.0/255.0};
	color coingold = {255.0/255.0,223.0/255.0,0.0/255.0};
	color red = {255.0/255.0,51.0/255.0,51.0/255.0};
	//color lightpink = {57/255.0,230/255.0,0/255.0};
	color green = {51/255.0,102/255.0,0/255.0};
	color black = {30/255.0,30/255.0,21/255.0};
	color blue = {0,0,1};
	color darkbrown = {46/255.0,46/255.0,31/255.0};
	color lightbrown = {95/255.0,63/255.0,32/255.0};
	color brown1 = {117/255.0,78/255.0,40/255.0};
	color brown2 = {134/255.0,89/255.0,40/255.0};
	color brown3 = {46/255.0,46/255.0,31/255.0};
	color cratebrown = {153/255.0,102/255.0,0/255.0};
	color cratebrown1 = {121/255.0,85/255.0,0/255.0};
	color cratebrown2 = {102/255.0,68/255.0,0/255.0};
	color skyblue2 = {113/255.0,185/255.0,209/255.0};
	color skyblue1 = {0.690, 0.769, 0.871};
	color skyblue = {132/255.0,217/255.0,245/255.0};
	color cloudwhite = {0.416, 0.353, 0.804};
	color cloudwhite1 = {204/255.0,255/255.0,255/255.0};
	color lightpink = {255/255.0,122/255.0,173/255.0};
	color darkpink = {255/255.0,51/255.0,119/255.0};
	color white = {255/255.0,255/255.0,255/255.0};
	color score = {117/255.0,78/255.0,40/255.0};




	/* Objects should be created before any other gl function and shaders */
	// Create the models
	//	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer

	//Create background image
	createRectangle("sky",10000,cloudwhite,cloudwhite,cloudwhite,cloudwhite,0,0,600,800,"backgroundobj");
	createCircle("sun",10000,skyblue1,3,3,0.5,100,"backgroundobj",1);
	createCircle("sun2", 10000, cloudwhite, 2.9,3.15,0.5,100,"backgroundobj", 1);


	//Create bucket objects
	createRectangle("bucket1",10000,red,red,red,red,-1.2,-3.6,1,1,"bucketobj");
	createRectangle("bucket2",10000,green,green,green,green,1.4,-3.6,1,1,"bucketobj");


	//Create cannon objects
	createRectangle("rear",10000,gold,gold,gold,gold,-3.9,1,0.45,0.6,"cannonobj");
	createRectangle("front",10000,grey,grey,grey,grey,-3.6,1,0.2,0.4,"cannonobj");


	//Create brick objects
	int iterator;

	int y=4;
	for(iterator=1; iterator<=60; iterator++)
	{
		//GLfloat randfloatcol= static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(255.0)));
		//color randcol = { randfloatcol, randfloatcol, randfloatcol};
		//cout << randcol.r << randcol.g << randcol.b;
		//Generating random float value
		GLfloat randfloat1= -2.5 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2.3)));
		GLfloat randfloat2= 1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.5)));
		//std::uniform_int_distribution<int> uni(-1,3); // guaranteed unbiased
		//auto rand_int = uni(rng);
		//  string pi = to_string(randfloat);
		string name = "brick";
		name.append(to_string(iterator));
		GLint randcol = rand()%3 + 1;
		//Randomly generating colorful bricks in two segments in two set of float ranges
		if(randcol==1)
		{
			GLint random_var = rand()%2 + 1;
			if(random_var==1)
				createBrick(name,10000,green,green,green,green,randfloat1,y,0.3,0.2,"brickobj",2);
			else
				createBrick(name,10000,green,green,green,green,randfloat2,y,0.3,0.2,"brickobj",2);
			//checkCollision(brickobj[name], bucketobj["bucket1"]);
		}
		else if(randcol==2)
		{
			GLint random_var = rand()%2 + 1;
			if(random_var==1)
				createBrick(name,10000,black,black, black, black,randfloat1,y,0.3,0.2,"brickobj",0);
			else
				createBrick(name,10000,black,black, black, black,randfloat2,y,0.3,0.2,"brickobj",0);

		}
		else if(randcol==3)
		{
			GLint random_var = rand()%2 + 1;
			if(random_var==1)
				createBrick(name,10000,red,red, red,red,randfloat1,y,0.3,0.2,"brickobj",1);
			else
				createBrick(name,10000,red,red, red,red,randfloat2,y,0.3,0.2,"brickobj",1);
		}
		y+=1;
	}


	//Create Laser Beam
	//Object is created via createCircle when user presses the key, not here

	//Create Mirror objects
	createMirror("hey",10000,darkpink,darkpink,darkpink,darkpink,0.1,-2.3,0.06,0.8,"mirrorobj", -120);
	createMirror("hey2",10000,darkpink,darkpink,darkpink,darkpink,0.1,2.6,0.06,0.8,"mirrorobj", 140);
	createMirror("hey3",10000,darkpink,darkpink,darkpink,darkpink,2.9,1.4,0.06,0.8,"mirrorobj", 140);
	createMirror("hey4",10000,darkpink,darkpink,darkpink,darkpink,3.1,-1.3,0.06,0.8,"mirrorobj", -140);



  //Create Seven Segment Displays
  createSegment(-3.8);



	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.0f, 0.0f, 0.0f, 0.1f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;


  GLFWwindow* window ;
window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;
	double brick_time = glfwGetTime();
	double now_time;


	/* Draw in loop */
	while (!glfwWindowShouldClose(window) && game_status ==1  && globalscore >=-10 && globalscore <=100) {

		now_time=glfwGetTime();

		//  if(now_time - brick_time >= 0.15)
		// OpenGL Draw commands
    segmentclear();

		draw(window);


		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
	}

  //Print final score
  if(globalscore >=100)
    cout <<"CONGRATULATIONS. YOU WIN!!!" <<endl;
  else
    cout <<"SORRY. YOU LOSE!" <<endl;
  cout << "Your final score is " << globalscore << endl;

	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}
