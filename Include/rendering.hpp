#ifndef _RENDERING_HPP_
#define _RENDERING_HPP_

#include <GL/gl.h>
#include <GL/glu.h>

static const float viewAngle = 60.0f;
static const float nearClip = 1.0f;

struct CamInfo
{
	float x, y, z, xrot, zrot, dist, aspect;
	CamInfo();
	CamInfo( float _x, float _y, float _z, float _xrot, float _zrot, float _dist, float _aspect ) :
			x( _x ), y( _y ), z( _z ), xrot( _xrot ), zrot( _zrot ), dist( _dist ), aspect( _aspect ) {}
	void set()
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		gluPerspective( viewAngle, aspect, nearClip, dist );
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glRotatef( xrot, 1.0f, 0.0f, 0.0f );
		glRotatef( zrot, 0.0f, 0.0f, 1.0f );
		glTranslatef( x, y, z );
	}
};

enum dm
{
	dmElement	=	0,
	dmElements	=	1,
	dmRange		=	2
};

struct renderinfostruct
{
	bool alphaMethod;
	dm drawMethod;
	renderinfostruct();
	renderinfostruct( bool aM, dm dM ) : alphaMethod( aM ), drawMethod( dM ) {}
};

#endif //_RENDERING_HPP_
