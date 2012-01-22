#ifndef _TERRAIN_HPP_
#define _TERRAIN_HPP_

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <vector>
#include <map>

#include "const.hpp"
#include "texture.hpp"
#include "texmap.hpp"
#include "utils.hpp"
#include "rendering.hpp"
#include "auto_array_ptr.hpp"
#include "geometry.hpp"

#include <except.hpp>

struct Segment
{
		static uint16_t * indices;
		auto_array_ptr<Point3f> vertexArray;
		auto_array_ptr<Point3f> normalArray;
		Point2f * texCoordArray;
		Texture tex;	//!< Tekstura segmentu
		int size;		//!< Wielkość segmentu, tj. rozmiar tablic.
		int W;			//!< Szerokość segmentu, tj. szerokość faktycznej zawartości (==size, jeżeli pełny segment).
		int H;			//!< Wysokość segmentu, tj. wysokość faktycznej zawartości (==size, jeżeli pełny segment).
		Segment();
		void set( int s, int w, int h ) { size = s; W = w; H = h; }
		void draw( const CamInfo& camInfo );
		void tempDraw();
};

class Terrain
{
	private:
		static const char CommentChar = '#';
		static const float tileSize = 1.0f;
		struct SegmentationInfo
		{
			unsigned int size;
			int W, H;
			auto_array_ptr<Point2f> texCoordArray;
			auto_array_ptr<Segment> segments;
			std::map<std::pair<int, int>, Uint32> map;
			SegmentationInfo() : size( 0 ), W( 0 ), H( 0 ) {}
			Segment& operator()( uint32_t x, uint32_t y ) { return segments[ y*W+x ]; }
		} seg;
		int W, H;
		unsigned int deftex;
		auto_array_ptr<float> hmap;
		auto_array_ptr<uint16_t> indices;
		TexMap texMap;
		float hmapGet( int x, int y );
		void make( Segment & s, int x, int y );
		void makeSegments();
		void loadHmap( const char * path );
		void rHmap( const char * path, int l, std::istream& is );
		void rSsize( const char * path, int l, std::istream& is );
		void rTex( const char * path, int l, std::istream& is );
		void rSeg( const char * path, int l, std::istream& is );
		void rDeftex( const char * path, int l, std::istream& is );
		void checkMTTVersion( const char * path, std::istream& is, float min, float max );
	public:
		Terrain();
		Terrain( const char * path );
		void loadFromMTT( const char * path );
		void draw( const CamInfo& camInfo, const bool correctCamPos = 1 );
		void tempDraw();
};


inline void Segment::draw( const CamInfo& camInfo )
{
	tempDraw();
}

inline void Segment::tempDraw()
{
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	glTranslatef( -tex.x1(), -tex.y1(), 0.0f );
	glScalef( tex.x2()-tex.x1(), tex.y2()-tex.y1(), 1.0f );
	
	tex.safeBind();
	
	glVertexPointer( 3, GL_FLOAT, 0, vertexArray );
	glNormalPointer( GL_FLOAT, 0, normalArray );
	
	for ( int iy = 0; iy < H; iy++ ){
		for ( int ix = 0; ix <= W; ix++ ){
			indices[2*ix] = iy*(size+1)+ix;
			indices[2*ix+1] = (iy+1)*(size+1)+ix;
		}
		
		glDrawRangeElements( GL_TRIANGLE_STRIP, iy*(size+1), (iy+1)*(size+1)+W, 2*W+2, GL_UNSIGNED_SHORT, indices );
	}
}

#endif //_TERRAIN_HPP_
