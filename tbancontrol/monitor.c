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
 ** Filename:        monitor.c
 ** Initial author:  marcus.jagemar@gmail.com
 **
 **
 ** DESCRIPTION
 ** -----------
 ** A simple console monitoring program that displays the basic TBan
 ** stuff such as channels and sensors.
 **
 *****************************************************************************/



/***************************************************************
 * System includes
 ***************************************************************/
#include <curses.h>
#include <form.h>
#include <time.h>
#include <signal.h>


/***************************************************************
 * XBan local includes
 ***************************************************************/
#include "tban.h"
#include "mini_ng.h"
#include "ll.h"



/***************************************************************
 * Window constants
 ***************************************************************/
#define SCREEN_CHANNEL_WIDTH   40
#define SCREEN_CHANNEL_HEIGHT  10
#define SCREEN_CHANNEL_X        0
#define SCREEN_CHANNEL_Y        1

#define SCREEN_SENSOR_WIDTH    70

#define SCREEN_TIME_WIDTH   25
#define SCREEN_TIME_HEIGHT   1
#define SCREEN_TIME_X        0
#define SCREEN_TIME_Y       20


/***************************************************************
 * Window identifiers
 ***************************************************************/
static WINDOW *mainwnd;
static WINDOW *screenChannels;
static WINDOW *screenChannelsInfo;
static WINDOW *screenSensors;
static WINDOW *screenTime;
static WINDOW *screenMenu;
static WINDOW *screenCommand;
static WINDOW *screenHelp;


/***************************************************************
 * 
 ***************************************************************/
struct tm *now_tm;


/***************************************************************
 * Currently selected channel/item
 ***************************************************************/
#define SEL_CHANNELINFO   0
#define SEL_CHANNEL       1

int selectedChannel = 0;
int selectedInfo = 0;
int selectedWindow = SEL_CHANNEL;


/***************************************************************
 * 
 ***************************************************************/
int now_sec, now_min, now_hour, now_day, now_wday, now_month, now_year;
time_t now;



/***************************************************************
 * 
 ***************************************************************/
#define SET_SEL_ATTR(index, COMMAND) { \
                              if((selectedWindow==SEL_CHANNELINFO) && (selectedInfo==index)) \
                                wattrset(screenChannelsInfo, A_REVERSE); \
                              COMMAND; \
                              if((selectedWindow==SEL_CHANNELINFO) && (selectedInfo==index)) \
                                wattrset(screenChannelsInfo, A_NORMAL); \
                              }


#define CHECK_NULL(expr, COMMAND) { \
                                    if(expr != NULL) { \
                                      COMMAND; \
                                    } \
                                  } \


/***************************************************************
 * Linked list for command logs
 ***************************************************************/
MetaInformation* cmdLog;
int cmdLog_count=0;

/* Add command to the local log, overwrite if necessary */
void logAddEntry(char* s) {
  char* string = malloc(32+strlen(s));
  sprintf(string, "%d-%02d-%02d:%02d:%02d:%02d %s", now_year, now_month, now_day, now_hour, now_min, now_sec, s);
  
  ll_append(cmdLog, string);
  cmdLog_count++;

  /* If we have passed the upper limit of number messages in the log
   * lets remove the first one */
  if(cmdLog_count >= 9) {
    cmdLog_count--;
    ll_removeindex(cmdLog, 0);
  }
}


/***************************************************************
 *  Get the i:th log entry from the beginning of the log
 ***************************************************************/
char* logGetEntry(int i) {
  MetaNode* m = ll_get(cmdLog, i);
  return m!=NULL ? (char*) m->data : NULL;
}




/***************************************************************
 * 
 ***************************************************************/
