#include "dmi.h"
#include <string.h>


bool calculateOffsetPos( unsigned offset, unsigned *x, unsigned *y )
{
	unsigned perLine = pngwidth / width;
	unsigned nX, nY;

	// if its less than the min number in a line, and theres less than 2 rows already
	if( perLine < MINCOLDEFAULT && pngheight < height * 2 )
	{
		if( x ) *x = offset * width;
		if( y ) *y = 0;
		return false;
	}

	nX = (offset % perLine) * width;
	nY = (offset / perLine) * height;

	if( x ) *x = nX;
	if( y ) *y = nY;

	// getting bottom right of the icon
	nX += width;
	nY += height;

	return ( nX <= pngwidth && nY <= pngheight );
}

static char *tableToString(void)
{
	unsigned i, j;
	iconstate *curEnt = NULL;

	char *export = sncatf( NULL, "# BEGIN DMI\nversion = 4.0\n" );
	export = sncatf( export, "\twidth = %u\n\theight = %u", width, height );

	for( i = 0; i < MAXSTATES; i++ )
	{
		curEnt = statetable + i;
		if( curEnt->name[0] == '\0' ) continue;

		export = sncatf( export, "\nstate = \"%s\"\n\tdirs = %u\n\tframes = %u",
				curEnt->name, curEnt->dirs, curEnt->frames );

		if( curEnt->frames > 1 )
		{
			export = sncatf( export, "\n\tdelay = " );
			for( j = 0; j < curEnt->frames; j++ )
			{
				if( j ) export = sncatf( export, "," );
				export = sncatf( export, "%u", curEnt->delay[j] );
			}
		}

		for( j = 0; j < MAXAUX; j++ )
			if( curEnt->aux[j] ) 
				export = sncatf( export, "\n%s", curEnt->aux[j] );
	}

	return sncatf( export, "\n# END DMI\n" );
}


// TODO actually read from base
void writeStateTable( const char *base, const char *path )
{
	FILE *source = efopen( base, "rb", "opening base file for copying from" );
	FILE *target = efopen( path, "wb+", "opening export file" );
	if( !target || !source ) return;

	// copies header
	long startpos;
	int c;
	for( startpos = 0; startpos < 0x21; startpos++ ) if( (c = getc(source)) != EOF ) putc(c, target);

	fprintf( target, "0000zTXt%s", header );
	fputc( '\0', target );
	fputc( '\0', target );

	char *tablerep = tableToString();
	unsigned long len = fwritecomp( target, tablerep, strlen(tablerep) );
	len += strlen( header ) + 2;	// adds in vis name + the two null bytes

	fseek( target, startpos, SEEK_SET );
	fwriteu32( target, len, true );
	fwritecrc32( target, len + 4, true );
	free(tablerep);

	// copy the rest of the file
        while( (c = getc(source)) != EOF ) putc(c, target);
	printf("wrote %ld bytes\n", ftell(target));
	fclose(target);
	fclose(source);
}
