/*****************************************************************************
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 **
 **
 ** FILE INFORMATION
 ** ----------------
 ** Filename:        parser.c
 ** Initial author:  marcus.jagemar@gmail.com
 **
 **
 ** DESCRIPTION
 ** -----------
 ** 
 ** 
 *****************************************************************************/


/* General includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

/* Local project files */
#include "tban.h"


/**********************************************************************
 * Some contacts used in parsing
 **********************************************************************/
#define PARSE_OK                   0   /* When parsing normal contacts
                                        * succeeds */
#define PARSE_APP_OK               1   /* When parsing appointments
                                        * succeeds */
#define PARSE_ERROR                10
#define PARSE_ERROR_MULTIPLE_CELLS 11

#define PARSER_FILE_END            0xffff


/**********************************************************************
 * Name        : tagParserFunc
 * Description : A prototype for the functions to be called before and
 *               after a tag has been handled by the parser.
 **********************************************************************/
typedef int (tagParserFunc)(FILE*, struct TBan*);



/**********************************************************************
 * Name        : TagList
 * Description : Define the functions to call when obtaining the
 *               corresponding TAG
 **********************************************************************/
struct TagList {
  /* TAG definition to be identified in the file */
  char*          string;
  /* The return value from the function */
  int            retval;
  /* A function (if!=NULL) to be called by the parser before starting to
     read values from the file*/
  tagParserFunc* earlyParserFunc;
  /* A function (if!=NULL) to be called by the parser after reading
     values from the file. */
  tagParserFunc* lateParserFunc;
};
typedef struct TagList TagList;

#define taglistlength (sizeof(taglist) / sizeof(TagList))




/**********************************************************************
 * Name        : Taglist
 * Description : Link each tag to a value cell in the ContactData
 *               structure. This is the destinaation of the data read
 *               from the file.
 **********************************************************************/
int tbanDsCb(FILE*, struct TBan*);
int tbanAsCb(FILE*, struct TBan*);
int tbanChCb(FILE*, struct TBan*);
int miniNGAsCb(FILE*, struct TBan*);
int miniNGChCb(FILE*, struct TBan*);
int commentCb(FILE*, struct TBan*);
int bigNGAsCb(FILE* file, struct TBan* tban);

TagList taglist[] = {
  /* Contacts tags */
  { "TBAN_DS",             PARSE_OK,       &tbanDsCb,     NULL },
  { "TBAN_AS",             PARSE_OK,       &tbanAsCb,     NULL },
  { "TBAN_CH",             PARSE_OK,       &tbanChCb,     NULL },
  { "MINI_NG_AS",          PARSE_OK,       &miniNGAsCb,   NULL },
  { "MINI_CH",             PARSE_OK,       &miniNGChCb,   NULL },
  { "BIG_NG_AS",           PARSE_OK,       &bigNGAsCb,     NULL },

  /* MISC control tags */
  { "FILE_END",       PARSER_FILE_END,     NULL,                   NULL },
  { "#",              PARSE_OK,            &commentCb,             NULL }
};



/**********************************************************************
 * Name        : matchWord
 * Description :
 * Arguments   :
 * Returning   :
 **********************************************************************/
int matchWord(char* match, char* chall) {
  if(strcasecmp(match, chall) != 0) {
    return 0;
  } else {
    return 1;
  }
}



/**********************************************************************
 * Name        : readWord
 * Description : Read one word (until reaching a selected stopChar)
 * Arguments   : arr = The destination array
 *               StopChars = The characters that ends the read
 *               ignoreChars = Characters to ignore when reading
 *               skipLeadingChar = How many leading characters to skip
 * Returning   :
 **********************************************************************/
