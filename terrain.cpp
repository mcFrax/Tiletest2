#include <terrain.hpp>
#include <bitmap.hpp>
#include <cstdio>
#include <cmath>
#include <except.hpp>

Terrain::Terrain() : W( 0 ), H( 0 ), deftex( 0 )
{
}

float Terrain::hmapGet( int x, int y )
{
    if ( x < 0 ) x = 0;
    else
        if ( x > W ) x = W;

    if ( y < 0 ) y = 0;
    else
        if ( y > H ) y = H;

    return hmap[ y*(W+1)+x ];
}

void Terrain::make( TerrainSegment & s, int x, int y )
{
    static const int d[5][2] = { {-1, 0}, {0, 1}, {1, 0}, {0 -1}, {-1, 0} };

    s.texCoordArray = seg.texCoordArray;

    s.vertexArray = new Point3f[ (seg.size+1) * (seg.size+1) ];
    s.normalArray = new Point3f[ (seg.size+1) * (seg.size+1) ];

    for ( int ix = 0; ix <= s.W; ix++ ){
        for ( int iy = 0; iy <= s.H; iy++ ){
            s.vertexArray[ iy*(seg.size+1)+ix ].set( tileSize*(x+ix), tileSize*(y+iy), hmap[ (y+iy)*(W+1)+(x+ix) ] );
            //~ log( TU, "v[%i, %i] => s.vertexArray[ %i ].set( %.2f, %.2f, %.2f )\n", ix, iy, iy*(seg.size+1)+ix, tileSize*(x+ix), tileSize*(y+iy), hmap[ (y+iy)*(W+1)+(x+ix) ] );
            s.normalArray[ iy*(seg.size+1)+ix ].set( 0.0f, 0.0f, 0.0f );
            Point3f p0( 0.0f, hmap[ (y+iy)*(W+1)+(x+ix) ], 0.0f ), p1, p2;
            for ( int i = 0; i < 4; i++ ){
                p1.set( d[i][0], hmapGet( x+ix+d[i][0], y+iy+d[i][1] ), d[i][1] );
                p2.set( d[i+1][0], hmapGet( x+ix+d[i+1][0], y+iy+d[i+1][1] ), d[i+1][1] );
                getNormal2( p0, p2, p1, s.normalArray[ iy*(seg.size+1)+ix ] );
            }
        }
    }
}

void Terrain::makeSegments()
{
    indices = new uint16_t[ seg.size*2+2 ];
    seg.texCoordArray = new Point2f[ (seg.size+1) * (seg.size+1) ];
    for ( unsigned int ix = 0; ix <= seg.size; ix++ )
        for ( unsigned int iy = 0; iy <= seg.size; iy++ )
            seg.texCoordArray[ iy*(seg.size+1) + ix ].set( 1.0f / seg.size * ix, 1.0f / seg.size * iy );

    int wb = W % seg.size == 0 ? seg.size : W % seg.size;	//szerokość brzegowych segmentów
    int hb = H % seg.size == 0 ? seg.size : H % seg.size;	//wysokość brzegowych segmentów

    seg.segments = new TerrainSegment [seg.W*seg.H];
    LOG(("Terrain: %i segments created\n", seg.H*seg.W));

    TerrainSegment::indices = indices;

    for ( int ix = 0; ix < seg.W; ix++ ){
        for ( int iy = 0; iy < seg.H; iy++ ){
            seg( ix, iy ).set( seg.size, ix == seg.W-1 ? wb : seg.size, iy == seg.H-1 ? hb : seg.size );
            seg( ix, iy ).tex = seg.map.find( std::make_pair( ix, iy ) ) == seg.map.end() ? texMap[ deftex ] : texMap[ seg.map[ std::make_pair( ix, iy ) ] ];
            make( seg( ix, iy ), ix*seg.size, iy*seg.size );
            seg( ix, iy ).createDisplayLists(seg.texCoordArray);
        }
    }

    seg.map.clear();
    hmap.reset();
}

void Terrain::loadHmap( const char * path )
{
    try
    {	
        int Bpp, bpp, colors;

        Bitmap bitmap;
        bitmap.logging = 0;
        bitmap.loadFromFile( path, 0, 8 | 16 );
        W = bitmap.width();
        H = bitmap.height();
        bpp = bitmap.bitsPerPixel();
        Bpp = bitmap.bytesPerPixel();
        colors = bitmap.colorsPerPixel();
        unsigned char *buff = bitmap.pixels();

        const float hFactor = 0.1f;

        float heightFactor = ( bpp == 8 ) ? hFactor : hFactor * 256.0f;

        if ( colors != 1 )
            throw std::runtime_error( "Heightmap must be grayscale bitmap" );

        hmap = new float[ W*H ];


        for ( int ix = 0; ix < W; ix++ )
            for ( int iy = 0; iy < W; iy++ ){
                int ph = 0;
                memcpy( reinterpret_cast<char *>( &ph ), &buff[ ( iy * W + ix ) * Bpp ], Bpp );
                hmap[ iy*W + ix ] = static_cast<float>( ph ) * heightFactor;
            }

        W--;
        H--;
        //~ log( TU, "Loaded heightmap %i x %i from \"%s\"\n", W, H, path );
    }
    catch( std::exception &ex )
    {
        throw MKRE_LOAD_ERROR( "heightmap", path, ex.what() );
    }
}

void Terrain::tempDraw( const CamInfo& camInfo, const bool correctCamPos )
{
    glCullFace( GL_FRONT );
    glEnable( GL_CULL_FACE );

    glDisable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    glEnable( GL_FOG );

    glActiveTexture( GL_TEXTURE0 );
    glEnable(GL_TEXTURE_2D);

    GLTERRORCHECK;

    for ( int ix = 0; ix < seg.W; ix++ )
        for ( int iy = 0; iy < seg.H; iy++ ){
            seg( ix, iy ).tempDraw(camInfo);
        }

    GLTERRORCHECK;

    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();
}

