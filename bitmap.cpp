#include "bitmap.hpp"


/*!
 * Zeruje pola.
 */
Bitmap::Bitmap()
{
	w = h = colors = bpp = 0;
	buff = 0;
	logging = 1;
}

/*!
 * Tworzy obiekt Bitmap zawierający kopię bitmapy.
 * \param pattern bitmapa do skopiowania
 */
Bitmap::Bitmap( Bitmap &pattern )
{
	*this = pattern;
}

/*!
 * Tworzy bitmapę z pliku.
 * \param path ścieżka pliku
 * \param noException jeżeli true, w przypadku niepowodzenia nie rzuca wyjątku, tylko wypisuje zawartość na stderr.
 * \param accepted_bit_depth akceptowana ilość bitów na kolor. Suma bitowa wszystkich akceptowanych głębi bitowych, domyślnie 8.
 */
Bitmap::Bitmap( const char *path, bool noException, unsigned int accepted_bit_depth )
{
	logging = 1;
	w = h = colors = bpp = 0;
	buff = 0;
	loadFromFile( path, noException, accepted_bit_depth );
}

/*!
 * Zwalnia bufor bitmapy.
 */
Bitmap::~Bitmap()
{
	if ( buff ) delete [] buff;
}

/*!
 * Zwalnia bufor bitmapy.
 */
void Bitmap::free()
{
	if ( buff ) delete [] buff;
	w = h = colors = bpp = 0;
	buff = 0;
}

/*!
 * Operator podstawienia
 * Kopiuje bitmapę do nowego bufora.
 * \return referencja do *this
 */
Bitmap& Bitmap::operator = ( Bitmap &pattern )
{
	w = pattern.w;
	h = pattern.h;
	colors = pattern.colors;
	bpp = pattern.bpp;
	buff = new unsigned char [ w*h*bytesPerPixel() ];
	memcpy( buff, pattern.buff, w*h*bytesPerPixel() );
	return *this;
}

void Bitmap::rescueCopyOf( Bitmap &pattern )
{
	memcpy( this, &pattern, sizeof( Bitmap ) );
	pattern.buff = 0;
}

