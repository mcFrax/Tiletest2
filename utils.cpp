#include "utils.hpp"
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_ttf.h>

#include <gl.hpp>


void glSetup()
{	
	glEnable(GL_NORMALIZE);
    
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
    glDisable(GL_COLOR_MATERIAL);
	
	glAlphaFunc( GL_GREATER, 0.5 );
	
	gltErrorCheck();
	
//	glShadeModel( GL_FLAT );

	static const float fog[4] = { 70.0f/255.0f, 180.0f/255.0f, 200.0f/255.0f, 1.0f };
	
    glClearColor( fog[0], fog[1], fog[2], fog[3] );
	
	glFogfv( GL_FOG_COLOR, fog );
	glFogi( GL_FOG_MODE, GL_LINEAR );
	//~ glFogi( GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH );
	glFogf( GL_FOG_DENSITY, 0.1 );
	glFogf( GL_FOG_START, DIST/5.0f );
	glFogf( GL_FOG_END, DIST );
	glEnable( GL_FOG );
	
	gltErrorCheck();
	
	setLight();
	
	gltErrorCheck();
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	gltErrorCheck();
}


