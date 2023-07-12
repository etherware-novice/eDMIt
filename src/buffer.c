#include "dmi.h"

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

void efremove( const char *path, const char *desc )
{
	if( !desc ) desc = "generic delete";
	if( !remove( path ) )
		fprintf( stderr, "deleting %s (%s): %s\n", path, desc, strerror(errno) );
}

void fcopyTemp( const char *dstsuffix, const char *srcsuffix )
{
	if( !dstsuffix || !srcsuffix ) return;

	char *dst = GETFSUF( dstsuffix );
	char *src = GETFSUF( srcsuffix );

	copyFile( dst, src );
	
	free( dst );
	free( src );
}

FILE *fopenTemp( const char *suffix, const char *mode )
{
	char *buf = GETFSUF( suffix );
	FILE *fstr = efopen( buf, mode, "opening tmp file swap" );

	free( buf );
	return fstr;
}

MagickWand *imgopenTemp( const char *suffix )
{
	char *buf = GETFSUF( suffix );
	MagickWand *mw = eLoadImg( buf );

	free(buf);
	return mw;
}

void fdelTemp( const char *suffix )
{
	char *buf = GETFSUF( suffix );
	efremove( buf, "deleting temp file" );
	free( buf );
}

void arrshiftfw( void *arr, unsigned start, size_t size, size_t nmemb )
{
	if( !arr ) return;
	if( start >= nmemb - 1 ) return;

	unsigned arrBytes = size * nmemb;
	char *cast = arr;
	start *= size;
	cast += start;

	memmove( cast + size, cast, arrBytes - (start + size) );
	memset( cast, 0, size );
}

void arrshiftbw( void *arr, unsigned start, size_t size, size_t nmemb )
{
	if( !arr ) return;
	if( start >= nmemb - 1 ) return;

	unsigned arrBytes = size * nmemb;
	char *cast = arr;
	start *= size;
	cast += start;

	memmove( cast, cast + size, arrBytes - ( start + size ) );
	memset( (cast - start) + (arrBytes - size), 0, size );
}

void fgetsln( char *s, int size, FILE *stream )
{
	unsigned short i;

	do
		fgets( s, size, stream );
	while
		(s[0] == '\n');

	for( i = 0; i < size; i++ )
	{
		if( s[i] != '\n' ) continue;
		s[i] = '\0';
		break;
	}

	while((size = fgetc(stream) != '\0')) 
		if( size == '\n' ) break;
}

char *strplace( const char *source, const char *find, const char *replace )
{
	unsigned i = 0;
	const size_t findSize = strlen(find);
	const size_t replaceSize = strlen(replace);
	char *buf = malloc(sizeof(char) * strlen(source) * replaceSize);

	while( *source != '\0' )
	{
		if(!strncmp( source, find, findSize )) 
		{
			memcpy( buf + i, replace, replaceSize );
			i += replaceSize;
			source += findSize;
		}
		else
		{
			buf[i++] = *source;
			source++;
		}
	}

	buf[i] = '\0';
	return realloc( buf, strlen(buf) + 1 );
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

void displayFile( const char *path )
{
	char *syscall = sncatf( NULL, "%s %s", IMGVIEW, path );
	system( syscall );

	free( syscall );
}
