#include "dmi.h"


int main(int argc, char *argv[])
{
	if( argc < 2 )
	{
		fputs("Please pass a dmi file to parse!\n", stderr);
		return ENOENT;
	}

	loadStateTable(argv[1]);
	writeStateTable( "checker.png", "/tmp/checker.dmi" );


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


	displayAndConf( makeGif(constructStateWand(statetable[20], -1), NULL) );
}
