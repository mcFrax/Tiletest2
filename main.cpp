#include <main.hpp>
#include <widgets>
#include <list>
#include <sstream>
#include <cmath>

static const long WINDOW_WIDTH = 800;
static const long WINDOW_HEIGHT = 600;

static const char* WINDOW_TITLE = "Tiletest 2.0";
static const char* WINDOW_TITLE_SHORT = WINDOW_TITLE;
static const char* WINDOW_ICON = "icon.png";

const static float mousefactor = 0.08f;

Scene::Scene( const std::string& name, Widget* new_parent, long left, long top, long width, long height, const char* mapPath, const CamInfo& camI )
	:	Widget( name, new_parent, left, top, width, height ), terrain( mapPath ), cam( camI )
{
}

void Scene::onMotion( const MouseMotionEvent& e )
{
	Engine::Redraw();
	register bool ppm = Mouse::button( SDL_BUTTON_RIGHT ), lpm = Mouse::button( SDL_BUTTON_LEFT );
	if ( ppm || lpm ){
		if ( ppm ){
			if ( lpm ){
				cam.z += e.yrel * mousefactor;
			} else {
				cam.xrot += e.yrel / 10.0f;
				cam.zrot += e.xrel / 10.0f;
				if ( cam.xrot > 360.0f ) cam.xrot = 360.0f;
				if ( cam.xrot < 180.0f ) cam.xrot = 180.0f;
				if ( cam.zrot > 360.0f ) cam.zrot -= 360.0f;
				if ( cam.zrot < 0.0f ) cam.zrot += 360.0f;
			}
		} else {
			cam.x -= mousefactor * ( e.yrel * sin( -3.14f*cam.zrot/180.0f ) + e.xrel * cos( -3.14f*cam.zrot/180.0f ) );
			cam.y -= mousefactor * ( e.xrel * sin( -3.14f*cam.zrot/180.0f ) + e.yrel * -cos( 3.14f*cam.zrot/180.0f ) );
		}
	}
}

void Scene::onDraw()
{
	Engine::Redraw();
	//~ static const float bg[4] = { 70.0f/255.0f, 180.0f/255.0f, 200.0f/255.0f, 1.0f };
	//~ 
    //~ glClearColor( bg[0], bg[1], bg[2], bg[3] );
	glClear( GL_DEPTH_BUFFER_BIT);
	
	cam.aspect = float( Engine::window().width() ) / Engine::window().height();
	
	cam.set();
	
	glEnable( GL_LIGHTING );
	glEnable( GL_DEPTH_TEST );
    //~ glDisable(GL_COLOR_MATERIAL);
    //~ glEnable(GL_COLOR_MATERIAL);
    glColor3f( 0.5f, 0.5f, 0.5f );
	
	setLightPos();
	
	terrain.tempDraw();
	
	//~ glFinish();
	
	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_TEXTURE_2D );
	glClear( GL_DEPTH_BUFFER_BIT);
}

bool fullscreen = 0;
void cbk( Widget* sender, const SDL_keysym& key )
{
	switch ( key.sym ){
		case SDLK_ESCAPE :
			Engine::Quit();
			break;
		case SDLK_F11 :
			fullscreen = !fullscreen;
			Engine::setVideoModeFlags( fullscreen ? SDL_FULLSCREEN : 0 );
			break;
		default :
			break;
	}
}

void esc( Widget* sender, const MouseButtonEvent& mbe )
{
	Engine::Quit();
}

std::string fpsstring( Uint32 n )
{
	std::ostringstream os;
	os << n << " FPS";
	return os.str();
}

static Label * fpsLabel;
static std::list<Uint32> fps;
void cbi( Widget * sender )
{
	Engine::Redraw();
	Uint32 t = SDL_GetTicks();
	fps.push_back( t );
	t = (t<1000) ? 0 : t-1000;
	while ( fps.front() < t ) fps.pop_front();
	fpsLabel -> setCaption( fpsstring( fps.size() ) );
}

int main( int argc, char* argv[] )
{
	try
	{
		LOG(( "%i %i %i %i %i\n", SDL_BUTTON_LEFT, SDL_BUTTON_RIGHT, SDL_BUTTON_MIDDLE, SDL_BUTTON_WHEELUP, SDL_BUTTON_WHEELDOWN ));
		
		CamInfo cam ( 5.0f, 5.0f, -12.0f, 300.0f, 45.0f, DIST, static_cast<double>(WINDOW_WIDTH)/WINDOW_HEIGHT );
		Engine engine( argc, argv, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, WINDOW_TITLE_SHORT, WINDOW_ICON );
		
		glSetup();
		static const float fog[4] = { 70.0f/255.0f, 180.0f/255.0f, 200.0f/255.0f, 1.0f };
		for ( int i = 0; i < 3; ++i )
			Engine::backgroundColor[i] = fog[i];
		
		Frame* f = new Frame( "Frame1" );
		f -> setVisible( 1 );
		Scene * scn = new Scene( std::string("scn"), f, 0, 0, Engine::window().width(), Engine::window().height(), "testmap.mtt", cam );
		scn -> anchors = 0xF;
		
		fpsLabel = new Label( std::string( "FPS Label" ), f, Engine::window().width()-70, 0, 70, 20, Engine::defFont, std::string() );
		fpsLabel -> setEnabled( 0 );
		fpsLabel -> anchors = Widget::anTop | Widget::anRight;
		fpsLabel -> setHorTextPos( Label::hpRight );
		
		Engine::window().keyUpCallback = &cbk;
		Engine::window().idleCallback = &cbi;
		
		( new Button( "Exit", f, 0, 0, 100, 24 ) ) -> clickCallback = &esc;
		
		engine.MainLoop();
	}
	catch ( std::logic_error &le )
	{
		fprintf( stderr, "Logic error : \n%s :\n%s\n", __PRETTY_FUNCTION__, le.what() );
	}
	catch ( std::runtime_error &re )
	{
		fprintf( stderr, "Runtime error : \n%s :\n%s\n", __PRETTY_FUNCTION__, re.what() );
	}
	catch ( ... )
	{
		fprintf( stderr, "Unexpected error\n" );
		throw;
	}
}
