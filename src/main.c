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

	//fdelTemp( FWORK );
	//fdelTemp( FTMP );
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

	/*
	unsigned i, j;
	iconstate *cur;

	makeOffsetSpace( 19, 8 );
	*/


	unsigned response, i, j;
	iconstate *current = NULL;
	while(response = vmenuscr( 4, "Save and Quit", "Edit iconstates", "Display general info", "Display all iconstate info"))
	{
		current = NULL;
		switch( response )
		{
			case 1:
			puts("iconstates");
			break;

			case 2:
			j = 0;
			recalculateOffsets(0);

			printf("\n--------%s--------\n", fsource );
			printf("dimensions: %ux%u\n", pngwidth, pngheight);

			for( i = 0; i < MAXSTATES; i++ )
			{
				if( statetable[i].name[0] == '\0' ) continue;
				current = statetable + i;
				j++;
			}
			printf("state count: %u\n", j);
			printf("total individual icons: %u\n\n", current->offset + current->size);
			getchar();
			break;

			case 3:
			for( i = 0; i < MAXSTATES; i++ )
			{
				current = statetable + i;
				if( current->name[0] == '\0' ) continue;
				printf("(%u/%u) [ ", current->frames, current->dirs);

				for( j = 0; j < current->frames; j++ )
					printf("%u ", current->delay[j]);

				printf("] <%u - %hu> %s\n", current->offset, current->size, current->name);
			}
		}
	}

	//swapEditState( statetable[7], 1 );
	//writeStateWork();

	//MagickWand *export = makeGif(constructStateWand(statetable[20], -1), NULL);
	//displayAndConf( export );
	//DestroyMagickWand( export );

	clean();
}
