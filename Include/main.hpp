#ifndef _MAIN_HPP_
#define _MAIN_HPP_

#include <GL/gl.h>

#include <exception>
#include <stdexcept>
#include <cmath>
#include <cctype>

#include "const.hpp"
#include "utils.hpp"
#include "rendering.hpp"
#include "terrain.hpp"
#include <widget.hpp>
#include <engine.hpp>

class Scene : public Widget
{
	private:
		Terrain terrain;
		CamInfo cam;
		void onDraw();
		void onMotion( const MouseMotionEvent& event );
	public:
		Scene( const std::string& name, Widget* new_parent, long left, long top, long width, long height, const char* mapPath, const CamInfo& camI );
};

#endif
