//
//	(c) Copyright Lutz Sammer
//	
//	This programm improves doc++, for better documenting
//
//	Moves / / / comments before the line containing it.
//

#include <stdio.h>
#include <string.h>

#undef main

int main(int argc,char** argv)
{
    FILE* fin;
    FILE* fout;

    fin=stdin;
    fout=stdout;
    
    if( argc!=2 ) {
	fprintf(stderr,"aledoc: input\n");
	exit(-1);
    }

    fin=fopen(argv[1],"r");
    if( !fin ) {
	fprintf(stderr,"aledoc: can't open input `%s'\n",argv[1]);
	exit(-1);
    }

    while( !feof(fin) ) {
	char line[8192];
	char* comment;

	fgets(line,sizeof(line),fin);
	if( (comment=strrchr(line,'/'))
		&& comment-2>line && comment[-1]=='/' && comment[-2]=='/' ) {
	    comment-=2;
	    fwrite(comment,strlen(comment),1,fout);
	    fwrite(line,comment-line,1,fout);
	    fputs("\n",fout);
	} else {
	    fwrite(line,strlen(line),1,fout);
	}
    }

    return 0;
}