void Bitmap::loadBMP( unsigned char *data, unsigned int size )
{
	//~ BI_RGB, BI_RLE8, and BI_RLE4 have the values 0, 1, and 2
	Bitmap backup;
	backup.rescueCopyOf( *this );
	try
	{
		int off, headerLength, paletteColors = 0, compression = 0;
		auto_array_ptr<unsigned char> palette;
		
		free();
		
		FakeIstream fin( data, size );
		
		if ( !( fin.get() == 'B' && fin.get() == 'M' ) )
			throw std::runtime_error( "Not BMP file - magic number is wrong" );
		
		fin.seekg( 10 );
		fin.read( reinterpret_cast<unsigned char*>( &off ), 4 );
		fin.read( reinterpret_cast<unsigned char*>( &headerLength ), 4 );
		switch ( headerLength )
		{
			case 12 :
				fin.read( reinterpret_cast<unsigned char*>( &w ), 2 );
				fin.read( reinterpret_cast<unsigned char*>( &h ), 2 );
				fin.seekg( 2, std::ios::cur );
				fin.read( reinterpret_cast<unsigned char*>( &bpp ), 2 );
				paletteColors = 256;
				break;
			case 40 :
				fin.read( reinterpret_cast<unsigned char*>( &w ), 4 );
				fin.read( reinterpret_cast<unsigned char*>( &h ), 4 );
				fin.seekg( 2, std::ios::cur );
				fin.read( reinterpret_cast<unsigned char*>( &bpp ), 2 );
				fin.read( reinterpret_cast<unsigned char*>( &compression ), 4 );
				//~ CLOG << "compress == " << compression << '\n';
				if ( compression != 0 ){
					throw std::runtime_error( "Compressed BMP files are not supported" );
					//~ if ( compression != 1 )
						//~ throw std::runtime_error( "This compression type is not supported" );
					//~ else
						//~ if ( bpp != 8 )
							//~ throw std::runtime_error( "Compression type not suitable for bit depth" );
				}
				fin.seekg( 46 );
				if ( bpp <= 8 ){
					fin.read( reinterpret_cast<unsigned char*>( &paletteColors ), 4 );
					fin.seekg( 54 );
				}
				break;
			default :
				throw std::runtime_error( "Bad data type : unknown DIB header legth" );
		}
		
		if ( bpp != 2 && bpp != 4 && bpp != 8 && bpp != 24 && bpp != 32 )
			switch ( bpp )
			{
				case 1 :
					throw std::runtime_error( "Bad data type : reading 1 bit bimaps is not supported" );
				case 16 :
					throw std::runtime_error( "Bad data type : reading HighColor( 16bpp ) bimaps is not supported" );
				default :
					throw std::runtime_error( "Bad data type : invalid color depth" );
			}
		
		bpc = 8;
		colors = bpp/8;
		int Bpp = colors;
		
		auto_array_ptr<unsigned char> tempbuff = new unsigned char[w*h*(( paletteColors == 0 ) ? Bpp : 3)];
		
		int lineend = ( w*Bpp % 4 != 0 ) ? ( 4-(w*Bpp % 4) ) : 0;
		
		if ( paletteColors != 0 ){
			colors = 3;
			
			palette = new unsigned char [paletteColors * 4];
			
			fin.read( palette, paletteColors * 4 );
			fin.seekg( off );
			
			int Bpl = w / ( 8 / bpp );// Bytes per Line
			while ( Bpl % 4 != 0 )
				Bpl++;
			
			auto_array_ptr<unsigned char> rowBuffer = new unsigned char[ w ];
			
			switch ( bpp )
			{
				case 2 :
					for ( int iy = h-1; iy >= 0; iy-- ){
						unsigned char *row = &tempbuff[w*iy*3];
						fin.read( row, Bpl );
						fin.seekg( lineend, std::ios::cur );
						for ( int i = 0; i < w; i++ )
							rowBuffer[ i ] = row[ i/4 ] >> (( i % 4 )*2) & 3;
						
						for ( int ix = 0; ix < w; ix++ )
							memcpy( &row[ ix*3 ], &palette[ rowBuffer[ ix ]*4 ], 3 );
					}
					break;
				case 4 :
					for ( int iy = h-1; iy >= 0; iy-- ){
						unsigned char *row = &tempbuff[w*iy*3];
						fin.read( row, Bpl );
						fin.seekg( lineend, std::ios::cur );
						for ( int i = 0; i < w; i++ )
							rowBuffer[ i ] = ( row[ i/2 ] >> (( i % 2 )*4) ) & 15;
						
						for ( int ix = 0; ix < w; ix++ )
							memcpy( &row[ ix*3 ], &palette[ rowBuffer[ ix ]*4 ], 3 );
					}
					break;
				case 8 :
					for ( int iy = h-1; iy >= 0; iy-- ){
						unsigned char *row = &tempbuff[w*iy*3];
						fin.read( rowBuffer, w );
						fin.seekg( lineend, std::ios::cur );
						for ( int ix = 0; ix < w; ix++ )
							memcpy( &row[ ix*3 ], &palette[ rowBuffer[ ix ]*4 ], 3 );
					}
					break;
			}
				
			if ( logging )
				LOG(( "Loaded BMP bitmap :\n	%i x %i pixels, %i bpp, paletted RGB\n", w, h, bpp ));
			
			bpp = 24;
		} else {
			fin.seekg( off );
			
			for ( int i = h-1; i >= 0; i-- ){
				fin.read( &tempbuff[w*Bpp*i], w*Bpp );
				fin.seekg( lineend, std::ios::cur );
			}
			
			if ( logging )
				LOG(( "Loaded BMP bitmap :\n	%i x %i pixels, %i bpp, %s\n", w, h, bpp, colors > 2 ? colors == 4 ? "RGBA" : "RGB" : "Grayscale" ));
				
		}
			
		buff = tempbuff;
		rev = 1;
		tempbuff.release();
		
	}
	catch( std::exception &ex )
	{
		rescueCopyOf( backup );
		STDRTHROW;
	}
}

void Bitmap::loadPNG( unsigned char *data, unsigned int size, unsigned int accepted_bit_depth )
{
	Bitmap backup;
	backup.rescueCopyOf( *this );
	try
	{
		free();
		
		//! \todo exceptions przy ładowaniu png
		pngreader pngr( data, size, this, accepted_bit_depth );
		
		rev = 0;
		
		if ( logging )
			LOG(( "Loaded PNG bitmap :\n	%i x %i pixels, %i bpp, %s\n", w, h, bpp, colors > 2 ? colors == 4 ? "RGBA" : "RGB" : "Grayscale" ));
	}
	catch( std::exception &ex )
	{
		rescueCopyOf( backup );
		STDRTHROW;
	}
}

/*!
 * Ładuje bitmapę z pliku, przypadku niepowodzenia przywraca poprzednią zawartość.
 * \param path ścieżka pliku
 * \param noException jeżeli true, w przypadku niepowodzenia nie rzuca wyjątku, tylko wypisuje zawartość na stderr.
 * \param accepted_bit_depth akceptowana głębia bitowa koloru. Suma bitowa wszystkich akceptowanych głębi bitowych, domyślnie 8. Pliki BMP są ładowane zawsze z głębią 8 bitów na kolor.
 * \warning Rozpoznaje typ pliku tylko po rozszerzeniu.
 */
