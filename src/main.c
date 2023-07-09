#include "dmi.h"
#include <ctype.h>


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
	char **arr;
	iconstate *current = NULL;
	MagickWand *mw = NULL;
	MagickWand *swapmw = NULL;

	unsigned response, i, j;
	while((response = vmenuscr( 5, "Save and Quit", "Edit iconstates", "Display general info", "Display all iconstate info", "Preview full dmi")))
	{
		current = NULL;
		arr = NULL;
		switch( response )
		{
			case 1:
			while((response = menuscr(i, NULL, arr = arrayOfStateNames( &i ))))
			{
				free(arr);
				current = statetable + (response - 1);
				printf("state: %s\n", current->name );

				while((response = vmenuscr( 4, "Return to List", "Preview state", "Edit state", "Display state info" )))
				{
					switch( response )
					{
						case 1:
						swapmw = constructStateWand( *current, -1 );
						mw = makeGif( swapmw, current->delay );

						displayAndConf(mw);
						DestroyMagickWand(mw); mw = NULL;
						break;

						case 2:
						i = 0;
						if( current->dirs > 1 )
						{
							printf("State has multiple directions (%u), which would you like to edit?\n", current->dirs);
							while((i = getchar()))
							{
								if( !isdigit(i) ) continue;
								i -= '0';
								if( ++i > current->dirs ) continue;
								break;
							}
						}
						printf("Loaded state into file ext %s, press enter when modifications are done\n", FTMP);
						swapEditState( *current, i );
						break;

						case 3:
						printf("\n-----%s-----\n", current->name);
						printf("directions: %u\t", current->dirs);
						printf("anim frames: %u\n", current->frames);

						if( current->aux[0] )
							for( i = 0; i < MAXAUX; i++ )
								if( current->aux[i] )
									printf("aux line %u: %s\n", i, current->aux[i]);
						
						printf("offset into file: %u\n\n", current->offset);
					}
				}
			}
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
