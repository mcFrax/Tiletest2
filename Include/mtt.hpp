#ifndef _MTT_HPP_
#define _MTT_HPP_

#include <terrain.hpp>

class MTTTerrain : public Terrain
{
	protected:
		void rHmap( const char * path, int l, std::istream& is );
		void rSsize( const char * path, int l, std::istream& is );
		void rTex( const char * path, int l, std::istream& is );
		void rSeg( const char * path, int l, std::istream& is );
		void rDeftex( const char * path, int l, std::istream& is );
		void checkMTTVersion( const char * path, std::istream& is, float min, float max );
	public:
		MTTTerrain() {}
		MTTTerrain( const char * path );
		void loadFromMTT( const char * path );
};

#endif //_MTT_HPP_
