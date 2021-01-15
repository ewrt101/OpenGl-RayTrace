/*==================================================================================
* COSC 363  Computer Graphics (2020)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf, Lab08.pdf for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "Plane.h"
#include "Cylinder.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>
#include "TextureBMP.h"
using namespace std;

const float WIDTH = 20.0;  
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;

const bool SS = true;

vector<SceneObject*> sceneObjects;

TextureBMP texture;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 lightPos(10, 40, -3);					//Light's position
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

	if (ray.index == 4)
	{
		//Stripe pattern
		int stripeWidth = 5;
		int iz = (ray.hit.z+100) / stripeWidth;
		int ix = (ray.hit.x+100) / stripeWidth;
		int k = iz % 2; //2 colors
		int j = ix % 2; //2 colors

		glm::vec3 color1 = glm::vec3(0.4, 1, 0.7);
		glm::vec3 color2 = glm::vec3(1, 1, 0.6);

		if(k == 0 && j == 0){
			color = color1;
		}else if (k != 0 && j == 0){
			color = color2;
		}else if (k == 0 && j != 0){
			color = color2;
		}
		else {
			color = color1;
		}
		obj->setColor(color);
		/*
		float texcoords = (ray.hit.x - -15)/(5 - -15);
		float texcoordt =  (ray.hit.z - -100)/(-90 - -100);
		if(texcoords > 0 && texcoords < 1 &&
		texcoordt > 0 && texcoordt < 1)
		{
		color=texture.getColorAt(texcoords, texcoordt);
		obj->setColor(color);
		}
		*/
		
	}

	if (ray.index == 5)
	{
		glm::vec3 color1 = glm::vec3(0, 0.5, 1);
		glm::vec3 color2 = glm::vec3(1, 0.5, 0);

		float results_1 = sin(ray.hit.y) * 2;
		color = color1 * results_1;
		float results_2 = sin(ray.hit.x) * 2;
		color += (color2 * results_2);
		obj->setColor(color);
	}



	color = obj->lighting(lightPos,-ray.dir,ray.hit);						//Object's colour

	glm::vec3 lightVec = lightPos - ray.hit;
	Ray shadowRay(ray.hit, lightVec);
	shadowRay.closestPt(sceneObjects);
	if ((shadowRay.index > -1) && (shadowRay.dist < glm::length(lightVec))){
		SceneObject* hitobj = sceneObjects[shadowRay.index];
		if (hitobj->isTransparent()){
			float factor = hitobj->getTransparencyCoeff();
			
			float secondFactor = (0.2f + (0.8f*factor));
			//color = secondFactor * obj->getColor();
			glm::vec3 temp = glm::vec3(1,1,1) - hitobj->getColor();
			//color = (secondFactor) * (color);
			color = (secondFactor) * (color - (temp * 0.2f));
		}else if(hitobj->isRefractive()){
			color = 0.7f * obj->getColor();
		}
		else{
			color = 0.2f * obj->getColor(); //0.2 = ambient scale factor
		}
		
	}


	


	if (obj->isTransparent() && step < MAX_STEPS)
	{

		float rho = obj->getTransparencyCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray refRay(ray.hit, ray.dir);
		refRay.closestPt(sceneObjects);
		Ray reflectedRay(refRay.hit, refRay.dir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		//color = obj->lighting(lightPos,-ray.dir,ray.hit) * (1-rho);
		color = color * (1-rho);
		color += (rho * reflectedColor);
	}

	if (obj->isRefractive() && step < MAX_STEPS)
	{

		float etc = (obj->getRefractiveIndex())/(obj->getRefractionCoeff());
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 g = glm::refract(ray.dir, normalVec, etc);
		Ray refrRay(ray.hit, g);
		refrRay.closestPt(sceneObjects);
		glm::vec3 mormalVec = obj->normal(refrRay.hit);
		glm::vec3 h = glm::refract(ray.dir, -mormalVec, 1/etc);
		Ray refractedRay(refrRay.hit, h);
		glm::vec3 refractedColor = trace(refractedRay, step + 1);
		//color = color + (refractedColor);
		//color = color * (1-rho);
		color = refractedColor;
	}

	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
	} 


	return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for(int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;
			if(!SS){
				//non SS
				glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray

		    	Ray ray = Ray(eye, dir);

				glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value
				glColor3f(col.r, col.g, col.b);
			}else{
				//supersample
				glm::vec3 dir1(xp+0.25*cellX, yp+0.25*cellY, -EDIST);
				Ray ray1 = Ray(eye, dir1);
				glm::vec3 dir2(xp+0.25*cellX, yp+0.75*cellY, -EDIST);
				Ray ray2 = Ray(eye, dir2);
				glm::vec3 dir3(xp+0.75*cellX, yp+0.25*cellY, -EDIST);
				Ray ray3 = Ray(eye, dir3);
				glm::vec3 dir4(xp+0.75*cellX, yp+0.75*cellY, -EDIST);
				Ray ray4 = Ray(eye, dir4);

				glm::vec3 col1 = trace (ray1, 1);
				glm::vec3 col2 = trace (ray2, 1);
				glm::vec3 col3 = trace (ray3, 1);
				glm::vec3 col4 = trace (ray4, 1);
				float colr = (col1.r + col2.r + col3.r + col4.r)/4;
				float colg = (col1.g + col2.g + col3.g + col4.g)/4;
				float colb = (col1.b + col2.b + col3.b + col4.b)/4;
				glColor3f(colr, colg, colb);
			}
		    
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}



