#ifndef _TERRAIN_HPP_
#define _TERRAIN_HPP_

#include "const.hpp"
#include "texmap.hpp"
#include "utils.hpp"
#include "rendering.hpp"
#include "auto_array_ptr.hpp"
#include "geometry.hpp"

#include <terrain_segment.hpp>

class Terrain
{
	protected:
		static const char CommentChar = '#';
		static const float tileSize = 1.0f;
		struct SegmentationInfo
		{
			unsigned int size;
			int W, H;
			auto_array_ptr<Point2f> texCoordArray;
			auto_array_ptr<TerrainSegment> segments;
			std::map<std::pair<int, int>, Uint32> map;
			SegmentationInfo() : size( 0 ), W( 0 ), H( 0 ) {}
			TerrainSegment& operator()( uint32_t x, uint32_t y ) { return segments[ y*W+x ]; }
		} seg;
		int W, H;
		unsigned int deftex;
		auto_array_ptr<float> hmap;
		auto_array_ptr<uint16_t> indices;
		TexMap texMap;
		float hmapGet( int x, int y );
		void make( TerrainSegment & s, int x, int y );
		void makeSegments();
		void loadHmap( const char * path );
	public:
		Terrain();
		void draw( const CamInfo& camInfo, const bool correctCamPos = 1 );
		void tempDraw( const CamInfo& camInfo, const bool correctCamPos = 1 );
};

#endif //_TERRAIN_HPP_
