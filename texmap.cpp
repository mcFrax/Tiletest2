#include <texmap.hpp>

TexMap::TexMap()
{
}

Texture& TexMap::load( unsigned int id, const char * path, bool mipmaps )
{
	Texture& tex = map[ id ];
	tex.loadFromFile( path, mipmaps );
	return tex;
}

Texture& TexMap::load( unsigned int id, int sourceid, int offsetx, int offsety, int w, int h )
{
	Texture& tex = map[ id ];
	tex = Texture( &map[ sourceid ], offsetx, offsety, w, h );
	return tex;
}
