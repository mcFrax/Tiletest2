#include "fakeistream.hpp"

#undef EXCEPTION_TREE


FakeIstream::FakeIstream()
{
	_end = _data = _curpos = 0;
	eof = 1;
}

FakeIstream::FakeIstream( unsigned char *data, unsigned int size ) : _data( data ), _curpos( data )
{
	_end = _data + size;
	eof = ( size == 0 );
}

void FakeIstream::seekg( int off, std::ios::seekdir dir )
{
	TRY
	if ( _data == 0 )
		throw std::logic_error( "Setting pos in not initialized stream" );
	
	switch ( dir )
	{
		case std::ios::beg : 
			if ( _data + off > _end ){
				eof = 1;
				throw std::runtime_error( "Position beyond end of stream" );
			}
			_curpos = _data + off;
			break;
		case std::ios::cur : 
			if ( _curpos + off > _end ){
				eof = 1;
				throw std::runtime_error( "Position beyond end of stream" );
			}
			_curpos += off;
			break;
		case std::ios::end : 
			if ( _end + off - 1 > _end ){
				eof = 1;
				throw std::runtime_error( "Position beyond end of stream" );
			}
			_curpos = _end + off - 1;
			break;
		default :
			throw std::runtime_error( "Hey, _S_ios_seekdir_end is not supported!" );
	}
	eof = ( _curpos == _end );
	CATCH
}

void FakeIstream::seekg( unsigned int pos )
{
	TRY
	if ( _data == 0 )
		throw std::logic_error( "Setting pos in not initialized stream" );
	if ( _data + pos > _end ){
		eof = 1;
		throw std::runtime_error( "Position beyond end of stream" );
	}
		
	_curpos = _data + pos;
	eof = ( _curpos == _end );
	CATCH
}

unsigned int FakeIstream::tellg()
{
	return _curpos - _data;
}

int FakeIstream::get()
{
	TRY
	if( _data == 0 )
		throw std::logic_error( "Reading from not initialized stream" );
	if ( _curpos >= _end )
		throw std::runtime_error( "Reading beyond end of stream" );
	
	_curpos++;
	eof = ( _curpos == _end );
	
	return static_cast<int>( *( _curpos-1 ) );
	CATCH
}

void FakeIstream::read( unsigned char *s, unsigned int n )
{
	TRY
	if( _curpos == 0 )
		throw std::logic_error( "Reading from not initialized stream" );
	if ( _curpos + n - 1 >= _end )
		throw std::runtime_error( "Reading beyond end of stream" );
		
	memcpy( s, _curpos, n );
	_curpos += n;
	eof = ( _curpos == _end );
	CATCH
}

bool FakeIstream::isEof()
{
	return eof;
}
