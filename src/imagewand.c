#include <dmi.h>

#ifndef IMGVIEW

#ifdef _WIN32
#define IMGVIEW "start"
#else
#define IMGVIEW "xdg-open"
#endif

#endif


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
	return mw;
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
	MagickWand *rect = CloneMagickWand(src);
	MagickCropImage( rect, width, height, x, y );

	if( !dst ) return rect;
	MagickAddImage( dst, rect );
	MagickResetIterator( dst );


	return MagickAppendImages( dst, MagickFalse );
}

void appendImgInPlace( MagickWand **dst, MagickWand *src, unsigned x, unsigned y, unsigned width, unsigned height )
{
	MagickWand *tmp = appendImg( *dst, src, x, y, width, height );
	if( *dst == NULL ) DestroyMagickWand( *dst );
	*dst = tmp;
}

// TODO read timing info
MagickWand *makeGif( MagickWand *frames, unsigned *timing )
{
	MagickWand *gif = MagickCoalesceImages(frames);
	unsigned i;

	size_t optCount, ti;
	char **opts = MagickGetOptions( gif, NULL, &optCount );
	for( ti = 0; ti < optCount; ti++ )
		puts(opts[ti]);

	for( i = 1; i < MagickGetNumberImages(gif); i++ )
		continue;

	MagickWand *decon = MagickCompareImagesLayers( gif, CompareAnyLayer );
	MagickSetOption( decon, "loop", 0 );

	DestroyMagickWand(gif);
	return decon;
}

// TODO make it save to .tmp.swap.FNAME
// TODO make it getc user input
bool displayAndConf( MagickWand *displayed )
{
	char tmpFile[] = "/tmp/export.gif";
	MagickWriteImages( displayed, tmpFile, true );

	char *syscall = sncatf( NULL, "%s %s", IMGVIEW, tmpFile );
	system( syscall );
	free( syscall );

	return false;
}
