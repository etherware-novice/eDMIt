#include "dmi.h"
#include <string.h>
#include <ctype.h>


iconstate statetable[MAXSTATES];
char header[MAXNAME];
char *fsource = NULL;
unsigned width;
unsigned height;

// TODO make fsource work with directories
// or in general
// this is going to be a disaster
void loadStateTable( const char *path )
{
	if( fsource ) free(fsource);
	fsource = sncatf( NULL, "%s", path );
	width = 0;
	height = 0;

	unsigned i = 0;
	char cur;
	FILE *target = efopen( path, "rb", "opening dmi for parsing" );

	if( target == NULL ) { errno = ENOENT; return; };
	if( !searchASCII(target, CHUNK) ) { errno = EOPNOTSUPP; return; };

	fseek(target, -8, SEEK_CUR);
	uint32_t chunkSize = fnibtou( target, true );

	fseek(target, 4, SEEK_CUR);
	while(( cur = fgetc(target) ) != '\0') if( i < MAXNAME - 1 ) header[i++] = cur;
	header[i] = '\0';
	fgetc(target);	// skips the second null term

	char *rawText = (char *) funcompz( target, chunkSize, NULL );


	char *offText = rawText;
	const char stateKey[] = "state = \"";

	i = 0;
	while( *offText != '\0' )
	{
		if(!strncmp( offText, "width", 5 )) width = strtoul(strchr(offText, '=') + 1, NULL, 10);
		if(!strncmp( offText, "height", 5 )) height = strtoul(strchr(offText, '=') + 1, NULL, 10);
		if( *offText == stateKey[i] ) i++;
		else i = 0;

		if( stateKey[i] == '\0' ) newStateEnt( ++offText );
		offText++;
	}

	free( rawText );
	fclose( target );

	recalculateOffsets(0);
}

void newStateEnt( const char *start )
{
	unsigned offset, arrsiz;
	unsigned *field = NULL;
	char *endptr;

	offset = getNextEmptyEnt();
	if( offset == MAXSTATES ) { errno = EOVERFLOW; return; };

	iconstate *newEnt = statetable + offset;

	for( offset = 0; start[offset] != '\"'; offset++ )
	{
		if(start[offset] == '\0') 
		{
			errno = ESPIPE;
			newEnt->name[0] = '\0';
			return;
		}
		newEnt->name[offset] = start[offset];
	}
	newEnt->name[offset] = '\0';

	while( isspace(*start) ) start++;
	if(start[offset] == '\0') 
	{
		errno = ESPIPE;
		return;
	}

	while((start = strchr(start, '\n')))
	{
		field = NULL;
		arrsiz = 1;
		start++;
		while( isblank(*start) ) start++;

		if( *start == '\0' )
		{
			errno = ESPIPE;
			break;
		}

		if( *start == '\n' ) break; // empty line
		if(!strncmp( "state", start, 5 )) break;  // encroaching on next entry

		if(!strncmp( "dirs", start, 4 )) field = &(newEnt->dirs);
		else if(!strncmp( "frames", start, 5 )) field = &(newEnt->frames);
		else if(!strncmp( "delay", start, 5 ))
		{
			arrsiz = newEnt->frames;
			if( !arrsiz )  // shouldnt really happen since offset is usually after frames but
				arrsiz = strchr(start, '\n') - start;	// bytes until the newline, overkill is better than under

			if( newEnt->delay ) free(newEnt->delay);
			newEnt->delay = field = emalloc(sizeof(unsigned) * arrsiz, "allocating delay field");
		}

		if( !field )
		{
			if( *start == '#' ) continue;
			if( *(start + 1) == '#' ) continue;

			for( offset = 0; offset < MAXAUX; offset++ )
				if(newEnt->aux[offset] == NULL) break;

			if( offset >= MAXAUX ) continue;
			for( arrsiz = 0; start[arrsiz] != '\0'; arrsiz++ ) if( start[arrsiz] == '\n' ) break;
			arrsiz++;	// null byte
			
			endptr = emalloc(sizeof(char) * (arrsiz+1), "allocating aux field");
			memset( endptr, '\0', arrsiz + 1 );
			*endptr = '\t';

			memcpy( endptr + 1, start, arrsiz - 1 );
			newEnt->aux[offset] = endptr;
			continue;
		}

		while( !isdigit(*start) ) if( *(start++) == '\n' ) break;

		while( arrsiz > 0 )
		{
			offset = strtoul( start, &endptr, 10 );
			if( start == endptr ) *field = 1;
			else *field = offset;

			if( !iscntrl(*start) ) start++;
			field++;
			arrsiz--;
		}
	}

	if( !newEnt->dirs ) newEnt->dirs = 1;
	if( !newEnt->frames ) newEnt->frames = 1;
	if( !newEnt->delay )
	{
		newEnt->delay = field = emalloc(sizeof(unsigned) * newEnt->frames, "allocating fallback delay field");
		for( offset = 0; offset < newEnt->frames; offset++ )
			field[offset] = 1;
	}
}

unsigned getNextEmptyEnt(void)
{
	for( unsigned i = 0; i < MAXSTATES; i++ )
		if( statetable[i].name[0] == '\0' ) return i;

	return MAXSTATES;
}

void recalculateOffsets( unsigned start )
{
	unsigned offset = statetable[start].offset;
	unsigned short size;

	while( start < MAXSTATES )
	{
		statetable[start].offset = offset;
		size = statetable[start].frames * statetable[start].dirs;

		statetable[start].size = size;
		offset += size;

		start++;
	}
}
