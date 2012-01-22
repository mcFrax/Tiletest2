#include <terrain_segment.hpp>

uint16_t * TerrainSegment::indices = 0;

TerrainSegment::TerrainSegment() : displayLists(0), size( 0 ), W( 0 ), H( 0 )
{
}

void TerrainSegment::draw( const CamInfo& camInfo )
{
	throw LogicError("Incomplete");
}

void TerrainSegment::tempDraw( const CamInfo& camInfo )
{
    if (displayLists){
        GLTERRORCHECK;
	    tex.safeBind();
        glCallList(displayLists+lod(camInfo));
        GLTERRORCHECK;
    }
}

void TerrainSegment::createDisplayLists( Point2f* texCoordArray )
{
    if (displayLists) return;
    GLTERRORCHECK;
    displayLists = glGenLists(size2+1);
    if (!displayLists) throw RuntimeError("glGenLists failed");

    GLTERRORCHECK;

    LOG(("Creating display lists\n"));
    for (int i = 0; i <= size2; ++i ){
        int off = 1<<i;
        glNewList(displayLists+i, GL_COMPILE);
        GLTERRORCHECK;
        {
            glMatrixMode( GL_TEXTURE );
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
        }
        glEndList();
        GLTERRORCHECK;
    }
    vertexArray.reset();
    normalArray.reset();
}

