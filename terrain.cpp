#include <terrain.hpp>
#include <bitmap.hpp>

//~ #define TER_LINELOG	//Wypisywanie czytanych linii


void Segment::draw( const CamInfo& camInfo )
{
	throw LogicError("Incomplete");
}

void Segment::tempDraw( const CamInfo& camInfo )
{
    if (displayLists){
        GLTERRORCHECK;
	    tex.safeBind();
        glCallList(displayLists+lod(camInfo));
        GLTERRORCHECK;
    }
}

void Segment::createDisplayLists( Point2f* texCoordArray )
{
    if (displayLists) return;
    GLTERRORCHECK;
    displayLists = glGenLists(size2+1);
    if (!displayLists) throw RuntimeError("glGenLists failed");

    GLTERRORCHECK;

    LOG(("Creating display lists\n"));
    for (int i = 0; i <= size2; ++i ){
        int off = i+1;
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

static inline void readFileName( const char * path, int l, std::istream & is, std::string& s )
{
	try
	{
	is.ignore( 1000000, '"' );
	std::getline( is, s, '"' );
	}
	catch ( std::istream::failure & ex )
	{
		fprintf( stderr, "%s:%i: Error: Unable to read file name\n", path, l );
		throw std::runtime_error( "Unable to read file name" );
	}
}

static inline void lowercase( std::string & s )
{
	for ( std::string::iterator i = s.begin(); i!= s.end(); i++ )
		if ( *i <= 'Z' && *i >= 'A' )
			*i = *i-'A'+'a';
}

Terrain::Terrain() : W( 0 ), H( 0 ), deftex( 0 )
{
}

Terrain::Terrain( const char * path ) : W( 0 ), H( 0 ), deftex( 0 )
{
	loadFromMTT( path );
}

void Terrain::rHmap( const char * path, int l, std::istream &is )
{
	if ( hmap ){
		fprintf( stderr, "%s:%i: Error: Heightmap redefined\n", path, l );
		throw std::runtime_error( std::string( "Heightmap redefined" ) );
	}
	std::string hmf;
	readFileName( path, l, is, hmf );
	try
	{
		loadHmap( hmf.c_str() );
	}
	catch( std::exception& ex )
	{
		fprintf( stderr, "%s:%i: Error: Could not read heightmap file \"%s\"\n", path, l, hmf.c_str() );
		//~ throw std::runtime_error( "Could not read heightmap file" );
		throw;
	}
    LOG(("MTT: Heightmap %ux%u loaded from \"%s\"\n", W, H, hmf.c_str()));
	if ( seg.size != 0 ){
		seg.W = ceil( static_cast<double>( W )/seg.size );
		seg.H = ceil( static_cast<double>( H )/seg.size );
	}
}

void Terrain::rSsize( const char * path, int l, std::istream &is )
{
	if ( seg.size != 0 ){
		fprintf( stderr, "%s:%i: Error: Segsize redefined\n", path, l );
		throw std::runtime_error( std::string( "Segsize redefined" ) );
	}
	is >> seg.size;
	if ( __builtin_popcount(seg.size) != 1 ){
		fprintf( stderr, "%s:%i: Error: Segsize is not a power of 2\n", path, l );
		throw std::runtime_error( std::string( "Segsize is not a power of 2" ) );
	}
    LOG(("MTT: Segsize %u\n", seg.size));
	if ( hmap ){
		seg.W = ceil( static_cast<double>( W )/seg.size );
		seg.H = ceil( static_cast<double>( H )/seg.size );
	}
}

void Terrain::rTex( const char * path, int l, std::istream &is )
{
	unsigned int id;
	is >> id;
	if ( texMap( id ) )
		fprintf( stderr, "%s:%i: Warning: Texture %i redefined\n", path, l, id );
	std::string s;
	is >> s;
	if ( s.size() != 0 ){
		if ( s[0] == 'f' ){
			readFileName( path, l, is, s );
			try
			{
				texMap.load( id, s.c_str(), 1 );
			}
			catch ( std::exception &ex )
			{
				fprintf( stderr, "%s:%i: Error: Unable to load texture #%u\n", path, l, id );
				STDTHROW;
			}
            LOG(("MTT: Texture %u loaded from file \"%s\"\n", id, s.c_str()));
			return;
		}
		if ( s[0] == 't' ){
			unsigned int sid;
			is >> sid;
			if ( !texMap( sid ) ){
				fprintf( stderr, "%s:%i: Warning: Source texture #%u for texture #%u has not been defined. Using empty texture\n", path, l, sid, id );
			}
			int toffx, toffy, tw, th;
			is >> toffx >> toffy >> tw >> th;
			texMap.load( id, sid, toffx, toffy, tw, th );
            LOG(("MTT: Texture %u created from %u texture\n", id, sid));
			return;
		}
	}
	
	fprintf( stderr, "%s:%i: Error: After \"texture\" must be 'f' or 't'\n", path, l );
	throw std::runtime_error( std::string( "After \"texture\" must be 'f' or 't'" ) );
}

void Terrain::rSeg( const char * path, int l, std::istream &is )
{
	int xo, yo;
	unsigned int id;
	is >> xo >> yo >> id;
	if ( !texMap( id ) ){
		texMap[ id ];//, Texture() ) );
		fprintf( stderr, "%s:%i: Warning: Texture for segment [ %i, %i ] ( texture #%u ) has not been defined. Using empty texture\n", path, l, xo, yo, id );
	}
	if ( !seg.map.insert( std::make_pair( std::make_pair( xo, yo ), id ) ).second ){
		fprintf( stderr, "%s:%i: Warning: Segment [ %i, %i ] redefined\n", path, l, xo, yo );
	}
    LOG(("MTT: Segment [ %i, %i ] texture set to %i.\n", xo, yo, id));
}

void Terrain::rDeftex( const char * path, int l, std::istream &is )
{
	unsigned int id;
	is >> id;
	deftex = id;
	if ( !texMap( id ) ){
		texMap[ id ];
		fprintf( stderr, "%s:%i: Warning: Deftex ( texture #%u ) has not been defined. Using empty texture\n", path, l, id );
	}
    LOG(("MTT: Deftex set to%u.\n", id));
}

void Terrain::checkMTTVersion( const char * path, std::istream& is, float min, float max )
{
	std::string magic;
	float v;
	is >> magic >> v;
	if ( !is ){
		fprintf( stderr, "%s:1: Error: Could not read signature or version\n", path );
		throw std::runtime_error( std::string( "Could not read signature or version" ) );
	}
	if ( magic != "MTTtext" ){
		fprintf( stderr, "%s:1: Error: Bad signature, must be MTTtext\n", path );
		throw std::runtime_error( std::string( "Bad signature" ) );
	}
	if ( v < min ){
		fprintf( stderr, "%s:1: Error: Too low MTT version. Must be at least %f\n", path, min );
		throw std::runtime_error( std::string( "Too low MTT version" ) );
	}
	if ( v > max ){
		fprintf( stderr, "%s:1: Error: Too great MTT version. Must be not greater than %f\n", path, max );
		throw std::runtime_error( std::string( "Too great MTT version" ) );
	}
	is.ignore( 1000000, '\n' );
    LOG(("MTT: Loading MTT %f\n", v));
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

void Terrain::make( Segment & s, int x, int y )
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
	
	seg.segments = new Segment [seg.W*seg.H];
    LOG(("Terrain: %i segments created\n", seg.H*seg.W));
	
	Segment::indices = indices;

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

enum Cenum
{
	ce_heightmap,
	ce_texture,
	ce_segment,
	ce_segsize,
	ce_deftex
};

inline void makedict( std::map<std::string, Cenum> & dict )
{
	dict[ "heightmap" ] = ce_heightmap;
	dict[ "texture" ] = ce_texture;
	dict[ "segment" ] = ce_segment;
	dict[ "segsize" ] = ce_segsize;
	dict[ "deftex" ] = ce_deftex;
}

void Terrain::loadFromMTT( const char * path )
{
	try
	{
	
	//~ reset();
	std::map<std::string, Cenum> dict;
	makedict( dict );
	std::ifstream fin( path, std::ios::in );
	if( !fin ){
		fprintf( stderr, "%s: Error: Could not open file\n", path );
		throw std::runtime_error( "Could not open file" );
	}
	
	std::string line;
	
	bool bdeftex = 0;
	
	checkMTTVersion( path, fin, 0.0, 0.1 );
	
	// Przetwarzanie linii:
	for ( int l = 2; !fin.eof(); l++ ){	// l == numer linii
		std::getline( fin, line );
		#ifdef TER_LINELOG
			LOG(( "Processing line %i \"%s\"\n", l, line.c_str() ));
		#endif
		for ( std::string::iterator i = line.begin(); i < line.end(); i++ ){
			if ( *i == CommentChar ){
				line.erase( i, line.end() );
				break;
			}
		}
		
		if ( line.empty() ) continue;
		
		try
		{
			std::istringstream is( line );
			is.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
			std::string command;
			is >> command;
			lowercase( command );
			std::map<std::string, Cenum>::iterator mi = dict.find( command );
			if ( mi == dict.end() ){
				fprintf( stderr, "%s:%i: Error: Unknown command \"%s\"\n", path, l, command.c_str() );
				throw std::runtime_error( std::string( "Unknown command" ) );
			}
			switch ( mi -> second )
			{
				case ce_heightmap :
					rHmap( path, l, is );
					break;
				case ce_segsize :
					rSsize( path, l, is );
					break;
				case ce_texture :
					rTex( path, l, is );
					break;
				case ce_segment :
					rSeg( path, l, is );
					break;
				case ce_deftex :
					rDeftex( path, l, is );
					bdeftex = 1;
					break;
			}
			while ( is.rdbuf()->in_avail() > 0 ){
				if ( !isspace( is.get() ) ){
					fprintf( stderr, "%s:%i: Warning: Ignored characters at end of line\n", path, l );
					break;
				}
			}
		}
		catch ( std::istream::failure& ex )
		{
			fprintf( stderr, "%s:%i: Error: I/O error: %s\n", path, l, ex.what() );
			throw std::runtime_error( std::string( "Error while reading data" ) );
		}
	}
	
	
	if ( hmap == 0 ){
		fprintf( stderr, "%s: Error: Heightmap not defined\n", path );
		throw std::runtime_error( "Heightmap not defined" );
	}
	if ( seg.size == 0 ){
		fprintf( stderr, "%s: Error: Segsize not defined\n", path );
		throw std::runtime_error( "Segsize not defined" );
	}
	
	if ( !texMap( deftex ) )
		texMap[ deftex ];
	
	LOG(( "MTT: Terrain loaded from MTT file \"%s\"\n", path ));
	
	makeSegments();
	
	}
	catch( std::exception &ex )
	{
		throw MKRE_LOAD_ERROR( "MTT", path, ex.what() );
	}
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

#if 0

void Terrain::draw( const CamInfo& camInfo, const bool correctCamPos )
{
	TRY
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glCullFace( GL_FRONT );
	glEnable( GL_CULL_FACE );
	
	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	
	glEnable( GL_FOG );
	glEnable( GL_LIGHTING );
	
	glActiveTexture( GL_TEXTURE0 );
	glEnable(GL_TEXTURE_2D);
	
	Segment::indices = indices;
	
	//!\todo Korekcja wysokości kamery
	
	glTexCoordPointer( 2, GL_FLOAT, 0, seg.texCoordArray );
	
	float maxdist = hypotf( (tanf( PI/3 )*aspect)*dist, hypot( tanf( PI/3 )*dist, dist ) );
	
	Point2f[5] p;
	p[0] = Point2f( -camInfo.x, -camInfo.y );
	p[1] = Point2f(  );
	p[2] = Point2f(  );
	p[3] = Point2f(  );
	p[4] = Point2f(  );
	
	//~ for ( int ix = 0; ix < seg.W; ix++ )
		//~ for ( int iy = 0; iy < seg.H; iy++ ){
			//~ seg( ix, iy ).tempDraw();
		//~ }
		
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	
	CATCH
}

#endif

void Terrain::tempDraw( const CamInfo& camInfo, const bool correctCamPos )
{
	//~ glEnableClientState(GL_VERTEX_ARRAY);
	//~ glEnableClientState(GL_NORMAL_ARRAY);
	//~ glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//~ glDisableClientState(GL_COLOR_ARRAY);
	
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

	//~ glTexCoordPointer( 2, GL_FLOAT, 0, seg.texCoordArray );
	for ( int ix = 0; ix < seg.W; ix++ )
		for ( int iy = 0; iy < seg.H; iy++ ){
			seg( ix, iy ).tempDraw(camInfo);
		}

    GLTERRORCHECK;
		
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
}

Segment::Segment() : displayLists(0), size( 0 ), W( 0 ), H( 0 )
{
}

uint16_t * Segment::indices = 0;
