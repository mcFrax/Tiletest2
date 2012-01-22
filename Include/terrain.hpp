#ifndef _TERRAIN_HPP_
#define _TERRAIN_HPP_

#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include <cmath>

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
        uint displayLists;
		static uint16_t * indices;
		auto_array_ptr<Point3f> vertexArray;
		auto_array_ptr<Point3f> normalArray;
		Point2f * texCoordArray;
		Texture tex;	//!< Tekstura segmentu
		int size;		//!< Wielkość segmentu, tj. rozmiar tablic.
        int size2;      //!< log_2 size
		int W;			//!< Szerokość segmentu, tj. szerokość faktycznej zawartości (==size, jeżeli pełny segment).
		int H;			//!< Wysokość segmentu, tj. wysokość faktycznej zawartości (==size, jeżeli pełny segment).
		Segment();
		void set( int s, int w, int h ) { size = s; size2 =__builtin_ctz(size); W = w; H = h; }
		void draw( const CamInfo& camInfo );
		void tempDraw( const CamInfo& camInfo );
        void createDisplayLists( Point2f* texCoordArray );
        int lod( const CamInfo& camInfo ) { return size2; }

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
		void tempDraw( const CamInfo& camInfo, const bool correctCamPos = 1 );
};

#endif //_TERRAIN_HPP_
