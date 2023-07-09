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

	printf("loaded file %s (%u x %u)\n\n", fsource, pngwidth, pngheight);


	char *buf;
	iconstate *current = NULL;

	unsigned response, i, j;
	while(response = vmenuscr( 5, "Save and Quit", "Edit iconstates", "Display general info", "Display all iconstate info", "Preview full dmi"))
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
			break;

			case 4:
			buf = GETFSUF( FWORK );
			displayFile( buf );
			free( buf );
			break;
		}
	}

	buf = GETFSUF( FWORK );
	writeStateTable( buf, fsource );
	
	free(buf);
	clean();
}