void formGetValue(int nr, char* text[], int* result[]) {
  FIELD **field;
  FORM  *my_form;
  int    ch;
  int    i;
  int doloop;

  /* Initialize curses */
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  /* Initialize the fields */
  field = malloc(sizeof(FIELD)*(nr+i));
  for(i=0; i<nr; i++) {
    field[i] = new_field(1, 10, 4+i, 18, 0, 0);
    /* Set field options */
    set_field_back(field[i], A_UNDERLINE);      /* Print a line for the option  */
    field_opts_off(field[i], O_AUTOSKIP);       /* Don't go to next field when this */
                                                /* Field is filled up
                                                 * */
    set_field_type(field[i], TYPE_INTEGER, 5);
  }
  field[i] = NULL;

  /* Create the form and post it */
  my_form = new_form(field);
  post_form(my_form);
  refresh();

  /* Set text */
  for(i=0; i<nr; i++)
    mvprintw(4+i, 10, text[i]);

  refresh();

  /* Clear return values */
  for(i=0; i<nr; i++)
    result[i]=0;
  
  /* Loop through to get user requests */
  doloop=1;
  while((ch = getch()) && (doloop)) {
    switch(ch) {
        case KEY_DOWN:
          /* Go to next field */
          form_driver(my_form, REQ_NEXT_FIELD);
          /* Go to the end of the present buffer leaves nicely at the
           * last character */
          form_driver(my_form, REQ_END_LINE);
          break;
        case KEY_UP:
          /* Go to previous field */
          form_driver(my_form, REQ_PREV_FIELD);
          form_driver(my_form, REQ_END_LINE);
          break;
        case 13:
/*           for(i=0; i<nr; i++) */
/*             result[i]=field */
        case 27:
          doloop=0;
          continue;
        default:
          /* If this is a normal character, it gets printed */  
          form_driver(my_form, ch);
          break;
    }
  }

  /* Un post form and free the memory */
  unpost_form(my_form);
  free_form(my_form);
  for(i=0; i<nr; i++)
    free_field(field[i]);
}


/***************************************************************
 * 
 ***************************************************************/
void screen_init(void) {
  int maxy, maxx;
  mainwnd = initscr();

  /* Check color support */
  if(has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    /* exit(0); */
  }
  start_color();                        /* Start color                  */

  getmaxyx(mainwnd, maxy, maxx);
  printf("maxy=%d maxx=%d \n", maxy, maxx);

  (void) nonl();         /* tell curses not to do NL->CR/NL on output */
  (void) cbreak();       /* take input chars one at a time, no wait for \n */
  (void) noecho();       /* don't echo input */
  
  nodelay(mainwnd, TRUE);
  refresh();
  wrefresh(mainwnd);
  /* Create channel screen */
  screenChannels = newwin(SCREEN_CHANNEL_HEIGHT, SCREEN_CHANNEL_WIDTH, SCREEN_CHANNEL_Y, SCREEN_CHANNEL_X);
  box(screenChannels, ACS_VLINE, ACS_HLINE);
  /* Create channel info screen */
  screenChannelsInfo = newwin(15, SCREEN_CHANNEL_WIDTH, 1+SCREEN_CHANNEL_HEIGHT, 0);
  box(screenChannelsInfo, ACS_VLINE, ACS_HLINE);
  /* Create Sensor screen */
  screenSensors = newwin(maxy-1-10, SCREEN_SENSOR_WIDTH, 1, SCREEN_CHANNEL_WIDTH);
  box(screenSensors, ACS_VLINE, ACS_HLINE);
  /* Create Help screen */
  screenHelp = newwin(maxy-1-10, maxx-SCREEN_SENSOR_WIDTH-SCREEN_CHANNEL_WIDTH, 1, SCREEN_SENSOR_WIDTH+SCREEN_CHANNEL_WIDTH);
  CHECK_NULL(screenHelp, box(screenHelp, ACS_VLINE, ACS_HLINE));
  /* Create time screen */
  screenTime = newwin(SCREEN_TIME_HEIGHT, SCREEN_TIME_WIDTH, 0, maxx-SCREEN_TIME_WIDTH);
  /* box(screenTime, ACS_VLINE, ACS_HLINE); */
  /* Create menu screen */
  screenMenu = newwin(1, maxx-SCREEN_TIME_WIDTH, 0, 0);
  /* box(screenTime, ACS_VLINE, ACS_HLINE); */
  screenCommand = newwin(10, maxx, maxy-10, 0);
  box(screenCommand, ACS_VLINE, ACS_HLINE);
}


/***************************************************************
 * updateChannels
 ***************************************************************/
