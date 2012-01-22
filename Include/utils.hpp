#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <GL/gl.h>
#include <SDL/SDL_image.h>

#include <exception>
#include <stdexcept>

#include "const.hpp"
#include <except.hpp>
		

inline int min2( const int n );
inline void getNormal( const float *p1, const float *p2, const float *p3, float *normal );
inline void getNormal2( const float *p1, const float *p2, const float *p3, float *normal );
inline void glDebugBegin( GLenum mode );
inline void glDebugEnd();
inline void setLight();
inline void setLightPos();
void glSetup();


inline int min2( const int n )
{
	for ( int i = 1;; i <<= 1 )
		if ( n <= i ) return i;
}

inline void getNormal( const float *p1, const float *p2, const float *p3, float *normal )
{
	float a[3], b[3];
	a[0]=p2[0]-p1[0]; a[1]=p2[1]-p1[1]; a[2]=p2[2]-p1[2];
	b[0]=p3[0]-p1[0]; b[1]=p3[1]-p1[1]; b[2]=p3[2]-p1[2];
	normal[0]=a[1]*b[2]-a[2]*b[1];
	normal[1]=a[2]*b[0]-a[0]*b[2];
	normal[2]=a[0]*b[1]-a[1]*b[0];
}

inline void getNormal2( const float *p1, const float *p2, const float *p3, float *normal )
{
	float a[3], b[3];
	a[0]=p2[0]-p1[0]; a[1]=p2[1]-p1[1]; a[2]=p2[2]-p1[2];
	b[0]=p3[0]-p1[0]; b[1]=p3[1]-p1[1]; b[2]=p3[2]-p1[2];
	normal[0]+=a[1]*b[2]-a[2]*b[1];
	normal[1]+=a[2]*b[0]-a[0]*b[2];
	normal[2]+=a[0]*b[1]-a[1]*b[0];
}

inline void glDebugBegin( GLenum mode )
{
    glDisable(GL_TEXTURE_2D);
	glDisable( GL_LIGHTING );
	glDisable( GL_FOG );
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_COLOR_MATERIAL );
	glBegin( mode );
}

inline void glDebugEnd()
{
	glEnd();
	glDisable( GL_COLOR_MATERIAL );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_FOG );
	glEnable( GL_LIGHTING );
    glEnable(GL_TEXTURE_2D);
}

inline void setLightPos()
{
	static const float lpos[4] = { 1.0f, -2.0f, 1.0f, 0.0f };
	glLightfv( GL_LIGHT0, GL_POSITION, lpos );
}

inline void setLight()
{
	//~ static const float zero[4] = { -1.0f, -1.0f, -1.0f, 1.0f };
	static const float one[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//~ static const float diff[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
	static const float half[4] = { 0.5f, 0.5f, 0.5f, 1.0f };

	glLightfv( GL_LIGHT0, GL_AMBIENT, half );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, one );
	//glLightfv( GL_LIGHT0, GL_SPECULAR, half );
	setLightPos();
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
}

#endif
