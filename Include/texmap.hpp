#ifndef _TEXMAP_HPP_
#define _TEXMAP_HPP_

#include <map>
#include <exception>
#include <stdexcept>
#include <texture.hpp>

class TexMap
{
	private:
		std::map<unsigned int, Texture> map;
	public:
		TexMap();
		Texture& load( unsigned int id, const char * path, bool mipmaps );
		Texture& load( unsigned int id, int sourceid, int offsetx, int offsety, int w, int h );
		Texture& operator[]( unsigned int id ) { return map[ id ]; }
		bool loaded( unsigned int id ) { return map.find( id ) != map.end(); }
		bool operator ()( unsigned int id ) { return map.find( id ) != map.end(); } // == loaded(uint)
};

#endif //_TEXMAP_HPP_