static void updateChannels(struct TBan* tban) {
  unsigned char pwm, temp;
  unsigned char mode;
  unsigned int  rpm, rpmMax;
  int i;
  int result;
  unsigned char x[6];
  unsigned char y[6];
  int           j;

  curs_set(0);
  mvwprintw(screenChannels,0,1, " CHANNELS ");

/***************************************************************
 * DISPLAY CHANNELS
 ***************************************************************/
  for(i=0; i<4; i++) {
    /* Get all channel data */
    result = tban_getChInfo(tban, i, &rpmMax, &pwm, &temp, &mode);
    rpm = (float) rpmMax * (float) pwm / 100.0;

    /* Draw data */
    if((selectedWindow == SEL_CHANNEL) && (i == selectedChannel)) {
      wattrset(screenChannels, A_REVERSE);
    }
    mvwprintw(screenChannels,1+i,1, "%d % 4d/%04d %3d%% %2.1f", i, rpm, rpmMax, pwm, (float) temp/2.0);
    if((selectedWindow == SEL_CHANNEL) && (i == selectedChannel)) {
      wattrset(screenChannels, A_NORMAL);
    }
  }

/***************************************************************
 * DISPLAY CHANNEL INFORMATION
 ***************************************************************/
  mvwprintw(screenChannelsInfo,0,1, " CHANNEL INFORMATION ");
  tban_getChCurve(tban, selectedChannel, x, y);
  
  /* Channel name and stuff */
  SET_SEL_ATTR(0, mvwprintw(screenChannelsInfo,1,1, "Name: %s", tban->chName[selectedChannel]));
  SET_SEL_ATTR(1, mvwprintw(screenChannelsInfo,2,1, "Desc: %s", tban->chDescr[selectedChannel]));
  SET_SEL_ATTR(2, mvwprintw(screenChannelsInfo,3,1, "Mode: %s", mode==0?"Auto":"Manual"));

  /* Channel data */
  mvwprintw(screenChannelsInfo,4,1, "Ch %d response curve\n", selectedChannel);
  mvwprintw(screenChannelsInfo,5,1, "# : %3s %3s \n", "deg", "pwm");
  for(j=0; j<7; j++) {
    SET_SEL_ATTR(3+j, mvwprintw(screenChannelsInfo,6+j,1, "%d : %.1f %3d \n", j, (float) x[j]/2.0, y[j]));
  }

  /* Display data */
  wrefresh(screenChannels);
  wrefresh(screenChannelsInfo);
}

static void updateSensors(struct TBan* tban) {
  unsigned char temp, rawTemp, cal;
  int i;
  int result;

  curs_set(0);
  mvwprintw(screenSensors,0,1, " SENSOR INFORMATION ");

  for(i=0; i<8; i++) {
    /* Get all channel data */
    result = tban_getdSensorTemp(tban, i, &temp, &rawTemp, &cal);

    /* Draw data */
    mvwprintw(screenSensors,1+i,1, "D%d[%s]: non-cal=%.1fC  Cal.factor=%d%%  temp=%.1fC",
              i,
              tban->dsName[i],
              (float) rawTemp / 2.0,
              cal,
              (float) temp / 2.0);
  }
  for(i=0; i<6; i++) {
    /* Get all channel data */
    result = tban_getaSensorTemp(tban, i, &temp, &rawTemp, &cal);
    /* Draw data */
    mvwprintw(screenSensors,1+8+i,1, "A%d[%s]: non-cal=%.1fC  Cal.factor=%d%%  temp=%.1fC",
              i,
              tban->asName[i],
              (float) rawTemp / 2.0,
              cal,
              (float) temp / 2.0);
  }


  /* Display data */
  wrefresh(screenSensors);
}


static void updateTime(struct TBan* tban) {
  curs_set(0);
  
  init_pair(1, COLOR_CYAN, COLOR_BLACK);
  wattron(screenTime, COLOR_PAIR(1));
  mvwprintw(screenTime,0,0,"%d-%02d-%02d", now_year, now_month, now_day);
  mvwprintw(screenTime,0,12,"%02d:%02d:%02d", now_hour, now_min, now_sec);
  wattroff(screenTime, COLOR_PAIR(1));
  wrefresh(screenTime);
}

static void updateMenu(struct TBan* tban) {
  curs_set(0);
  wattrset(screenMenu, A_REVERSE);
  mvwprintw(screenMenu,0,0,"XBan 0.4 beta");
  wattrset(screenMenu, A_STANDOUT);
  wrefresh(screenMenu);
}


static void updateCommand(struct TBan* tban) {
  int i;
  curs_set(0);
  mvwprintw(screenCommand,0,1," LOG ");
   for(i=0; i<10; i++) {
     char* logEntry;
     logEntry = logGetEntry(i);
     if(logEntry != NULL)
       mvwprintw(screenCommand,1+i,1,"%s", logEntry);
     else
       mvwprintw(screenCommand,1+i,1,".");
   }
  wrefresh(screenCommand);
}


static void updateHelp(struct TBan* tban) {
  if(screenHelp != NULL) {
    int i=0;
    curs_set(0);
    mvwprintw(screenHelp,0,1," HELP ");

    mvwprintw(screenHelp,1+i++,1,"ESC - Quit program");
    mvwprintw(screenHelp,1+i++,1,"RET - Edit param");
    mvwprintw(screenHelp,1+i++,1,"TAB - Move selector");
    mvwprintw(screenHelp,1+i++,1,"q - Move cursor up");
    mvwprintw(screenHelp,1+i++,1,"a - Move cursor down");
    mvwprintw(screenHelp,1+i++,1,"l - Force update");

    wrefresh(screenHelp);
  }
}


