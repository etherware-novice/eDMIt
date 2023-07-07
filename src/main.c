#include "dmi.h"
#include <zlib.h>
#include <endian.h>

#include <string.h> // tmp


int main(int argc, char *argv[])
{
	if( argc < 2 )
	{
		fputs("Please pass a dmi file to parse!\n", stderr);
		return ENOENT;
	}

	loadStateTable(argv[1]);
	writeStateTable( "checker.png", "/tmp/checker.dmi" );

	MagickWand *foo = eLoadImg( "checker.png" );
	MagickWand *rfire = eLoadImg( "rfirebad.png" );

	/*
	appendImgInPlace( &rfire, rfire, 0, 0, 64, 64 );
	appendImgInPlace( &rfire, foo, 8, 8, 128, 128 );

	MagickCompositeImage( rfire, foo, OverCompositeOp, MagickFalse, 5, 50 );
	*/

	unsigned x, y;
	calculateOffsetPos( 37, &x, &y );

	foo = eLoadImg(argv[1]);
	rfire = appendImg( NULL, foo, x, y, width, height );
	addSize( rfire, 32, 16 );

	printf("%s (%u x %u)\n\n", fsource, pngwidth, pngheight);

	unsigned i, j;
	iconstate *cur;
	for( i = 0; i < MAXSTATES; i++ )
	{
		cur = statetable + i;
		if( cur->name[0] == '\0' ) continue;
		printf("(%u/%u) [ ", cur->frames, cur->dirs);

		for( j = 0; j < cur->frames; j++ )
			printf("%u ", cur->delay[j]);

		printf("] <%u - %hu> %s\n", cur->offset, cur->size, cur->name);
	}

	displayAndConf( rfire );
}
