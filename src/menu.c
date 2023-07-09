#include "dmi.h"
#include <ctype.h>

unsigned menuscr( unsigned menuCount, const char **entries )
{
	int i;
	static const unsigned short perPage = 8;
	unsigned page = 0;
	unsigned pageOffset;

	bool canForward;

	while( true )
	{
		canForward = true;

		puts("\n------");
	
		for( i = 0; i < perPage; i++ )
		{
			pageOffset = page * perPage;
			if( i + pageOffset >= menuCount ) 
			{
				canForward = false;
				break;
			}
			printf("%u ) %s\n", i + 1, entries[i + pageOffset]);
		}

		if( page )
			printf("%u ) Previous Page\n", perPage);
		if( canForward )
			printf("%u ) Next Page\n", perPage + 1);

		printf("\n> ");

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

			i += pageOffset;
			if( i >= menuCount ) continue;
			return i - 1;
		}
	}
}

unsigned vmenuscr( unsigned menuCount, ... )
{
	va_list args;
	va_start(args, menuCount);

	unsigned i;
	char **ents = malloc(sizeof(char *) * menuCount);
	for( i = 0; i < menuCount; i++ )
		ents[i] = va_arg(args, const char *);

	return menuscr( menuCount, ents );
}
