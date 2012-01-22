#ifndef _TERRAIN_SEGMENT_HPP_
#define _TERRAIN_SEGMENT_HPP_

#include "geometry.hpp"
#include "texture.hpp"
#include "rendering.hpp"

struct TerrainSegment
{
    private:
        uint displayLists;
        float xpos, ypos;
        float bx, by, bz;
    public:
		Texture tex;
        //!< Tekstura segmentu
		int size;
        //!< Wielkość segmentu, tj. rozmiar tablic.
        int size2;
        //!< log_2(size)
		int W;
        //!< Szerokość segmentu, tj. szerokość faktycznej zawartości.
		int H;
        //!< Wysokość segmentu, tj. wysokość faktycznej zawartości.
		TerrainSegment();
        ~TerrainSegment();
		void set( int s, int w, int h );
		void draw( const CamInfo& camInfo );
		void tempDraw( const CamInfo& camInfo );
		void infoDraw( const CamInfo& camInfo );
        void createDisplayLists( Point3f* vertexArray, Point3f* normalArray, 
                Point2f* texCoordArray );
};

#endif // _TERRAIN_SEGMENT_HPP_