int readWord(FILE* infile, char* arr, char stopChars[], char ignoreChars[], char skipLeadingChar) {
  char c;
  int  i         = 0;
  int  j;
  int  endMarker = 0;
  int  retval    = PARSE_OK;
  int  leading   = 1;
  int ignored;
  
  /* Read until reaching the stop character */
  for(;;) {
    c = fgetc(infile);
    /* Match the current character to any in the list of supplied ending
       characters. If match then stop the read */
    endMarker = 0;
    for(j=0; j<strlen(stopChars); j++) {
      if(c == (char) stopChars[j]) {
        endMarker = 1;
      } /* if */
    } /* for j */

    /* Check if we have any of the characters to be ignored. If that is
     * so lets remove them from the stream and just discard them.  */
    ignored=0;
    if(ignoreChars != NULL) {
      for(j=0; j<strlen(ignoreChars); j++) {
        if(c == (char) ignoreChars[j]) {
          ignored=1;
          break;
        } /* if */
      } /* for j */
    } /* if ignoreChars */
    
    /* */
    if(endMarker == 1)
      break;

    if(feof(infile)) {
      retval = PARSE_ERROR;
      break;
    }

    if((c != ' ') && (c != '\t') && (c != '\n')) {
      leading = 0;
    }
    
    if((ignored != 1) && (leading == 0)) {
      arr[i] = c;
        i++;
    }
  } /* for (;;) */

  arr[i]='\0';
  return retval;
}


/**********************************************************************
 * Name        : convertAsciiToInt
 * Description : Convert the ascii value in the string to an integer.
 * Arguments   : string = The string represenation of the integer
 * Returning   : The value that the string represents or
 *               -2 If the first character is not a digit
 *               -3 If the conversion failed (see errno)
 **********************************************************************/
int convertAsciiToInt(char* string) {
  int value;

  /* Since the atoi-function will only convert the numerical part of the
   * string until it finds a non-numerical character we must make sure
   * that the first character is a digit, otherwise we get a 0 (zero)
   * result...*/
  if(!isdigit((int) string[0]))
    return -2;

  /* Perform the conversion */
  errno=0;
  value = atoi(string);

  if((value==0) && (errno != 0)) {
    return -3;
  }
  return value;
}


/**********************************************************************
 * Name        : commentCb
 * Description : 
 * Arguments   :
 * Returning   :
 **********************************************************************/
int commentCb(FILE* file, struct TBan* tban) {
  /* Discard all character until next newline */
  char tag[1024];
  readWord(file, tag, "\n", NULL, 1);
  return 0;
}


/**********************************************************************
 * Name        : tbanDsCb
 * Description : Read Tban digital sensor names.
 * Arguments   : -
 * Returning   : -
 **********************************************************************/
int tbanDsCb(FILE* file, struct TBan* tban) {
  char tag[256];
  int value;

  /* Read sensor number */
  readWord(file, tag, " ", NULL, 1);
  value = convertAsciiToInt(tag);

  /* Read sensor short name */
  readWord(file, tag, " ", "\"", 1);
  tban->dsName[value] = malloc(strlen(tag)+1);
  strcpy(tban->dsName[value], tag);

  /* Read sensor long name */
  readWord(file, tag, "\n", "\"", 1);
  tban->dsDescr[value]=malloc(strlen(tag)+1);
  strcpy(tban->dsDescr[value], tag);
  
  return 0;
}



/**********************************************************************
 * Name        : tbanAsCb
 * Description : Read Tban analog sensor names.
 * Arguments   : -
 * Returning   : -
 **********************************************************************/
int tbanAsCb(FILE* file, struct TBan* tban) {
  char tag[256];
  int value;

  /* Read sensor number */
  readWord(file, tag, " ", NULL, 1);
  value = convertAsciiToInt(tag);

  /* Read sensor short name */
  readWord(file, tag, " ", "\"", 1);
  tban->asName[value] = malloc(strlen(tag)+1);
  strcpy(tban->asName[value], tag);

  /* Read sensor long name */
  readWord(file, tag, "\n", "\"", 1);
  tban->asDescr[value]=malloc(strlen(tag)+1);
  strcpy(tban->asDescr[value], tag);

  return 0;
}


/**********************************************************************
 * Name        : tbanChCb
 * Description : Read TBan channel names
 * Arguments   : -
 * Returning   : -
 **********************************************************************/
int tbanChCb(FILE* file, struct TBan* tban) {
  char tag[256];
  int value;

  /* Read sensor number */
  readWord(file, tag, " ", NULL, 1);
  value = convertAsciiToInt(tag);

  /* Read sensor short name */
  readWord(file, tag, " ", "\"", 1);
  tban->chName[value] = malloc(strlen(tag)+1);
  strcpy(tban->chName[value], tag);

  /* Read sensor long name */
  readWord(file, tag, "\n", "\"", 1);
  tban->chDescr[value]=malloc(strlen(tag)+1);
  strcpy(tban->chDescr[value], tag);

  return 0;
}


