#ifndef DMI_H

#include <stdio.h>  // for the FILE specifier
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <ImageMagick-7/MagickWand/MagickWand.h>

#define MAXNAME 64
#define MAXSTATES 200
#define MAXAUX 5

#define CHUNK "zTXt"
#define FHEAD ".tmp.swap"

typedef struct {
	char name[MAXNAME];
	unsigned frames;
	unsigned dirs;
	unsigned *delay;
	char *aux[MAXAUX];	// extra lines not parsed by above
	unsigned offset;	// how many icons into the png does it start (0 indexed)
} iconstate;


extern iconstate statetable[MAXSTATES];
extern char header[MAXNAME];
extern unsigned width;
extern unsigned height;

// parser.c - high level reading data
void loadStateTable( const char *path );	// loads state table from png path
void newStateEnt( const char *start );		// makes new entry by reading the raw lines
unsigned getNextEmptyEnt(void);			// gets next empty entry, testing name
void recalculateOffsets( unsigned start );	// recalculates the offsets var on the struct, starting at the start var

// export.c - high level writing data
void writeStateTable( const char *base, const char *path );	// copies the png base from base to path, inserting state table

// buffer.c - wrappers for c functions
void *emalloc( size_t size, const char *desc );	// on error, display the strerror of the call with the added description (can be null)
void *erealloc( void *ptr, size_t size, const char *desc );
FILE *efopen( const char *path, const char *mode, const char *desc );

void fswapos( FILE *stream, long *pos );		// seeks to pos, saves old stream position to pos
char *sncatf( char *str, const char *format, ... );	// takes a MALLOCED char array of size `size`, appends formatted string, updates size pointer to new size of block nd returns

// imagewand.c - image manipulation with imagemagick
MagickWand *eLoadImg( const char *path );									// loads image at path into wand, returns
void addSize( MagickWand *mw, int xMov, int yMov );								// adds xmov and ymov to size of wand
void multSize( MagickWand *mw, double xMov, double yMov );							// multiplies current x and y of wand by xmov/ymov
MagickWand *appendImg( MagickWand *dst, MagickWand *src, unsigned x, unsigned y, unsigned width, unsigned height );	// appends rectangle from src to bottom right of dst, or makes a new image if dst is NULL
void appendImgInPlace( MagickWand **dst, MagickWand *src, unsigned x, unsigned y, unsigned width, unsigned height );	// frees the old dest, updates the var
MagickWand *makeGif( MagickWand *frames, unsigned *timing );							// makes a gif with frames, and optionally an array of timing info (ms)
bool displayAndConf( MagickWand *displayed );									// displays to user, prompts for y/n, returns answer

// fileman.c - low level file calls
bool searchASCII( FILE *haystack, const char *needle );	// searches thru a file for the ascii string, returns success
uint32_t fnibtou( FILE *source, bool bend );		// gets a nibble (4 bytes) from source and converts to a 32b number
bool fwritenib( FILE *source, uint32_t num, bool bend ); // writes a nibble to the source file, if bend is true it writes big endien, else little

// reads bytes from source, decompresses it to a malloced char array, returns it
unsigned char *funcompz( FILE *source, unsigned bytes, unsigned long *arrsize ); 
// compresses char array, writes it to file, returns compressed size
unsigned long fwritecomp( FILE *source, const unsigned char *from, unsigned long fromLen );

// reads x bytes from source, calculates the crc32, writes it with big-endian if bend is true
void fwritecrc32( FILE *source, unsigned long bytes, bool bend );

uint32_t crc32_byte(unsigned char *p, unsigned bytelength);


#endif
