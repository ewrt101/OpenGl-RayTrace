
#include "Cylinder.h"
#include <math.h>
#include <iostream>
#include <string>
using namespace std;

/**
* Cylinder's intersection method.  The input is a ray. 
*/
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir) 
{
    /*
    glm::vec3 vdif = p0 - center;   //Vector s (see Slide 28)
    float b = glm::dot(dir, vdif);
    float len = glm::length(vdif);
    float c = len*len - radius*radius;
    float delta = b*b - c;
   
	if(fabs(delta) < 0.001) return -1.0; 
    if(delta < 0.0) return -1.0;

    float t1 = -b - sqrt(delta);
    float t2 = -b + sqrt(delta);
    if(fabs(t1) < 0.001 )
    {
        if (t2 > 0) return t2;
        else t1 = -1.0;
    }
    if(fabs(t2) < 0.001 ) t2 = -1.0;

	return (t1 < t2)? t1: t2;
    */
    float q = dir.x;
    float w = dir.z;
    float x = (p0.x-center.x);
    float z = (p0.z-center.z);
    float r = radius;

    float b = -q*x-w*z;
    
    float b_ = (q*q)*(r*r)-(x*x)*(q*q) + (w*w)*(r*r);
    float c  = 2*w*z*q*x+q*(x*x)+(w*w)*(x*x);
    float delta = b_ + c;
    //cout << delta << endl;
    //if(fabs(delta) < 0.001) return -1.0; 
    if(delta < 0.0) return -1.0;
    
    float t1 = -b - sqrt(delta);
    float t2 = -b + sqrt(delta);
    
    t1 = t1/(2*((dir.x * dir.x)+(dir.z * dir.z)));
    t2 = t2/(2*((dir.x * dir.x)+(dir.z * dir.z)));
    /*
    if(fabs(t1) < 0.001 )
    {
        if (t2 > 0) return t2;
        else t1 = -1.0;
    }
    if(fabs(t2) < 0.001 ) t2 = -1.0;
	return (t1 < t2)? t1: t2;
    */
   
   if (t1 < 0.001 && t2 < 0.001){
       cout << t1 << endl;
       if(t1<t2){
           return t1;
       }else {
           return t2;
       }
   }
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    glm::vec3 n = p - center;
    n = glm::normalize(n);
    return n;
}