/**********************************************************************
 * Name        : 
 * Description :
 * Arguments   :
 * Returning   :
 **********************************************************************/
int miniNGAsCb(FILE* file, struct TBan* tban) {
  char tag[256];
  int value;

  /* Read sensor number */
  readWord(file, tag, " ", NULL, 1);
  value = convertAsciiToInt(tag);

  /* Read sensor short name */
  readWord(file, tag, " ", "\"", 1);
  tban->miniNG.asName[value] = malloc(strlen(tag)+1);
  strcpy(tban->miniNG.asName[value], tag);

  /* Read sensor long name */
  readWord(file, tag, "\n", "\"", 1);
  tban->miniNG.asDescr[value]=malloc(strlen(tag)+1);
  strcpy(tban->miniNG.asDescr[value], tag);

  return 0;
}



/**********************************************************************
 * Name        : miniNGChCb
 * Description : Read miniNG channel names
 * Arguments   : -
 * Returning   : -
 **********************************************************************/
int miniNGChCb(FILE* file, struct TBan* tban) {
  char tag[256];
  int value;

  /* Read sensor number */
  readWord(file, tag, " ", NULL, 1);
  value = convertAsciiToInt(tag);

  /* Read sensor short name */
  readWord(file, tag, " ", "\"", 1);
  tban->miniNG.chName[value] = malloc(strlen(tag)+1);
  strcpy(tban->miniNG.chName[value], tag);

  /* Read sensor long name */
  readWord(file, tag, "\n", "\"", 1);
  tban->miniNG.chDesc[value]=malloc(strlen(tag)+1);
  strcpy(tban->miniNG.chDesc[value], tag);

  return 0;
}


/**********************************************************************
 * Name        : bigNGAsCb
 * Description : Read Tban analog sensor names.
 * Arguments   : -
 * Returning   : -
 **********************************************************************/
int bigNGAsCb(FILE* file, struct TBan* tban) {
  char tag[256];
  int value;

  /* Read sensor number */
  readWord(file, tag, " ", NULL, 1);
  value = convertAsciiToInt(tag);

  /* Read sensor short name */
  readWord(file, tag, " ", "\"", 1);
  tban->bigNG.asName[value] = malloc(strlen(tag)+1);
  strcpy(tban->bigNG.asName[value], tag);

  /* Read sensor long name */
  readWord(file, tag, "\n", "\"", 1);
  tban->bigNG.asDescr[value]=malloc(strlen(tag)+1);
  strcpy(tban->bigNG.asDescr[value], tag);

  return 0;
}


/**********************************************************************
 * Name        : tban_parseConfig
 * Description : Main parse function for the TBan configuration
 * 		 file. This function will try to read channel and sensor
 * 		 settings from the file and store them conveniently.
 * Arguments   : tban     = The TBan struct to work on
 *               filename = Name of config file
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_CONFIG_FILE_ERROR
 **********************************************************************/
int tban_parseConfig(struct TBan* tban, char* filename) {
  int i;
  int processed;
  FILE* infile;
  char tag[128];

  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Open file for reading */
  infile = fopen(filename, "r");
  if(infile == NULL) {
    return TBAN_CONFIG_FILE_ERROR;
  }

  /* Parse the file */
  while(!feof(infile)) {
    readWord(infile, tag, " ", NULL, 1);
    processed=0;
    for(i=0; i<taglistlength; i++) {
      if(matchWord(tag, taglist[i].string)) {
        int ret;
        /* Call the parse function for the TAG if there exists a parse
         * function, otherwise just skip it. */
        if(taglist[i].earlyParserFunc != NULL)
          ret = taglist[i].earlyParserFunc(infile, tban);
        processed=1;
        break;
      }
    } /* for */

    /* Print some error message if the tag has not been processed above */
    if((processed==0) && (!feof(infile))) {
      printf("Error parsing config file when processing parameter: %s\n", tag);
      return TBAN_CONFIG_FILE_ERROR;
    }

  } /* while feof */
 
  /* Close file */
  fclose(infile);
  return TBAN_OK;
}



