#include <terrain_segment.hpp>
#include <engine.hpp>
#include <sstream>

TerrainSegment::TerrainSegment() : displayLists(0), size( 0 ), W( 0 ), H( 0 )
{
}

TerrainSegment::~TerrainSegment()
{
    if (displayLists){
        glDeleteLists(displayLists, size2+1);
        displayLists = 0;
    }
}

void TerrainSegment::set(int s, int w, int h)
{ 
    if (displayLists){
        glDeleteLists(displayLists, size2+1);
        displayLists = 0;
    }
    size = s;
    size2 =__builtin_ctz(size);
    W = w;
    H = h;
}

void TerrainSegment::draw( const CamInfo& camInfo )
{
	throw LogicError("Incomplete");
}

void TerrainSegment::tempDraw( const CamInfo& cam )
{
    if (displayLists){
	float dist = hypotf(xpos+cam.x, ypos+cam.y);
	if (dist > cam.dist+size*1.42) return;
        int lod = std::min(int(dist/cam.dist*(size2+1)), size2);
        //~ int lod = trunc(hypotf(xpos+cam.x, ypos+cam.y)/cam.dist*(size2-1));
        //~ if (lod > size2) return;
        GLTERRORCHECK;
	    tex.safeBind();
        glCallList(displayLists+lod);
        GLTERRORCHECK;
    }
}

void TerrainSegment::infoDraw( const CamInfo& cam )
{
	float dist = hypotf(xpos+cam.x, ypos+cam.y);
	if (dist > cam.dist+size) return;
        int lod = std::min(int(dist/cam.dist*(size2+1)), size2);
    std::stringstream s;
    s << lod;
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
		glDisable(GL_CULL_FACE);
		glTranslatef(bx, by, bz+5.0);
		glRotatef(90, -1.0f, 0.0f, 0.0f);
		Engine::defFont -> render(s.str(),0,0, 2);
		glEnable(GL_CULL_FACE);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_LIGHTING);
	glEnable( GL_FOG );
	glDisable( GL_COLOR_MATERIAL );
}

void TerrainSegment::createDisplayLists( Point3f* vertexArray,
        Point3f* normalArray, Point2f* texCoordArray )
{
    if (displayLists) return;
    GLTERRORCHECK;
    displayLists = glGenLists(size2+1);
    if (!displayLists) throw RuntimeError("glGenLists failed");

    GLTERRORCHECK;

	bx = vertexArray[0].x();
	by = vertexArray[0].y();
	bz = vertexArray[0].z();
	
    xpos = (vertexArray[0].x()+vertexArray[W-1].x())/2.0f;
    ypos = (vertexArray[0].y()+vertexArray[(H-1)*(size+1)].y())/2.0f;
    for (int i = 0; i <= size2; ++i ){
        int off = 1<<i;
        glNewList(displayLists+i, GL_COMPILE);
        GLTERRORCHECK;
        {
            glMatrixMode( GL_TEXTURE );
            glPushMatrix();
            glLoadIdentity();
            glTranslatef( -tex.x1(), -tex.y1(), 0.0f );
            glScalef( tex.x2()-tex.x1(), tex.y2()-tex.y1(), 1.0f );

            for ( int iy = 0; iy < H; iy+=off ){
                glBegin( GL_TRIANGLE_STRIP );
                for ( int ix = 0; ix <= W; ix+=off ){
                    int i1 = iy*(size+1)+ix;
                    int i2 = (iy+off)*(size+1)+ix;
                    glTexCoord2fv( texCoordArray[i1] );
                    glNormal3fv( normalArray[i1] );
                    glVertex3fv( vertexArray[i1] );
                    glTexCoord2fv( texCoordArray[i2] );
                    glNormal3fv( normalArray[i2] );
                    glVertex3fv( vertexArray[i2] );
                }
                glEnd();
            }
            glMatrixMode( GL_TEXTURE );
            glPopMatrix();
        }
        glEndList();
        GLTERRORCHECK;
    }
}