void screen_end(void) {
  endwin();
}


void maketime(void) {
  /* Get the current date/time */
  now       = time (NULL);
  now_tm    = localtime (&now);
  now_sec   = now_tm->tm_sec;
  now_min   = now_tm->tm_min;
  now_hour  = now_tm->tm_hour;
  now_day   = now_tm->tm_mday;
  now_wday  = now_tm->tm_wday;
  now_month = now_tm->tm_mon + 1;
  now_year  = now_tm->tm_year + 1900;
}



void monCb(void* ptr, int cur, int max) {
  char string[32];
  (void) ptr;

  sprintf(string, "CB: %d/%d", cur, max);
  logAddEntry(string);
}


void updateScreen(struct TBan* tban) {
  maketime();
  updateChannels(tban);
  updateSensors(tban);
  updateTime(tban);
  updateMenu(tban);
  updateCommand(tban);
  updateHelp(tban);
  refresh();
}


void checkResult(int result) {
  if(result != TBAN_OK) {
    char string[128];
    sprintf(string, "Error when updating: %s [%d]", tban_strerror(result), result);
    logAddEntry(string);
  }
}


/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
void Signal_Handler(int sig) {
  screen_end();
  screen_init();

  switch(sig){
      case SIGWINCH:
        break;          
  }
}


/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
int tbancontrolMonitor(struct TBan* tban) {
  int doloop = 1;
  int i;
  char ch;
  time_t last, cur;

  /* Create the log */
  cmdLog = ll_init();
  logAddEntry("XBan monitor starting");

  /* Change the callback so we can fetch all interesting info in the GUI */
  tban_setProgressCb(tban, monCb, (void*) NULL);

  /* */
  signal(SIGWINCH,Signal_Handler);
  screen_init();
  i=0;
  last=time(NULL);
  updateScreen(tban);
  
  keypad(mainwnd, TRUE);
  
  while (doloop) {
    int result;
    ch = getch();
    switch(ch) {
      
        case KEY_LEFT:
          updateScreen(tban);
          break;
        case KEY_RIGHT:
          updateScreen(tban);
          break;
        case KEY_UP:
        case 'q':
          switch(selectedWindow) {
              case SEL_CHANNEL :
                selectedChannel--;
                if(selectedChannel<0)
                  selectedChannel=3;
                break;
              case SEL_CHANNELINFO :
                selectedInfo--;
                if(selectedInfo<0)
                  selectedInfo=9;
                updateScreen(tban);
                break;
          }
          updateScreen(tban);
          break;
        case KEY_DOWN:
        case 'a':
          switch(selectedWindow) {
              case SEL_CHANNEL :
                selectedChannel++;
                if(selectedChannel>3)
                  selectedChannel=0;
                updateScreen(tban);
                break;
              case SEL_CHANNELINFO :
                selectedInfo++;
                if(selectedInfo>9)
                  selectedInfo=0;
                updateScreen(tban);
                break;
          }
          break;
        case '\t':
          switch(selectedWindow) {
              case SEL_CHANNEL :
                selectedWindow=SEL_CHANNELINFO;
                break;
              case SEL_CHANNELINFO :
                selectedWindow=SEL_CHANNEL;
                break;
          }
          updateScreen(tban);
          break;

          /***************************************************************
           * ENTER
           ***************************************************************/
        case 13:
          switch(selectedWindow) {
              case SEL_CHANNEL : {
                int* value;
                char* text[1] = { "testtest" };
                value = malloc(2);
                formGetValue(1, text, &value);
                free(value);
                updateScreen(tban);
                break;
              }
              case SEL_CHANNELINFO : {
                int* value;
                char* text[] = { "testtest", "ababababa" };
                value = malloc(2);
                formGetValue(2, text, &value);
                free(value);
                updateScreen(tban);
                break;
              }
          }
          updateScreen(tban);
          break;
          /***************************************************************
           * Force update commands
           ***************************************************************/
        case 'l' :
          logAddEntry("Status updated");
          result = tban_queryStatus(tban);
          checkResult(result);
          updateScreen(tban);
          break;
          /***************************************************************
           * ESCAPE
           ***************************************************************/
        case 27:
          doloop=0;
          break;
    }

    /* Update every 10 sec */
    cur = time(NULL);
    if(cur > (last+5)) {
      int result;
      last=cur;
      result = tban_queryStatus(tban);
      if(result == TBAN_OK) {
        logAddEntry("Automatic status update performed ok");
      } else {
        char string[128];
        sprintf(string, "Error when performing status update: %s[%d]", tban_strerror(result), result);
      }
      updateScreen(tban);
    }

    i++;
  }
  screen_end();

  return TBAN_OK;
}
