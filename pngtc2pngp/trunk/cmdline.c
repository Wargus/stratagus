/*
    (c) Frank Loeffler (2004)

    This file is part of pngtc2pngp, a conversion utility from true
    type color pngs to palette pngs including partial alpha support.
    If fact it is the only C-file in the utility.

    pngtc2pngp is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    pngtc2pngp is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with pngtc2pngp; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdlib.h>

#include "pngtc2pngp.h"

/*
   file1 [file2 [-p palette]] [-s file3 [file4]]
*/

void cmdline(int argc, char** argv,
             char** fn_input1, char** fn_palette1, char** fn_output1,
             char** fn_input2, char** fn_output2,
             char** title, char** author, char** copyright,
             char** disclaimer, char** source)
{
    int status=0;
    for(int i = 1; i < argc; i++)
    {
        /* verbose */
        if (!strcmp(argv[i], "-v"))
        {
            option_verbose++;
            DEBUG(1,"[cmdl] Verbose information requested\n");
        }
        /* version */
        else if (!strcmp(argv[i], "-V"))
        {
            option_version=1;
            DEBUG(1,"[cmdl] Version information requested\n");
        }
        /* help */
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
            option_help=1;
            DEBUG(1,"[cmdl] Help requested\n");
        }
        else if (!strcmp(argv[i], "-f"))
        {
            option_force=1;
            DEBUG(1,"[cmdl] Overwrite forced\n");
        }
        else if (!strcmp(argv[i], "--title"))
        {
            if (i >= argc-1)
            {
                printf("--title without text specified\n");
                exit(1);
            }
            i++;
            *title=argv[i];
            DEBUG(1,"[cmdl] Title '%s' specified\n" _C_ *title);
        }
        else if (!strcmp(argv[i], "--author"))
        {
            if (i >= argc-1)
            {
                printf("--author without text specified\n");
                exit(1);
            }
            i++;
            *author=argv[i];
            DEBUG(1,"[cmdl] Author '%s' specified\n" _C_ *author);
        }
        else if (!strcmp(argv[i], "--copyright"))
        {
            if (i >= argc-1)
            {
                printf("--copyright without text specified\n");
                exit(1);
            }
            i++;
            *copyright=argv[i];
            DEBUG(1,"[cmdl] Copyright '%s' specified\n" _C_ *copyright);
        }
        else if (!strcmp(argv[i], "--disclaimer"))
        {
            if (i >= argc-1)
            {
                printf("--disclaimer without text specified\n");
                exit(1);
            }
            i++;
            *disclaimer=argv[i];
            DEBUG(1,"[cmdl] Disclaimer '%s' specified\n" _C_ *disclaimer);
        }
        else if (!strcmp(argv[i], "--source"))
        {
            if (i >= argc-1)
            {
                printf("--source without text specified\n");
                exit(1);
            }
            i++;
            *source=argv[i];
            DEBUG(1,"[cmdl] Source '%s' specified\n" _C_ *source);
        }
        /* palette input file */
        else if (!strcmp(argv[i], "-p"))
        {
            if (i >= argc-1)
            {
                printf("-p without palette file name specified\n");
                exit(1);
            }
            i++;
            *fn_palette1=argv[i];
            DEBUG(1,"[cmdl] Palette file '%s' specified\n" _C_ *fn_palette1);
        }
        else if (!strcmp(argv[i], "--player-color"))
        {
            if (i >= argc-1)
            {
                printf("--player-color without player color hue specified\n");
                exit(1);
            }
            i++;
            if ((sscanf(argv[i], "%d", &option_player_color_hue)!=1) ||
                (option_player_color_hue <   0) ||
                (option_player_color_hue > 359))
            {
                printf("player color should be an integer in the range "
                       "from 0 to 359 (the hue value)\n");
                exit(1);
            }
            DEBUG(1,"[cmdl] Player color %d specified\n" _C_ \
                    option_player_color_hue);
        }
        /* shadow option */
        else if (!strcmp(argv[i], "-s"))
        {
            if (i >= argc-1)
            {
                printf("-s without shadow input file specified\n");
                exit(1);
            }
            if (status==0)
            {
                printf("No 'normal' input file for shadow extraction "
                       "specified\n");
                exit(1);
            }
            status=2;
        }
        /* filename lists */
        else
        {
            switch(status)
            {
              case 0:
                *fn_input1 = argv[i];
                status=1;
                DEBUG(1,"[cmdl] Normal input filename '%s' specified\n" _C_ \
                        *fn_input1);
                break;
              case 1:
                *fn_output1 = argv[i];
                status=4;
                DEBUG(1,"[cmdl] Normal output filename '%s' specified\n" _C_ \
                        *fn_output1);
                break;
              case 2:
                *fn_input2 = argv[i];
                status=3;
                DEBUG(1,"[cmdl] Shadow input filename '%s' specified\n" _C_ \
                        *fn_input2);
                break;
              case 3:
                *fn_output2 = argv[i];
                status=4;
                DEBUG(1,"[cmdl] Shadow output filename '%s' specified\n" _C_ \
                        *fn_output2);
                break;
              case 4:
                printf("More filenames specified than required\n");
                exit(1);
                break;
              default:
                printf("Internal error\n");
                exit(1);
            }
        }
    }
    if (!option_version)
    switch (option_help ? 0 : status)
    {
      case 0:
        printf(
          "%s [-Vvf] [-p palette] input1 [output1] [-s input2 [output2]] "
          "--player-color <color>\n"
          " -V         : print version information and exit\n"
          " -h, --help : print this help\n"
          " -v         : show verbose information\n"
          " -f         : force writing to input files when output "
                         "filenames are obmitted\n"
          " -p <file> : specify filename for palette needed by conversion\n\n"
          " PNG text informations:\n"
          "--title      <string> : Set the title of the image\n"
          "--author     <string> : Set the author of the image\n"
          "--copyright  <string> : Set copyright information about the image\n"
          "--disclaimer <string> : Set a disclaimer about the image\n"
          "--source     <string> : Set information about the source of the "
                                  "image\n\n"
          " BOS (Stratagus) specific options:\n"
          " -s <files>: specify filenames for input and output files of "
                       "shadow extraction\n"
          " --player-color <color> : set the player color to a hue value of \n"
          "                          <value> (in the range of 0..359)\n"
          "                          If not specified, player colors are not "
                                     "used\n"
          "                          typical values are:\n"
          "                            0 : red\n"
          "                          120 : green\n"
          "                          240 : blue\n"
          , argv[0]);
        exit(1);
        break;
      case 1:
        if (option_force)
        {
            fn_output1 = fn_input1;
            DEBUG(1,"[cmdl] Forced writing to input file '%s'\n" _C_ *fn_input1);
        }
        else
        {
            printf("No output1 specified and '-f' not given\n");
            exit(1);
        }
        break;
      case 3:
        if (option_force)
        {
            fn_output2 = fn_input2;
            DEBUG(1,"[cmdl] Forced writing to input file '%s'\n" _C_ *fn_input2);
        }
        else
        {
            printf("No output2 specified and '-f' not given\n");
            exit(1);
        }
        break;
    }

    DEBUG(1,"[cmdl] command line reading went ok\n");
    return;
}

