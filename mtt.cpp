#include <mtt.hpp>
#include <bitmap.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include <except.hpp>

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

MTTTerrain::MTTTerrain( const char * path )
{
    loadFromMTT( path );
}

void MTTTerrain::rHmap( const char * path, int l, std::istream &is )
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

void MTTTerrain::rSsize( const char * path, int l, std::istream &is )
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

void MTTTerrain::rTex( const char * path, int l, std::istream &is )
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

void MTTTerrain::rSeg( const char * path, int l, std::istream &is )
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

void MTTTerrain::rDeftex( const char * path, int l, std::istream &is )
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

void MTTTerrain::checkMTTVersion( const char * path, std::istream& is, float min, float max )
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

void MTTTerrain::loadFromMTT( const char * path )
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
