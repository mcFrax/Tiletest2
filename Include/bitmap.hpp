#ifndef _BITMAP_HPP_
#define _BITMAP_HPP_

#define PNG_NO_MNG_FEATURES

#include <png.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <memory>
#include <string>

#include <except.hpp>
#include "fakeistream.hpp"
#include "auto_array_ptr.hpp"

/*!
 * \file bitmap.hpp
 * \brief Implementacja klasy Bitmap
 */


//! Klasa przechowująca bitmapę
/*!
 * Umożliwia ładowanie, zapisywanie i dostęp do bitmap ( aktualnie zapisywanie jest niedostępne ).
 * Obsługuje pliki 2, 4, 8, 24 i 32 bitowe bmp i chyba wszystkie możliwe png.
 * Docelowo miała czytać pliki również z pamięci programu, ale to nie powinno być potrzebne i na razie nie wygląda na to, żeby coś się miało w tej kwestii ruszyć.
 */
class Bitmap
{
	private:
		class pngreader
		{
			private:
				unsigned char *img;
				const char *err_msg;
				unsigned long w, h, rbytes;
				int bpp, color_type;
				unsigned int abd;
				void cb_info(png_structp, png_infop);
				void cb_row(png_structp, png_bytep, png_uint_32, int);
				void cb_end(png_structp, png_infop);
				void cb_error(png_structp, png_const_charp);
				static void scb_info(png_structp, png_infop);
				static void scb_row(png_structp, png_bytep, png_uint_32, int);
				static void scb_end(png_structp, png_infop);
				static void scb_error(png_structp, png_const_charp);
			public:
				pngreader( unsigned char* data, unsigned int size, Bitmap* bmp, unsigned int accepted_bit_depth = 8 );
				~pngreader();
		};
		
		class pngwriter
		{
			private:
			public:
		};
		
		int w, h;
		int bpp, bpc, colors;
		unsigned char *buff;
		bool rev;
		
		void rescueCopyOf( Bitmap &pattern );
		void loadBMP( unsigned char *data, unsigned int size );
		void loadPNG( unsigned char *data, unsigned int size, unsigned int accepted_bit_depth = 8 );
		//~ void loadJPG( unsigned char *data, unsigned int size );
	public:
		Bitmap();					//!< Pusty kontruktor
		Bitmap( Bitmap &pattern );	//!< Konstruktor kopiujący
		Bitmap( const char *path, bool noException = 0, unsigned int accepted_bit_depth = 8 );	//!< Konstruktor ładujący z pliku
		Bitmap( unsigned char *data, unsigned int size, bool noException = 0, unsigned int accepted_bit_depth = 8 );	//!< Konstruktor ładujący z pamięci
			//!< \warning Jeszcze nie zaimplementowane!
		Bitmap( std::istream stream, bool noException = 0, unsigned int accepted_bit_depth = 8 );	//!< Konstruktor ładujący z pamięci
			//!< \warning Jeszcze nie zaimplementowane!
			//!< \warning Wymaga zastąpienia FakeIstream przez MemoryStream
		~Bitmap();					//!< Destruktor
		
		bool logging;				//!< Określa, czy mają być wypisywane logi. Domyślnie true.
		
		void free();				//!< Usuwa bitmapę z pamięci
		
		void loadFromFile( const char *path, bool noException = 0, unsigned int accepted_bit_depth = 8 );	//!< Ładuje z pliku
		void loadFromMemory( unsigned char *data, unsigned int size, bool noException = 0, unsigned int accepted_bit_depth = 8 );	//!< Ładuje z pamięci
			//!< \warning Jeszcze nie zaimplementowane!
		void loadFromStream( std::istream stream, bool noException = 0, unsigned int accepted_bit_depth = 8 );	//!< Ładuje ze strumienia
			//!< \warning Jeszcze nie zaimplementowane!
			//!< \warning Wymaga zastąpienia FakeIstream przez MemoryStream
		
		//! Pobranie szerokości
		int width() { return w; }					//!< \return szerokość tekstury w pikselach
		//! Pobranie wysokości
		int height() { return h; }					//!< \return wysokość tekstury w pikselach
		//! Bajty na piksel
		int bytesPerPixel() { return ( bpc > 8 ) ? colors * 2 : colors; }	//!< \return ilość bajtów zajmowanych przez piksel
		//! Bity na piksel
		int bitsPerPixel() { return bpp; }			//!< \return głębokość bitowa piksela
		//! Bity na kolor
		int bitsPerColor() { return bpc; }			//!< \return głębokość bitowa koloru
		//! Kolory na piksel
		int colorsPerPixel() { return colors; }		//!< \return ilość kanałow
		//! Bufor
		unsigned char * pixels() { return buff; }	//!< \return wskaźnik na bufor przechowujący piksele bitmapy
		//! Odwrócenie bajtów
		bool reversed() { return rev; }				//!< \return 1 przy BGR lub BGRA, 0 w innym przypadku
		
		//! Operator podstawienia
		Bitmap& operator = ( Bitmap &pattern );
};

#endif
