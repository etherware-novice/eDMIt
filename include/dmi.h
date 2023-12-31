#ifndef DMI_H

#include <stdio.h>  // FILE specifier
#include <stdint.h> // uint32_t 
#include <stdbool.h>
#include <ImageMagick-7/MagickWand/MagickWand.h> // magickwand return
#include <zlib.h>

// not needed for the header, but used often in multiple files
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#ifndef IMGVIEW

#ifdef _WIN32
#define IMGVIEW "start"
#else
#define IMGVIEW "xdg-open"
#endif

#endif

#define BYONDTPS 10

#define MAXNAME 64
#define MAXSTATES 200
#define MAXAUX 5

#define MINCOLDEFAULT 10

#define CHUNK "zTXt"
#define FWORK ".swap.png"
#define FTMP ".tmp.png"

#define GETFSUF( suf ) ( sncatf( NULL, "%s%s", fsource, suf ) )

typedef struct {
	char name[MAXNAME];
	unsigned frames;
	unsigned dirs;
	unsigned *delay;
	char *aux[MAXAUX];	// extra lines not parsed by above
	unsigned offset;	// how many icons into the png does it start (0 indexed)
	unsigned short size;	// how many icons this iconstate has
} iconstate;


extern iconstate statetable[MAXSTATES];
extern char header[MAXNAME];
extern char *fsource;
extern uint32_t pngwidth;	// width of full png
extern uint32_t pngheight;	// height of full png

extern unsigned width;		// width of one icon
extern unsigned height;		// height of one icon

// parser.c - high level reading data
void loadStateTable( const char *path );	// loads state table from png path
void newStateEnt( const char *start );		// makes new entry by reading the raw lines
unsigned getNextEmptyEnt(void);			// gets next empty entry, testing name
void recalculateOffsets( unsigned start );	// recalculates the offsets var on the struct, starting at the start var

// export.c - high level writing data
char **arrayOfStateNames( unsigned *size );				// gets malloced array of state names, writes total to size
void moveOffsetToOffset( MagickWand *mw, unsigned dst, unsigned src, bool expand );	// copies offset src on top of offset dst, if oob and expand is true it will increase size
void makeOffsetSpace( unsigned offset, int spaces );			// either makes more space after offset, or removes icons after offset if negative ( -2 would remove offset and the next state )
void swapEditState( iconstate data, int dir, const char *edit );	// writes the states to FTMP, waits, then writes it back into FWORK, calling system with the edit 
MagickWand *constructStateWand( iconstate data, int dir );		// constructs statewand for state, for direction dir (or all if -1) (not implemented)
void writeStateTable( const char *base, const char *path );		// copies the png base from base to path, inserting state table
void writeStateWork(void);						// autofills writestatetable with fsource+FWORK and fsource

// menu.c - menu programming funcs
unsigned menuscr( unsigned menuCount, const char *exitText, const char **entries );		// displays menu, returns users choice
unsigned vmenuscr( unsigned menuCount, const char *exitText, ... );				// variatic version of above

// buffer.c - wrappers for c functions
void *emalloc( size_t size, const char *desc );	// on error, display the strerror of the call with the added description (can be null)
void *erealloc( void *ptr, size_t size, const char *desc );
FILE *efopen( const char *path, const char *mode, const char *desc );
void efremove( const char *path, const char *desc );

void fcopyTemp( const char *dstsuffix, const char *srcsuffix );	// copies fsource + srcsuffix to fsource + dstsuffix (or plain fsource if null)
FILE *fopenTemp( const char *suffix, const char *mode );	// opens file fsource + suffix
MagickWand *imgopenTemp( const char *suffix );			// opens file fsource + suffix as a magickwand
void fdelTemp( const char *suffix );				// deletes file fsource + suffix

void arrshiftfw( void *arr, unsigned start, size_t size, size_t nmemb );	// shifts array forward one starting at start, zeroing the old start pos and discarding last entry
void arrshiftbw( void *arr, unsigned start, size_t size, size_t nmemb );	// shifts array backward one starting at start, zeroing the last entry and discarding start

void fgetsln( char *s, int size, FILE *stream );		// fgets a line, removes the newline from s, and skip up to the next newline from stream
char *strplace( const char *source, const char *find, const char *replace );	// mallocs a new array of source, with find replaced by replace
void fswapos( FILE *stream, long *pos );			// seeks to pos, saves old stream position to pos
char *sncatf( char *str, const char *format, ... );		// takes a MALLOCED char array of size `size`, appends formatted string, updates size pointer to new size of block nd returns
void displayFile( const char *path );				// displays file path

// imagewand.c - image manipulation with imagemagick
bool calculateOffsetPos( unsigned offset, unsigned *x, unsigned *y );	// calculates the topleft position of the offset into the statetable, returns false if the position is out of bounds
MagickWand *eLoadImg( const char *path );									// loads image at path into wand, returns
void touchImage( const char *path, const char *color, unsigned x, unsigned y );					// creates a new blank image at the file path specified
void resizeWand( MagickWand *mw, unsigned xNew, unsigned yNew );						// resizes mw to xnew, ynew
void addSize( MagickWand *mw, int xMov, int yMov );								// adds xmov and ymov to size of wand
void multSize( MagickWand *mw, double xMov, double yMov );							// multiplies current x and y of wand by xmov/ymov
MagickWand *appendImg( MagickWand *dst, MagickWand *src, unsigned x, unsigned y, unsigned width, unsigned height );	// appends rectangle from src to bottom right of dst, or makes a new image if dst is NULL
void appendImgInPlace( MagickWand **dst, MagickWand *src, unsigned x, unsigned y, unsigned width, unsigned height );	// frees the old dest, updates the var
MagickWand *makeGif( MagickWand *frames, const unsigned *timing );							// sets up a wand to be exported as gif, optionally with a timing array provided
bool displayAndConf( MagickWand *displayed );									// displays to user, prompts for y/n, returns answer

// fileman.c - low level file calls
bool searchASCII( FILE *haystack, const char *needle );	// searches thru a file for the ascii string, returns success
uint32_t freadu32( FILE *source, bool bend );		// gets 4 bytes from source, bend being if reading as a big endian or little, and converts to a 32b number
bool fwriteu32( FILE *source, uint32_t num, bool bend ); // writes 4 bytes to the source file, bend being if writing a big or little endian

// reads bytes from source, decompresses it to a malloced char array, returns it
unsigned char *funcompz( FILE *source, unsigned bytes, unsigned long *arrsize ); 
// compresses char array, writes it to file, returns compressed size
unsigned long fwritecomp( FILE *source, const unsigned char *from, unsigned long fromLen );

// reads x bytes from source, calculates the crc32, writes it with big-endian if bend is true
void fwritecrc32( FILE *source, unsigned long bytes, bool bend );

// copies file from src to dst
void copyFile( const char *dst, const char *src );

uint32_t crc32_byte(unsigned char *p, unsigned bytelength);


#endif
