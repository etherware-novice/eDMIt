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


	char buf[MAXNAME];
	char **arr = NULL;
	MagickWand *mw = NULL;
	MagickWand *swapmw = NULL;

	unsigned current, response, i, j;
	while((response = vmenuscr( 5, "Save and Quit", "Edit iconstates", "Display general info", "Display all iconstate info", "Preview full dmi")))
	{
		switch( response )
		{
			case 1:
			while((response = menuscr(i, NULL, arr = arrayOfStateNames( &i ))))
			{
				free(arr);
				arr = NULL;

				current = response - 1;
				printf("state: %s\n", statetable[current].name );

				while((response = vmenuscr( 8, "Return to List", "Rename state", "Preview state", "Edit state", "Add/Remove Frames", "Display state info", "Add state before", "Add state after" )))
				{
					switch( response )
					{
						case 1:
						printf("What name should it have? (Empty to cancel)\n");
						if(!fgets(buf, MAXNAME, stdin)) break;

						for( j = 0; j < MAXNAME; j++ )
						{
							if( buf[j] != '\n' ) continue;
							buf[j] = '\0';
						};
						strncpy( statetable[current].name, buf, MAXNAME - 1 );
						break;

						case 2:
						swapmw = constructStateWand( statetable[current], -1 );
						mw = makeGif( swapmw, statetable[current].delay );

						displayAndConf(mw);
						DestroyMagickWand(mw); mw = NULL;
						DestroyMagickWand(swapmw); swapmw = NULL;
						break;

						case 3:
						i = 0;
						if( statetable[current].dirs > 1 )
						{
							printf("State has multiple directions (%u), which would you like to edit?\n", statetable[current].dirs);
							while((i = getchar()))
							{
								if( !isdigit(i) ) continue;
								i -= '0';
								if( ++i > statetable[current].dirs ) continue;
								break;
							}
						}
						printf("Loaded state into file ext %s, press enter when modifications are done\n", FTMP);
						swapEditState( statetable[current], i );
						break;

						case 4:
						puts("How many frames to add/remove? (Negative for remove)");
						while((i = getchar()))
						{
							if( !isdigit(i) ) continue;
							i -= '0';
							break;
						}
						if( i == 0 ) break;
						makeOffsetSpace( (statetable[current].offset + statetable[current].size) - 1, i * statetable[current].dirs );
						statetable[current].frames += i;
						recalculateOffsets(0);

						break;

						case 5:
						printf("\n-----%s-----\n", statetable[current].name);
						printf("directions: %u\t", statetable[current].dirs);
						printf("anim frames: %u\n", statetable[current].frames);

						if( statetable[current].aux[0] )
							for( i = 0; i < MAXAUX; i++ )
								if( statetable[current].aux[i] )
									printf("aux line %u: %s\n", i, statetable[current].aux[i]);
						
						printf("offset into file: %u\n\n", statetable[current].offset);
						break;

						case 6:
						case 7:
						response -= 6;
						if( statetable[MAXSTATES - 1].name[0] != '\0' ) break;

						printf("What name should it have? (Empty to cancel)\n");
						if(!fgets(buf, MAXNAME, stdin)) break;

						for( j = 0; j < MAXNAME; j++ )
						{
							if( buf[j] != '\n' ) continue;
							buf[j] = '\0';
							break;
						}

						current += response;
						arrshiftfw(statetable, current, sizeof(iconstate), MAXSTATES);
						
						strncpy( statetable[current].name, buf, MAXNAME - 1 );

						printf("How many frames?\n");
						while( !statetable[current].frames )
						{
							fgets(buf, MAXNAME, stdin);
							i = strtoul(buf, NULL, 10);

							if( errno ) perror("parsing error");
							statetable[current].frames = i;
						}

						statetable[current].delay = malloc(sizeof(unsigned) * i );
						for( j = 0; j < i; j++ )
							statetable[current].delay[j] = 1;

						printf("How many directions? (1, 4, 8)\n");
						while( !statetable[current].dirs )
						{
							fgets(buf, MAXNAME, stdin);
							i = strtoul(buf, NULL, 10);
							if( errno ) perror("parsing error");

							if( !i ) continue;
							if( i == 1 || i == 4 || i == 8 ) statetable[current].dirs = i;
						}


						if( current )
						{
							recalculateOffsets(current-1);
							makeOffsetSpace( statetable[current - 1].offset + (statetable[current-1].size - 1), statetable[current].size );
						}
						else 
							recalculateOffsets(0);
						break;
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
				j++;
			}
			printf("state count: %u\n", j);
			printf("total individual icons: %u\n\n", statetable[j].offset + statetable[j].size);
			getchar();
			break;

			case 3:
			for( i = 0; i < MAXSTATES; i++ )
			{
				if( statetable[i].name[0] == '\0' ) continue;
				printf("(%u/%u) [ ", statetable[i].frames, statetable[i].dirs);

				for( j = 0; j < statetable[i].frames; j++ )
					printf("%u ", statetable[i].delay[j]);

				printf("] <%u - %hu> %s\n", statetable[i].offset, statetable[i].size, statetable[i].name);
			}
			break;

			case 4:
			snprintf( buf, MAXNAME-1, "%s%s", fsource, FWORK );
			displayFile( buf );
			break;
		}
	}

	snprintf( buf, MAXNAME-1, "%s%s", fsource, FWORK );
	writeStateTable( buf, fsource );
	
	clean();
}
