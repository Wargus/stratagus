//
//	(c) Copyright 1999-2001 Lutz Sammer
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//	This programm improves doc++, for better documenting
//
//	Moves / / / comments before the line containing it.
//

#include <stdio.h>
#include <stdlib.h>
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
