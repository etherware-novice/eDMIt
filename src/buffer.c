#include "dmi.h"
#include <error.h>
#include <string.h>
#include <stdarg.h>

void *emalloc( size_t size, const char *desc )
{
	if( !desc ) desc = "generic malloc";
	void *block = malloc(size);

	if( !block ) perror(desc);
	return block;
}

void *erealloc( void *ptr, size_t size, const char *desc )
{
	if( !desc ) desc = "generic realloc";
	void *block = realloc(ptr, size);
	
	if (!block )
	{
		perror(desc);
		return ptr;
	}

	if( !ptr ) memset( block, 0, size );
	errno = 0;
	return block;
}

FILE *efopen( const char *path, const char *mode, const char *desc )
{
	if( !desc ) desc = "generic fopen";
	FILE *block = fopen(path, mode);

	if( !block ) perror(desc);
	return block;
}

FILE *fopenTemp( const char *suffix, const char *mode )
{
	char *buf = sncatf( NULL, "%s%s", fsource, suffix );
	FILE *fstr = efopen( buf, suffix, "opening tmp file swap" );

	free( buf );
	return fstr;
}

void fswapos( FILE *stream, long *pos )
{
	long buf = ftell(stream);
	fseek( stream, *pos, SEEK_SET );
	*pos = buf;
}

char *sncatf( char *str, const char *format, ... )
{
	unsigned size = 0;
	if(str) size = strlen(str) + 1;

	va_list args;
	va_start(args, format);
	va_list argsCp;
	va_copy(argsCp, args);

	int appendSize = vsnprintf(NULL, 0, format, argsCp);
	va_end(argsCp);
	if( appendSize++ < 1 ) return str;

	size += appendSize;
	str = erealloc( str, sizeof(char) * size, "expanding space for concatination" );
	if(errno) return str;

	vsnprintf(strchr(str, '\0'), appendSize, format, args);
	va_end(args);

	return str;
}