void Bitmap::loadFromFile( const char *path, bool noException, unsigned int accepted_bit_depth )
{
	unsigned int size;
	
	try
	{
		if ( strlen( path ) < 5 )
			throw std::runtime_error( "Too short filename" );
		
		char ext[5] = { 0 };
		
		strcpy( ext, path + strlen( path ) - 4 );
		
		for ( int i = 0; i < 4; i++ )
			if ( ext[ i ] >= 'a' )
				ext[ i ] -= 'a' - 'A';
				
		const char extstr[][5] = { ".BMP", ".PNG" };
		int extcount = 2;
		int filetype = -1;
		
		for ( int i = 0; i < extcount; i++ )
			if ( strcmp( ext, extstr[i] ) == 0 ){
				filetype = i;
				break;
			}
		if ( filetype == -1 )
			throw std::runtime_error( "Unknown file extension" );
			
		std::ifstream fin(path, std::ios::in | std::ios::binary);
		if( !fin ) throw std::runtime_error( "Could not open file" );
		
		
		fin.seekg( 0, std::ios::end );
		size = fin.tellg();
		fin.seekg( 0, std::ios::beg );
		
		auto_array_ptr<unsigned char> buffer = new unsigned char [ size ];
		
		fin.read( reinterpret_cast<char *>( static_cast<unsigned char *>( buffer ) ), size );
		
		if ( logging )
			LOG(( "Loadind bitmap from \"%s\"...\n", path ));
		
		switch ( filetype )
		{
			case 0 :
				loadBMP( buffer, size );
				break;
			case 1 :
				loadPNG( buffer, size, accepted_bit_depth );
				break;
		}
	}
	catch( std::exception &ex )
	{
		if ( noException ){
			fprintf( stderr, "Error while loading bitmap from \"%s\" : %s\n", path, ex.what() );
		} else {
			throw MKRE_LOAD_ERROR( "bitmap", path, ex.what() );
		}
	}
}



Bitmap::pngreader::pngreader( unsigned char* data, unsigned int size, Bitmap* bmp, unsigned int accepted_bit_depth )
{
	abd = accepted_bit_depth | 8;
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, scb_error, NULL);
	png_infop png_info = png_create_info_struct(png_ptr);
	png_set_progressive_read_fn(png_ptr, this, scb_info, scb_row, scb_end);
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &png_info, NULL);
		throw std::runtime_error( err_msg );
	}
	png_process_data(png_ptr, png_info, reinterpret_cast<unsigned char*>(data), size);
	if ( bpp != 8 ){
		throw std::runtime_error( "This bit depth cannot be handled" );
	}
	switch ( color_type )
	{
		case PNG_COLOR_TYPE_GRAY :
			bmp -> colors = 1;
			break;
		case PNG_COLOR_TYPE_RGB :
			bmp -> colors = 3;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA :
			bmp -> colors = 4;
			break;
		default :
			throw std::runtime_error( "This color type cannot be handled" );
	}
	bmp -> bpp = bmp -> colors * bpp;
	bmp -> bpc = bpp;
	
	bmp -> w = w;
	bmp -> h = h;
	bmp -> buff = img;
	img = 0;
	
	png_destroy_read_struct(&png_ptr, &png_info, NULL);
}

Bitmap::pngreader::~pngreader()
{
}

// ================== funkcje zwrotne z pnglib (callbacki) ==================
void Bitmap::pngreader::cb_info(png_structp png_ptr, png_infop png_info)
{
	png_get_IHDR(png_ptr, png_info, &w, &h, &bpp, &color_type, NULL, NULL, NULL);

	if ( ( bpp & abd ) == 0 ){
		if ( bpp < 8 ){
			png_set_packing( png_ptr );
			bpp = 8;
		} else {
			png_set_strip_16( png_ptr );
			bpp = 8;
		}
	}
	
	if (color_type == PNG_COLOR_TYPE_PALETTE){
		png_set_palette_to_rgb(png_ptr);
		color_type = PNG_COLOR_TYPE_RGB;
	}
	
	if (png_get_valid(png_ptr, png_info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	png_read_update_info(png_ptr, png_info);
	rbytes = png_get_rowbytes(png_ptr, png_info);
	img = new unsigned char[h * rbytes];
}
void Bitmap::pngreader::cb_row(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass)
{
	png_progressive_combine_row(png_ptr, &img[rbytes * row_num], new_row);
}
void Bitmap::pngreader::cb_end(png_structp png_ptr, png_infop png_info)
{
}
void Bitmap::pngreader::cb_error(png_structp png_ptr, png_const_charp msg)
{
	err_msg = msg;
	longjmp(png_ptr->jmpbuf, 1);
}

// ================== statyczne wrappery do callback�w ==================
void Bitmap::pngreader::scb_info(png_structp png_ptr, png_infop png_info)
{
	static_cast<Bitmap::pngreader*>(png_get_progressive_ptr(png_ptr))->cb_info(png_ptr, png_info);
}
void Bitmap::pngreader::scb_row(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass)
{
	static_cast<Bitmap::pngreader*>(png_get_progressive_ptr(png_ptr))->cb_row(png_ptr, new_row, row_num, pass);
}
void Bitmap::pngreader::scb_end(png_structp png_ptr, png_infop png_info)
{
	static_cast<Bitmap::pngreader*>(png_get_progressive_ptr(png_ptr))->cb_end(png_ptr, png_info);
}
void Bitmap::pngreader::scb_error(png_structp png_ptr, png_const_charp msg)
{
	static_cast<Bitmap::pngreader*>(png_get_progressive_ptr(png_ptr))->cb_error(png_ptr, msg);
}
