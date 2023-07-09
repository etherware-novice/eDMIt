#include "dmi.h"


void clean(void)
{
	unsigned i, j;
	for( i = 0; i < MAXSTATES; i++ )
	{
		free( statetable[i].delay );

		for( j = 0; j < MAXAUX; j++ )
			free( statetable[i].aux[j] );
	}
	MagickWandTerminus();

	fdelTemp( FWORK );
	fdelTemp( FTMP );
}

int main(int argc, char *argv[])
{
	if( argc < 2 )
	{
		fputs("Please pass a dmi file to parse!\n", stderr);
		return ENOENT;
	}

	loadStateTable(argv[1]);

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

	makeOffsetSpace( 19, 8 );


	unsigned response = vmenuscr( 11, "opt1", "opt2", "opt3", "opt4", "opt5", "opt6", "opt7", "opt8", "opt9", "opt10", "opt11" );

	printf( "\n%u\n", response );

	//swapEditState( statetable[7], 1 );
	//writeStateWork();

	//MagickWand *export = makeGif(constructStateWand(statetable[20], -1), NULL);
	//displayAndConf( export );
	//DestroyMagickWand( export );

	clean();
}