//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);


	texture = TextureBMP("Butterfly.bmp");


	Sphere *sphere1 = new Sphere(glm::vec3(-10.0, 10.0, -90.0), 10.0);
	sphere1->setColor(glm::vec3(0, 1, 0));   //Set colour to blue
	//sphere1->setShininess(5);
	//sphere1->setTransparency(true,0.8);
	//sphere1->setReflectivity(true, 0.9);
	//sphere1->setRefractivity(true,1.5,1);
	sceneObjects.push_back(sphere1);		 //Add sphere to scene objects

	Sphere *sphere2 = new Sphere(glm::vec3(5, 5, -70.0), 4);
	sphere2->setColor(glm::vec3(1, 0, 1));
	//sphere2->setColor(glm::vec3(0, 0, 0));
	//sphere2->setReflectivity(true, 0.9);
	//sphere2->setTransparency(true,0.5);
	//sphere2->setRefractivity(true,1.15,1);
	//sceneObjects.push_back(sphere2);		 //Add sphere to scene objects

	Sphere *sphere3 = new Sphere(glm::vec3(10, -5, -60.0), 5.0);
	sphere3->setColor(glm::vec3(0, 1, 0)); 
	//sphere3->setTransparency(true,0.9);
	sphere3->setReflectivity(true, 0.25);
	sphere3->setRefractivity(true,1.01,1);
	sceneObjects.push_back(sphere3);

	Sphere *sphere4 = new Sphere(glm::vec3(8, 10, -60.0), 3.0);
	sphere4->setColor(glm::vec3(1, 0, 0));   //Set colour to blue
	//sphere4->setTransparency(true,0);
	sphere4->setReflectivity(true, 0.9);
	sceneObjects.push_back(sphere4);		 //Add sphere to scene objects

	Sphere *sphere5 = new Sphere(glm::vec3(-3, -10, -60.0), 3.0);
	sphere5->setColor(glm::vec3(0, 0, 1)); 
	sphere5->setTransparency(true,0.75);
	sphere5->setReflectivity(true, 0.2);
	//sphere5->setRefractivity(true,1.01,1);
	sceneObjects.push_back(sphere5);




	Plane *plane = new Plane (glm::vec3(-100., -15, -40), //Point A
		glm::vec3(100., -15, -40), //Point B
		glm::vec3(100., -15, -200), //Point C
		glm::vec3(-100., -15, -200)); //Point D

	plane->setSpecularity(false);

	sceneObjects.push_back(plane);

	double wall_distacne = -150;

	Plane *plane2 = new Plane (glm::vec3(-50., -15, wall_distacne), //Point A
		glm::vec3(50., -15, wall_distacne), //Point B
		glm::vec3(50., 40, wall_distacne), //Point C
		glm::vec3(-50., 40, wall_distacne)); //Point D

	plane2->setSpecularity(false);
	plane2->setColor(glm::vec3(1, 1, 0));

	sceneObjects.push_back(plane2);
	//front
	Plane *plane3 = new Plane (glm::vec3(-15., -10, -75), //Point A
		glm::vec3(-10., -10, -75), //Point B
		glm::vec3(-10., -5, -75), //Point C
		glm::vec3(-15., -5, -75)); //Point D
	plane3->setColor(glm::vec3(1, 0.5, 1));
	sceneObjects.push_back(plane3);
	//right
	Plane *plane4 = new Plane (glm::vec3(-10., -10, -75), //Point A
		glm::vec3(-10., -10, -80), //Point B
		glm::vec3(-10., -5, -80), //Point C
		glm::vec3(-10., -5, -75)); //Point D
	plane4->setColor(glm::vec3(1, 0.5, 1));
	sceneObjects.push_back(plane4);
	//left
	Plane *plane5 = new Plane (glm::vec3(-15., -10, -75), //Point A
		glm::vec3(-15., -10, -80), //Point B
		glm::vec3(-15., -5, -80), //Point C
		glm::vec3(-15., -5, -75)); //Point D
	plane5->setColor(glm::vec3(1, 0.5, 1));
	sceneObjects.push_back(plane5);
	//back
	Plane *plane6 = new Plane (glm::vec3(-15., -10, -80), //Point A
		glm::vec3(-10., -10, -80), //Point B
		glm::vec3(-10., -5, -80), //Point C
		glm::vec3(-15., -5, -80)); //Point D
	plane6->setColor(glm::vec3(1, 0.5, 1));
	sceneObjects.push_back(plane6);
	//bottom
	Plane *plane7 = new Plane (glm::vec3(-10., -10, -75), //Point A
		glm::vec3(-10., -10, -80), //Point B
		glm::vec3(-15., -10, -80), //Point C
		glm::vec3(-15., -10, -75)); //Point D
	plane7->setColor(glm::vec3(1, 0.5, 1));
	sceneObjects.push_back(plane7);
	//top
	Plane *plane8 = new Plane (glm::vec3(-10., -5, -75), //Point A
		glm::vec3(-10., -5, -80), //Point B
		glm::vec3(-15., -5, -80), //Point C
		glm::vec3(-15., -5, -75)); //Point D
	plane8->setColor(glm::vec3(1, 0.5, 1));
	sceneObjects.push_back(plane8);

	//Cylinder *cylinder = new Cylinder(glm::vec3(-10.0, 10.0, -90.0),5,5);
	//cylinder->setColor(glm::vec3(1, 1, 1));
	//sceneObjects.push_back(cylinder);

}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
