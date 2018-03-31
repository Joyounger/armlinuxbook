#include <stdio.h> 
#include <sqlite.h> 
#include<stdlib.h> 

static int callback(void *NotUsed, int argc, char **argv, char **azColName){ 

int i; 
for(i=0; i<argc; i++)
{ 
	printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); 
} 

	printf("\n"); 
	return 0; 
} 

int main(int argc, char **argv)
{ 
	sqlite *db; 
	char *zErrMsg = 0; 
	int rc; 
	
	if( argc!=3 )
	{ 
		fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]); 
		exit(1); 
	} 
	db = sqlite_open(argv[1], 0666,&zErrMsg); 
	if( db==NULL)
	{ 
		fprintf(stderr, "Can't open database: %s\n", &zErrMsg); 
		sqlite_close(db); 
		exit(1); 
	} 
	rc = sqlite_exec(db, argv[2], callback, 0, &zErrMsg); 
	
	if( rc!=SQLITE_OK )
	{ 
		fprintf(stderr, "SQL error: %s\n", zErrMsg); 
	} 
	sqlite_close(db); 
	return 0; 
} 

