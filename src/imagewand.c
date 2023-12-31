#include <dmi.h>



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

static void conGen(void)
{
	if( IsMagickWandInstantiated() == MagickTrue) return;
	puts("Initialized ImageMagick");
	MagickWandGenesis();
}

MagickWand *eLoadImg( const char *path )
{
	conGen();
	MagickWand *mw = NewMagickWand();
	MagickReadImage( mw, path );

	PixelWand *pw = NewPixelWand();
	PixelSetColor( pw, "none" );
	MagickSetImageBackgroundColor( mw, pw );

	DestroyPixelWand(pw);

	MagickSetImageTicksPerSecond( mw, BYONDTPS );
	return mw;
}

void touchImage( const char *path, const char *color, unsigned x, unsigned y )
{
	const char *tmp = "tmp.png";

	PixelWand *pw = NewPixelWand();
	PixelSetColor( pw, color );

	MagickWand *mw = NewMagickWand();
	MagickNewImage( mw, x, y, pw );

	MagickWriteImage( mw, tmp );
	copyFile( path, tmp );
	//remove(tmp);
}

void resizeWand( MagickWand *mw, unsigned xNew, unsigned yNew )
{
	MagickExtentImage( mw, xNew, yNew, 0, 0 );
}

void addSize( MagickWand *mw, int xMov, int yMov )
{
	size_t x, y;
	//MagickGetSize( mw, &x, &y );
	x = MagickGetImageWidth( mw );
	y = MagickGetImageHeight( mw );

	if( xMov < 0 && xMov * -1 > (long long) x ) x = 1;
	else x += xMov;

	if( yMov < 0 && yMov * -1 > (long long) y ) y = 1;
	else y += yMov;

	resizeWand( mw, x, y );
}

void multSize( MagickWand *mw, double xMov, double yMov )
{
	float calc;
	size_t x, y;
	MagickGetSize( mw, &x, &y );

	calc = x * xMov;
	if( calc < 0 ) x = 1;
	else x = calc;

	calc = y * yMov;
	if( calc < 0 ) y = 1;
	else y = calc;

	resizeWand( mw, x, y );
}

MagickWand *appendImg( MagickWand *dst, MagickWand *src, unsigned x, unsigned y, unsigned width, unsigned height )
{
	MagickWand *rect = MagickGetImage(src);
	MagickCropImage( rect, width, height, x, y );

	if( !dst ) return rect;
	MagickAddImage( dst, rect );
	MagickSetImageDispose( dst, BackgroundDispose );
	MagickSetImagePage( dst, width, height, 0, 0 );

	DestroyMagickWand( rect );
	return dst;
}

void appendImgInPlace( MagickWand **dst, MagickWand *src, unsigned x, unsigned y, unsigned width, unsigned height )
{
	MagickWand *tmp = appendImg( *dst, src, x, y, width, height );
	//if( *dst != NULL ) DestroyMagickWand( *dst );
	*dst = tmp;
}

// i never want to touch magickwand again
MagickWand *makeGif( MagickWand *frames, const unsigned *timing )
{
	MagickWand *gif = NewMagickWand();
	MagickWand *curFrame = NULL;
	unsigned i;

	MagickResetIterator( frames );

	while( MagickNextImage( frames ) != MagickFalse )
	{
		curFrame = MagickGetImage( frames );
		printf("%u\n", *timing);

		i = 0;
		do
			MagickAddImage( gif, curFrame );
		while
			( ++i < *timing );

		DestroyMagickWand(curFrame);
		//MagickNextImage( frames );
		timing++;
	}
	//MagickSetImageIterations( frames, 10 * *(timing++) );
	MagickResetIterator( gif );
	MagickSetOption( gif, "loop", "0" );

	return gif;
}

// TODO make it save to .tmp.swap.FNAME
// TODO make it getc user input
bool displayAndConf( MagickWand *displayed )
{
	const char *tmpFile = NULL;
	if( fsource )
		tmpFile = GETFSUF( "prev.gif" );
	else
		tmpFile = "tempDisplay.gif";

	MagickResetIterator( displayed );
	MagickWriteImages( displayed, tmpFile, MagickTrue );
	displayFile( tmpFile );

	//efremove( tmpFile, "removes temporary show-to-user file" );
	if( fsource ) free( tmpFile );
	return false;
}
