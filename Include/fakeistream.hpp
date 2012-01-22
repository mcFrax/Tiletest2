#ifndef _FAKEISTREAM_HPP_
#define _FAKEISTREAM_HPP_

#include <istream>
#include <cstring>
#include <except.hpp>

/*!
 * \file fakeistream.hpp
 * \brief Implementacja klasy FakeIstream
 * 
 * Docelowo ma zostać zstąpiony przez memorystream.hpp
 */

//! Strumień pamięci
/*!
 * Umożliwia czytanie z bufora jako ze strumienia.
 * Docelowo ma zostać zastąpiony przez MemoryStream.
 * Aktualna implementacja nie dziedziczy z std::istream, bo implementuje zbyt mało metod.
 * Być może zamiast implementowania MemoryStream użyję jakoiegoś gotowca, ale w STLu z tego co wiem nic takiego nie ma.
 */
class FakeIstream
{
	private:
		unsigned char *_data;
		unsigned char *_end;
		unsigned char *_curpos;
		bool eof;
	public:
		FakeIstream();
		FakeIstream( unsigned char *data, unsigned int size );
		void seekg( int off, std::ios::seekdir dir );
		void seekg( unsigned int pos );
		unsigned int tellg();
		int get();
		void read( unsigned char *s, unsigned int n );
		bool isEof();
};

#endif //_FAKEISTREAM_HPP_
