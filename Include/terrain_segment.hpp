#ifndef _TERRAIN_SEGMENT_HPP_
#define _TERRAIN_SEGMENT_HPP_

#include "geometry.hpp"
#include "texture.hpp"
#include "rendering.hpp"

struct TerrainSegment
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
		TerrainSegment();
		void set( int s, int w, int h ) { size = s; size2 =__builtin_ctz(size); W = w; H = h; }
		void draw( const CamInfo& camInfo );
		void tempDraw( const CamInfo& camInfo );
        void createDisplayLists( Point2f* texCoordArray );
        int lod( const CamInfo& camInfo ) { return 0; }

};

#endif // _TERRAIN_SEGMENT_HPP_
