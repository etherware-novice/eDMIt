#include "dmi.h"



void moveOffsetToOffset( MagickWand *mw, unsigned dst, unsigned src, bool expand )
{
	unsigned x, y;
	calculateOffsetPos( src, &x, &y );

	MagickWand *rect = MagickGetImageRegion( mw, width, height, x, y );
	if( !rect ) return;

	if( !calculateOffsetPos( dst, &x, &y ) )
	{
		if( !expand ) return;

		if( x < pngwidth )
		{
			addSize( mw, 0, height );
			pngheight += height;
			printf( "%u x %u out of range, new height %u\n", x, y, pngheight );
		}
		else
		{
			addSize( mw, width, 0 );
			pngwidth += width;
			printf( "%u x %u out of range, new width %u\n", x, y, pngwidth );
		}
	}
	MagickCompositeImage( mw, rect, CopyCompositeOp, MagickFalse, x, y );
}

// TODO fix later, this is....so bordke
void makeOffsetSpace( unsigned offset, int spaces )
{
	char *buf = GETFSUF( FWORK );
	MagickWand *mw = eLoadImg( buf );

	MagickWand *erase = NewMagickWand();
	PixelWand *pw = NewPixelWand();
	PixelSetColor( pw, "none" );

	MagickNewImage( erase, width, height, pw );
	DestroyPixelWand( pw );

	unsigned i, j, x, y;
	if( spaces < 0 )
	{
		i = offset - spaces;
		i--;
		while( calculateOffsetPos( i++, NULL, NULL ) )
			moveOffsetToOffset( mw, i + spaces, i, false );			

		displayAndConf( mw );
	}
	else
	{
		for( i = MAXSTATES - 1; i > 0; i-- )
			if( statetable[i].offset )
			{
				j = statetable[i].offset + statetable[i].size;
				break;
			}
		if( !(i=j) ) return;

		while( i > offset )
		{
			moveOffsetToOffset( mw, i + spaces, i, true );
			i--;
		}
		
		for( j = 0; j < (unsigned) spaces; j++ )
		{
			calculateOffsetPos( i + j + 1, &x, &y );
			MagickCompositeImage( mw, erase, CopyCompositeOp, MagickFalse, x, y );
		}
	}

	MagickWriteImages( mw, buf, MagickTrue );
	DestroyMagickWand( mw );
	DestroyMagickWand( erase );
	free(buf);
}

void swapEditState( iconstate data, int dir )
{
	MagickWand *state = constructStateWand( data, dir );
	MagickResetIterator( state );
	MagickWand *strip = MagickAppendImages( state, MagickFalse );
	MagickWand *rect = NULL;
	DestroyMagickWand( state );


	char *tmp = GETFSUF( FTMP );
	char *work = GETFSUF( FWORK );
	MagickWriteImages( strip, tmp, MagickTrue );
	DestroyMagickWand( strip );

	getchar();

	strip = eLoadImg( tmp );
	state = eLoadImg( work );


	unsigned start = data.offset;
	unsigned ilace = 1;
	if( dir < 0 ) ilace = data.dirs;
	else start = data.offset += dir * data.frames;

	unsigned short i, j, internalOffset;
	unsigned x, y;
	for( i = 0; i < ilace; i++ )
		for( j = 0; j < data.frames; j++ )
		{
			internalOffset = start + i + (j*data.dirs);
			rect = MagickGetImageRegion( strip, width, height, j * width, 0 );
			calculateOffsetPos( internalOffset, &x, &y );

			MagickCompositeImage( state, rect, CopyCompositeOp, MagickFalse, x, y );

			DestroyMagickWand(rect);
			rect = NULL;
		}

	MagickWriteImages( state, work, MagickTrue );

	efremove( tmp, "cleaning up temp file" );
	DestroyMagickWand( state );
	DestroyMagickWand( strip );
	free( tmp );
	free( work );
}

// TODO implement all-directions
MagickWand *constructStateWand( iconstate data, int dir )
{
	MagickWand *src = imgopenTemp( FWORK );

	MagickWand *constructed = NewMagickWand();
	MagickWand *frameVis = NULL;

	unsigned start = data.offset;
	unsigned frames = 1;
	if( dir >= 0 ) start += dir * data.frames;
	else frames = data.dirs;

	unsigned x, y;
	unsigned short i, j;

	for( i = 0; i < data.frames; i++ )
	{
		for( j = 0; j < frames; j++ )
		{
			calculateOffsetPos( start + j + (i*data.dirs), &x, &y );
			frameVis = appendImg( frameVis, src, x, y, width, height );
		}
		MagickResetIterator( frameVis );
		MagickAddImage( constructed, MagickAppendImages( frameVis, MagickFalse ));

		MagickSetImageDispose( constructed, BackgroundDispose );
		MagickSetImagePage( constructed, width, height, 0, 0 );

		DestroyMagickWand( frameVis );
		frameVis = NULL;
	}

	DestroyMagickWand(src);
	return constructed;
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

	size_t read, write = 1;
	char buf[8192];

	// copies header
	long startpos;
	
	for( startpos = 0; startpos < 0x21; startpos++ ) if( (*buf = getc(source)) != EOF ) putc(*buf, target);

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
 	do
	{
		read = fread(buf, 1, sizeof(buf), source);
		if( read ) write = fwrite(buf, 1, sizeof(buf), target);
		else read = 0;
	} while( read > 0 && read == write );

	printf("wrote %ld bytes\n", ftell(target));
	fclose(target);
	fclose(source);
}

void writeStateWork(void)
{
	char *buf = GETFSUF( FWORK );
	writeStateTable( buf, fsource );
	free( buf );
}
