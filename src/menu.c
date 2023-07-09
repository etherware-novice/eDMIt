#include "dmi.h"
#include <ctype.h>

unsigned menuscr( unsigned menuCount, const char *exitText, const char **entries )
{
	if( !exitText ) exitText = "Back to Menu";
	int i;
	static const unsigned short perPage = 8;
	unsigned page = 0;
	unsigned pageOffset;

	bool canForward;

	while( true )
	{
		canForward = true;

		puts("\n------");
	
		for( i = 0; i < perPage - 1; i++ )
		{
			pageOffset = page * perPage;
			if( i + pageOffset >= menuCount ) 
			{
				canForward = false;
				break;
			}
			printf("%u ) %s\n", i + 1, entries[i + pageOffset]);
		}

		puts("");
		if( page )
			printf("%u ) Previous Page\n", perPage);
		if( canForward )
			printf("%u ) Next Page\n", perPage + 1);

		printf("0 ) %s\n> ", exitText);

		while( true )
		{
			printf("\b ");

			i = getchar();
			if( !isdigit(i) ) continue;

			i -= '0';

			if( i == perPage )
			{
				if( page )
				{
					page--;
					break;
				}
				continue;
			}

			if( i == perPage + 1 )
			{
				if( canForward )
				{
					page++;
					break;
				}
				continue;
			}

			if( i == 0 )
				return 0;

			i += pageOffset;
			if( (unsigned) i > menuCount ) continue;
			return i;
		}
	}
}

unsigned vmenuscr( unsigned menuCount, const char *exitText, ... )
{
	if( menuCount-- < 2 ) return 0;

	va_list args;
	va_start(args, exitText);

	unsigned i;
	char **ents = malloc(sizeof(char *) * menuCount);
	for( i = 0; i < menuCount; i++ )
		ents[i] = va_arg(args, const char *);

	unsigned retr = menuscr( menuCount, exitText, ents );
	free(ents);

	return retr;
}
