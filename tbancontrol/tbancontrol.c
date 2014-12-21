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
 ** Filename:        tban.h
 ** Initial author:  marcus.jagemar@gmail.com
 **
 **
 ** DESCRIPTION
 ** -----------
 ** This is command line test program that exercises the TBan library
 ** (libtban).
 **
 ** REVISION HISTORY
 ** ----------------
 **
 ** Date        Comment
 ** =================================================================
 ** 2006-06-12  Inital release (marcus.jagemar@gmail.com)
 ** 2006-06-29  Big cleanup. Removed duplicated code and implemented
 **             some more functions to handle response curves and
 **             hardware information. (marcus.jagemar@gmail.com)
 ** 2006-07-01  Bug fixes in the curve code.
 **             Implemented LED and buzzer commands
 **             (marcus.jagemar@gmail.com)
 ** 2006-07-03  Added dev parameter allowing the user to use different
 **             paths for the TBan device. (marcus.jagemar@gmail.com)
 ** 2006-07-04  First version released (0.1) (marcus.jagemar@gmail.com)
 ** 2006-07-05  Changed commands:
 **             - setchmode for easier usage
 ** 2006-07-06  Added commands:
 **             - getas:     Getting analog sensor value.
 **             - getscfact: Getting and setting sensor scaling factor
 **                          info.
 **             - setscfact: -"-
 **             - getchhyst: Getting channel hysteresis
 **             Misc changes:
 **             - Clean up help text
 **             (marcus.jagemar@gmail.com)
 ** 2006-07-07 Changed command:
 **            - Changed command from queryhw to gethwinfo
 **            - Check if opened before closing
 **            - Added error description to user output
 **             (marcus.jagemar@gmail.com)
 ** 2006-07-10 Added commands:
 **            - getchsens
 **            - setchsens
 **             (marcus.jagemar@gmail.com)
 ** 2006-07-11 Misc changes:
 **            - Improved command getstatus (print unsigned char and hex)
 **            - Improved command getindex  (print hex-data)
 **            - gethwinfo now also prints the app version
 **            (marcus.jagemar@gmail.com)
 ** 2006-07-21 Added commands:
 **            - mgetstat (Dump miniNG status vector)
 **            - mgetch (Print miniNG channel information)
 **            - mgetchcurve (Dump miniNG response curve)
 **            - New gnuplot output option added.
 **            (marcus.jagemar@gmail.com)
 ** 2006-07-25 Added commands:
 **            - mgetchhyst (Print miniNG channel hysteresis)
 **            Misc changes:
 **            - Added callback function that displays progress when
 **              sending commands to TBan or miniNG
 **            (marcus.jagemar@gmail.com)
 ** 2006-07-26 First version after release: tbancontroller-0.3
 ** 2006-07-27 Added commands:
 **            - iterate (loops through commands several times)
 **            Bugfixes
 **            - gethwinfo (corrected channel overtemp)
 **            - getchcurve (added the last curve point)
 **            - setchcurve (added the last curve point)
 **            Misc changes
 **            - mgethwinfo and gethwinfo (Added timebase information
 **              and many other improvements)
 **            (marcus.jagemar@gmail.com)
 ** 2006-08-01 General improvements:
 **            - Reads configuration options from a config file and
 **              display the defined sensor/channel name instead of
 **              index.
 ** 2006-08-17 Removed "mon" command before delivery.
 ** 2006-09-01 First version after release: tbancontroller-0.4
 ** 2006-09-01 Added commands:
 **            - setchhyst (Set channel hysteresis)
 ** 2006-09-27 General improvements:
 **            - added support for BigBG watchdog in gethwinfo
 **            - added support for BigNG output modes
 **            - now returning runtime error if trying to execute BigNG
 **              commands on systems that does not contain that device
 **              type.
 **            - getallch displays output mode for BigNG devices.
 **            Added commands:
 **            - bgetmode
 **            - bgetchsens
 ** 2006-10-02 Added commands:
 **            - ping
 **            - bsetoutmode
 **            General improvements
 **            - gethwinfo Corrected firmware date presentation.
 **            - gethwinfo Corrected warning description error
 **              (displayed wrong text)
 **            - Changed name from bgetmode to bgetoutmode
 ** 2006-10-06 General improvements
 **            - getas now prints analog sensor names correctly
 **            - bgetas now supports special sensor names for BigNG
 **            Added commands
 **            - getallsens (Lists all sensors in the system, no matter
 **              what kind of device they belong to)
 ** 2006-10-08 Added commands
 **            - banner
 **            - separator
 ** 2006-10-11 Removed commands
 **            - getmotion (included in the gethwinfo)
 **            - Sanity check on fakdev files
 **            - getdscfact (included in getallsens)
 **            - getallchcurve
 ** 2006-10-13 Added command
 **            - setmotion (Set motion or blockage detection parameters)
 ** 2006-10-18 Removed command
 **            - bgetchsens (included in getchsens)
 ** 2006-10-23 Added command
 **            - lockwait (The maximum time we will wait for the lock to
 **                        be released)
 **            - lockfile (The lockfile name)
 **            General improvements
 **            - Added functionality to handle multiple applications
 **              using the same hardware. This is managed by using a
 **              lock file mechanism.
 ** 2006-11-21 Added command:
 ** 	       - watchdog (If used all commands will set the watchdog
 ** 	                   before being issued, i.e. there is a maximum
 ** 	                   time of 10 secs to complete the command
 ** 	                   otherwise the TBan device will reset.)
 **
 ** 2007-03-08 Added command (Wainer Vandelli)
 **            - settacho
 **            - scaling factor
 ** 2007-03-11 First version after release: tbancontrol-0.7
 ** 
 *****************************************************************************/

/* Standard includes */
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>

/* TBan library interface */
#include "tban.h"
#include "mini_ng.h"
#include "big_ng.h"
#include "monitor.h"


#define BIGNG_DEVICE_NOT_FOUND  -99

/* If not connected lets try to run anyway. Simulate that the device is
 * open and that we have queried the TBan for data. */

/**********************************************************************/
/* Debug code, should be removed                         [ q1w2e3r4 ] */
/**********************************************************************/
#ifdef DRYRUN
#define tban_queryStatus(tban) TBAN_OK; tban->opened=1
#define miniNG_queryStatus(tban) TBAN_OK; tban->opened=1
#endif
/**********************************************************************/

/*****************************************************************************
 * Output format
 *****************************************************************************/
#define XBAN_FORMAT_STD            0
#define XBAN_FORMAT_STD_HEADER     1
#define XBAN_FORMAT_GNUPLOT        2
#define XBAN_FORMAT_GNUPLOT_HEADER 3

/* Use a global timer to time stamp some commands */
static time_t startTime;

/*****************************************************************************
 *  Global variables used in the main loop.
 *****************************************************************************/
int nrRetries    = 3;
int verbose      = 0;


/*****************************************************************************
 * Global TBan structure
 *****************************************************************************/
struct TBan* tban;


/*****************************************************************************
 * Function declarations
 *****************************************************************************/
static void closeDevice();


/**********************************************************************
 * Name        : local_nanosleep
 * Description : Sleep for some time
 * Arguments   : sec  = Seconds
 *               nano = Nanoseconds
 * Returning   : none
 **********************************************************************/
static void local_nanosleep(long sec, long nano) {
  int             result;
  struct timespec delay;

  /* Set delay settings for nanosleep */
  delay.tv_sec  = (time_t) sec;
  delay.tv_nsec = nano;

  /* If we recieve a signal durin the nanosleep we will return
   * immediately from the function call. In our case this may not be the
   * best thing to do so lets force a delay for at least the time we
   * sepcified. */
  while(1) {
    result = nanosleep(&delay, NULL);
    if (result == 0)
      return;
    continue;
  }
}


/**********************************************************************
 * Name        : readWord
 * Description : Read one word (until reaching a selected stopChar)
 * Arguments   : arr = The destination array
 *               StopChars = The characters that ends the read
 * Returning   :
 **********************************************************************/
static int readWord(FILE* infile, char* arr, char stopChars[]) {
  char c;
  int  i         = 0;
  int  j;
  int  endMarker = 0;
  int  retval    = 0;
  int  leading   = 1;

  /* Read until reaching the stop character */
  for(;;) {
    c = fgetc(infile);
    /* Match the current character to any in the list of supplied ending
       characters. If match then stop the read */
    endMarker = 0;
    for(j=0; j<strlen(stopChars); j++) {
      if(c == (char) stopChars[j]) {
        endMarker = 1;
      }
    }

    if(endMarker == 1)
      break;

    if(feof(infile)) {
      retval = -1;
      break;
    }

    if((c != ' ') && (c != '\t') && (c != '\n')) {
      leading = 0;
    }

    if(leading == 0) {
      arr[i] = c;
      i++;
    }
  }

  arr[i]='\0';
  return retval;
}


/*****************************************************************************
 * Fake settings
 * Used when faking (emulating) the devices so that we can actually get
 * something real from the functions calls without having the TBan
 * connected.
 *****************************************************************************/
char fake_miniNGName[1024];
char fake_tbanName[1024];

static void fake_fillData(unsigned char* data, char* filename) {
  char buf[128];
  char stopChars[] = "\n\t";
  int i;
  FILE* infile;
  
  infile = fopen(filename, "r");
  if(infile == NULL) {
    printf("Unable to open file(%s) => NULL (errno=%d) %s\n",
           filename,
           errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  /* Start reading data from the file */
  i=0;
  while(!feof(infile)) {
    int value;
    int result = readWord(infile, buf, stopChars);
    
    if(result != 0)
      break;

    if(!isdigit((int) buf[0]))
      break;

    /* Perform the conversion */
    errno=0;
    value = atoi(buf);

    if((value==0) && (errno != 0)) {
      break;
    }
    data[i++] = value;
  }
  /* printf("Read %d bytes from file %s (errno=%d) \n", i, filename, errno); */

  (void) fclose(infile);
}



/*****************************************************************************
 * Functions
 *****************************************************************************/
static int parseCmdArgument(char argv[], int* v) {
  int value;

  /* Make sure we have something to parse */
  if((argv == NULL) || (v == NULL))
    return -1;

  /* Since the atoi-function will only convert the numerical part of the
   * string until it finds a non-numerical character we must make sure
   * that the first character is a digit, otherwise we get a 0 (zero)
   * result...*/
  if(!isdigit((int) argv[0]))
    return -2;

  /* Perform the conversion */
  errno=0;
  value = atoi(argv);

  if((value==0) && (errno != 0)) {
    return -3;
  }
  *v = value;
  return TBAN_OK;
}


static int parseCmdArgumentUC(char argv[], unsigned char* v) {
  int value;
  int result;
  result = parseCmdArgument(argv, &value);
  if(result != TBAN_OK)
    return result;

  /* Convert the value to an unsigned char if possible */
  if(value < 0)
    return -5;
  if(value > 255)
    return -6;

  *v = (unsigned char) value;
  return TBAN_OK;
}


void print_numerical_data(unsigned char* thedata, int count) {
  unsigned char In1;
  int           i;
  char          message[20000];
  for (i=0; i< count; i++) {
    In1 = thedata[i];
    sprintf(message,"%3i - 0x%02x  %3u ", i, In1, In1);
    printf("%s\n", message);
  }
}


/* Check if the result from the function called is anything else than
 * TBAN_OK. If so return from the function with the obtained return
 * value. */
#define CHECK_RESULT(COMMAND, STRING) { \
                               int __retryCounter=0; \
                               int result=COMMAND; \
                               while((result!=TBAN_OK) && (__retryCounter<nrRetries)) { \
                                 VERBOSE(printf("Retry command (%d/%d)\n", __retryCounter, nrRetries)); \
                                 result = COMMAND; \
                                 __retryCounter++; \
                               } \
                               if(result != TBAN_OK) { \
                                 printf("RUNTIME ERROR: %s(%d) \"%s\" when: %s\n", tban_strerror(result), result, tban_strerror(result), STRING); \
                                 return result; \
                               } \
                             }

#define CHECK_RESULT_EXIT(COMMAND, STRING) { \
                               int local_result=COMMAND; \
                               if(local_result != TBAN_OK) { \
                                 printf("RUNTIME ERROR: %s(%d) \"%s\" when: %s\n", tban_strerror(local_result), local_result, tban_strerror(local_result), STRING); \
                                 closeDevice(); \
                                 exit(EXIT_FAILURE); \
                               } \
                             }



#define VERBOSE(COMMAND) if(verbose) \
                           COMMAND


#define PRETEND_RUN(COMMAND) { \
                               int __retryCounter=0; \
                               if(!pretendmode) { \
                                 result=COMMAND; \
                                 while((result!=TBAN_OK) && (__retryCounter<nrRetries)) { \
                                   VERBOSE(printf("Retry command (%d/%d)\n", __retryCounter, nrRetries)); \
                                   result = COMMAND; \
                                   __retryCounter++; \
                                 } \
                               } else { \
                                 result=TBAN_OK; \
                               } \
                             }


#define CHECK_NUMBER_ARGUMENTS(tot, nr, cmd) { \
                                          if((nr+1)>=tot) { \
                                            printf("RUNTIME ERROR: Missing argument for %s\n", cmd); \
                                            closeDevice(); \
                                            exit(EXIT_FAILURE); \
                                          } \
                                        }



/**********************************************************************
 * Name        : printSensorInfo
 * Description : Generic function used to print sensor information on
 *               the console. First call to this function could use the
 *               XBAN_FORMAT_STD_HEADER parameter to print the
 *               header. Subsequent calls use the XBAN_FORMAT_STD
 *               printmode for formatted output.
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int printSensorInfo(char* sensorName, int printmode, 
			   unsigned char index, unsigned char temp,
                           unsigned char rawTemp, unsigned char cal)  {
  /* Print the result */
  switch(printmode) {
      case XBAN_FORMAT_STD_HEADER:
        printf("%s\n", sensorName);
        printf("%-2s %-12s %10s %11s %11s\n",
               "#",
               "Name",
               "Raw temp",
               "Cal fact",
               "Temp");
        break;

      case XBAN_FORMAT_STD:
        printf("%-2d %-12s %10.1f  %10.2f  %10.1f\n",
               index,
               sensorName,
               (float) rawTemp / 2.0,
               (float) cal / 100.0,
               (float) temp / 2.0);
        break;

      case XBAN_FORMAT_GNUPLOT_HEADER: {
        struct tm* localTime;
        time_t tt;
        tt = time(NULL);
        localTime = localtime(&tt);
        printf("%d", (int) tt);
        break;
      }

      case XBAN_FORMAT_GNUPLOT: {
        printf(" %.1f", (float)temp/2.0);
        break;
      }
      default:
        printf("Unknown format\n");
  }
}


/**********************************************************************
 * Name        : printSensorInfobigNG
 * Description : Generic function used to print BigNG sensor information on
 *               the console. First call to this function could use the
 *               XBAN_FORMAT_STD_HEADER parameter to print the
 *               header. Subsequent calls use the XBAN_FORMAT_STD
 *               printmode for formatted output.
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int printSensorInfobigNG(char* sensorName, int printmode, 
				unsigned char index, unsigned char temp,
				unsigned char rawTemp, unsigned char cal, 
				unsigned char abscal) {
  /* Print the result */
  switch(printmode) {
      case XBAN_FORMAT_STD_HEADER:
        printf("%s\n", sensorName);
        printf("%-2s %-12s %10s %11s %11s %10s\n",
               "#",
               "Name",
               "Raw temp",
               "Cal fact",
	       "Abs fact",
               "Temp");
        break;

      case XBAN_FORMAT_STD:
        printf("%-2d %-12s %10.1f  %10.2f  %10.1f %10.1f\n",
               index,
               sensorName,
               (float) rawTemp / 2.0,
               (float) cal / 100.0,
	       (float) (1.*abscal-100.0)/2.,
               (float) temp / 2.0);
        break;

      case XBAN_FORMAT_GNUPLOT_HEADER: {
        struct tm* localTime;
        time_t tt;
        tt = time(NULL);
        localTime = localtime(&tt);
        printf("%d", (int) tt);
        break;
      }

      case XBAN_FORMAT_GNUPLOT: {
        printf(" %.1f", (float)temp/2.0);
        break;
      }
      default:
        printf("Unknown format\n");
  }
}


/**********************************************************************
 * Name        : cmdPrintHwStatus
 * Description : Print the HW status of the currently connected
 *               TBan-system. This function will print HW-related data
 *               for TBan/miniNG/BigNG (Different output for different
 *               device) 
 * Arguments   : tban
 * Returning   : -
 **********************************************************************/
static int cmdPrintHwStatus(struct TBan* tban) {
  TBan_deviceType deviceType;
  TBan_appType    appType;
  unsigned char   led, buz, fw_major, fw_minor, fw_year, fw_month, proto, timebase, warn;
  unsigned char   timeConst, maxBorder, incrValue, override[4], rotate[4], current[4];
  unsigned int    len;
  int             i;
  unsigned char   wdenabled, wd;
  unsigned char   overtemp;
  char*           text;
  char*           desc;
  unsigned char ot;

  /* Query the current status from the TBan lib (locally cached) */
  CHECK_RESULT(tban_getHwInfo(tban, &deviceType, &appType, &fw_major, &fw_minor, &fw_year, &fw_month, &proto, &len, &warn, &timebase), "tban_getHwInfo");
  CHECK_RESULT(tban_getLED(tban, &led), "tban_getLED");
  CHECK_RESULT(tban_getBuzzer(tban, &buz), "tban_getBuzzer");
  CHECK_RESULT(tban_getMotionSettings(tban, &timeConst, &maxBorder, &incrValue, override, rotate, current), "tban_getMotionSettings");

  /* BigNG special information */
  if(bigNG_present(tban)) {
    CHECK_RESULT(bigNG_getOvertemp(tban, &overtemp), "bigNG_getOvertemp");
  }

  /* Information for newer devices (firmwares) */
  if((10*fw_major+fw_minor)>=28) {
    CHECK_RESULT(tban_getWatchdog(tban, &wdenabled, &wd), "tban_getWatchdog");
  }

  /* Start printing */
  printf("TBan hardware info:\n");
  printf("TBan type:         %s (0x%x)\n", deviceType==TBAN_DEVICE_TYPE_TBAN?"TBan XL/SL/Classic":deviceType==TBAN_DEVICE_TYPE_BIGNG?"BigNG":"Unknown", deviceType);
  printf("TBan app:          %s (0x%x)\n", appType==TBAN_APP_TYPE_TBAN?"TBan XL/SL/Classic":appType==TBAN_APP_TYPE_BIGNG?"BigNG":"Unknown", appType);
  printf("Firmware version:  %d.%d\n", fw_major, fw_minor);
  printf("Firmware date:     year:20%02d month:%d\n", fw_year, fw_month);
  printf("Protocol version:  %2.2f\n", (float) proto / 10.0);
  printf("Serial buf len:    %d\n", len);
  printf("LED:               %s(%d)\n", led==0?"off":"on", led);
  printf("Buzzer:            %s(%d)\n", buz==0?"off":"on", buz);
  printf("Timebase:          %.1f s\n", (float) timebase/10.0);
  text = tban_strwarn(warn);
  desc = tban_strwarndesc(warn);
  printf("Warning:           %s(%d): %s\n", text!=NULL?text:"unknown", warn, desc!=NULL?desc:"unknown");

  /* Check over temp settings */
  printf("Overtemp:          ");
  for(i=0; i<TBAN_NUMBER_CHANNELS; i++) {
    CHECK_RESULT(tban_getChOvertemp(tban, i, &ot), "tban_getChOvertemp");
    printf("%.1f ", i, (float) ot / 2.0);
  }
  printf("\n");

  /* Watchdog is only for FW newer than 2.8 */
  printf("Watchdog:          ");
  if((10*fw_major+fw_minor)>=28) {
    if(wdenabled != 0)
      printf("%d\n", wd);
    else
      printf("disabled\n");
  } else {
    printf("Not supported, need fw >=2.8\n");
  }

  /* Fan motion detecion */
  printf("Motion estimation:\n");
  printf("Decrement interval: %d \n", timeConst);
  printf("Maximum border:     %d \n", maxBorder);
  printf("Increment value:    %d \n", incrValue);
  printf("Override flags:     ");
  for(i=0; i<4; i++) {
    printf("%d ", override[i]);
  }
  printf("\nRotation flags:     ");
  for(i=0; i<4; i++) {
    printf("%d ", rotate[i]);
  }
  printf("\nCurrent value:      ");
  for(i=0; i<4; i++) {
    printf("%d ", current[i]);
  }
  printf("\n");

  
  /* Only for BigNG */
  if(bigNG_present(tban)) {
    printf("BigNG specific hardware info\n");
    printf("Overtemp:          %d\n", overtemp);
  }

  /* Only for miniNG */
  if(miniNG_present(tban)) {
    unsigned char status, jumper, pot1, pot2, ot1, ot2, timebase;

    /* Query the current status from the TBan lib (locally cached) */
    CHECK_RESULT(miniNG_getHwInfo(tban, &status, &jumper, &pot1, &pot2, &timebase), "miniNG_getHwInfo");
    CHECK_RESULT(miniNG_getChOverTemp(tban, 0, &ot1), "miniNG_getChOverTemp");
    CHECK_RESULT(miniNG_getChOverTemp(tban, 1, &ot2), "miniNG_getChOverTemp");

    /* Print result */
    printf("MiniNG specific hardware info:\n");
    printf("Status:            %d\n", status);
    printf("Warn level:        %s(%d)\n", miniNG_strstat(status & 0x0f), status & 0x0f);
    printf("Jumper:            %d\n", jumper);
    printf("Pot1:              %d\n", pot1);
    printf("Pot2:              %d\n", pot2);
    printf("Over temperature1: %.1f\n", (float) ot1 / 2.0);
    printf("Over temperature2: %.1f\n", (float) ot2 / 2.0);
    printf("Timebase:          %.1f s\n", (float) timebase/10.0);
  }

  return TBAN_OK;
}




static int cmdPrintStatusCont(struct TBan* tban, int index) {
  unsigned char value;

  CHECK_RESULT(tban_getValue(tban, index, &value), "tban_getValue");
  printf("index %d = %d (0x%x)\n", index, value, value);
  return TBAN_OK;
}


/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int printChannelInfo(struct TBan* tban, int printmode, int index, 
			    char* channelName, unsigned int rpmMax, 
			    unsigned char pwm, unsigned char mode,
                            unsigned char temp, unsigned char target,
			    unsigned char targetmode, TBan_deviceType device) {
  unsigned int rpm;
  switch(printmode) {

      case XBAN_FORMAT_STD_HEADER: {
        printf("%s\n", channelName);
        printf("%-2s %12s %10s %10s %10s %10s %10s",
               "#",
               "Name",
               "RPM",
               "Max-RPM",
               "PWM",
               "temp",
               "Mode");
        /* BigNG specific stuff */
        if(device == TBAN_DEVICE_TYPE_BIGNG) {
          printf("%12s %10s %13s", 
		 "Outputmode",
		 "Target",
		 "Targetmode");
	}

        printf("\n");
        break;
      }
        
      case XBAN_FORMAT_STD: {
        rpm = (int) ( (float) rpmMax * (float) pwm / 100.0);
        printf("%-2d %12s %10d %10d %10d %10.1f %10s",
               index,
               channelName,
               rpm,
               rpmMax,
               pwm,
               (float)temp/2.0,
               mode==0?"auto":"manual");
        /* BigNG specific stuff */
        if(device == TBAN_DEVICE_TYPE_BIGNG) {
          int result;
          unsigned char outmode[4];
          result = bigNG_getOutputMode(tban, outmode);
          printf("%12s %10.1f %13d",
                 outmode[index]==BIGNG_OUTPUT_MODE_ANALOG ? "analog" :
                 outmode[index]==BIGNG_OUTPUT_MODE_PWM ? "PWM" : "unknown",
		 (float)target/2.0,
		 targetmode);
        }
        printf("\n");
        break;
      } /* case */


      case XBAN_FORMAT_GNUPLOT_HEADER: {
        time_t currentTime;
        struct tm* localTime;
        time_t tt;
        tt = time(NULL);
        localTime = localtime(&tt);
        printf("%d ", (int) tt);
        break;
      }
  
      case XBAN_FORMAT_GNUPLOT: {
        rpm = (int) ( (float) rpmMax * (float) pwm / 100.0);
        printf("%d %d %d %.1f ", rpm, rpmMax, pwm, (float)temp/2.0);
	if(device == TBAN_DEVICE_TYPE_BIGNG) {
	  printf("%.1f",(float)target/2.0);
	}
        break;
      } /* case */
  } /* switch */
}


/**********************************************************************
 * Name        : cmdPrintAllChInfo
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int cmdPrintAllChInfo(struct TBan* tban, int printmode) {
  int           j;
  unsigned char pwm, temp;
  unsigned char mode;
  unsigned int  rpm, rpmMax;
  unsigned char bigngPresent;
  unsigned char target, targetmode;

  /* */
  bigngPresent = bigNG_present(tban);

  /* Print header (if possible) */
  switch(printmode) {
      case XBAN_FORMAT_STD :
        printChannelInfo(tban, XBAN_FORMAT_STD_HEADER, 
			 0, bigngPresent==BIGNG_PRESENT ? "BigNG channels" : "TBan channels",
                         0, 0, 0, 0, 0, 0,
                         bigngPresent==BIGNG_PRESENT ? TBAN_DEVICE_TYPE_BIGNG : TBAN_NO_DEVICE);
        break;
      case XBAN_FORMAT_GNUPLOT :
        printChannelInfo(tban, XBAN_FORMAT_GNUPLOT_HEADER, 0, 0, 0, 0, 0, 
			 0, 0, 0, -1);
        break;
  }

  /* TBan standard channels */
  for(j=0; j<TBAN_NUMBER_CHANNELS; j++) {
    if(bigngPresent==BIGNG_PRESENT) {
      CHECK_RESULT(bigNG_getChInfo(tban, j, &rpmMax, &pwm, &temp, &mode, &target, &targetmode), "bigNGgetChInfo");
      printChannelInfo(tban, printmode, j, tban->chName[j], rpmMax, pwm, mode, temp, target, targetmode, bigngPresent==BIGNG_PRESENT ? TBAN_DEVICE_TYPE_BIGNG : TBAN_DEVICE_TYPE_TBAN);
    } else {
      CHECK_RESULT(tban_getChInfo(tban, j, &rpmMax, &pwm, &temp, &mode), "tban_getChInfo");    
      printChannelInfo(tban, printmode, j, tban->chName[j], rpmMax, pwm, mode, temp, 0, 0, bigngPresent==BIGNG_PRESENT ? TBAN_DEVICE_TYPE_BIGNG : TBAN_DEVICE_TYPE_TBAN);
    }
  } /* for */

  /* miniNG specific channels */
  if(miniNG_present(tban)) {
    unsigned char temp, calTemp, cal;
    if(printmode == XBAN_FORMAT_STD) {
      printChannelInfo(tban, XBAN_FORMAT_STD_HEADER, 0, "miniNG channels", 0, 0, 0,0, 0, 0, TBAN_NO_DEVICE);
    }

    for(j=0; j<MINI_NG_NUMBER_CHANNELS; j++) {
      CHECK_RESULT(miniNG_getChRpm(tban, j, (unsigned char*) &rpm, (unsigned char*) &rpmMax), "miniNG_getChRpm");
      CHECK_RESULT(miniNG_getaSensorTemp(tban, j, &temp, &calTemp, &cal), "miniNG_getaSensorTemp");
      pwm = (unsigned char) (100.0 * (float) rpm / (float) rpmMax);
      printChannelInfo(tban, printmode, j, tban->miniNG.chName[j], rpmMax, pwm, 0, temp, 0, 0, TBAN_NO_DEVICE);
    }
  }
  
  /* Print footer if needed */
  if(printmode == XBAN_FORMAT_GNUPLOT) {
    printf("\n");
  }

  return TBAN_OK;
}


/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int cmdPrintChInfo(struct TBan* tban, int channel, int printmode) {
  unsigned char pwm, temp, mode;
  unsigned int  rpm, rpmMax;

  /* Fetch the data */
  CHECK_RESULT(tban_getChInfo(tban, channel, &rpmMax, &pwm, &temp, &mode), "tban_getChInfo");
  rpm = (int) ( (float) rpmMax * (float) pwm / 100.0);
  switch(printmode) {
      case XBAN_FORMAT_STD:
        printf("Ch%d (%s) : %d/%d rpm  pwm=%d temp=%.1f C (%d=%s) \n",
               channel,
               tban->chName[channel],
               rpm,
               rpmMax,
               pwm,
               (float)temp/2.0,
               mode,
               mode==0?"A":"M");
        break;
      case XBAN_FORMAT_GNUPLOT: {
        struct tm* localTime;
        time_t tt;
        tt = time(NULL);
        localTime = localtime(&tt);
        printf("%d %d %d %d %.1f\n", (int) tt, rpm, rpmMax, pwm, (float)temp/2.0);
        break;
      }
      default:
        printf("Unknown format\n");
  }

  return TBAN_OK;
}


/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int cmdPrintBigNGChInfo(struct TBan* tban, int channel, int printmode) {
  unsigned char pwm, temp, mode, target, targetmode;
  unsigned int  rpm, rpmMax;

  /* Fetch the data */
  CHECK_RESULT( bigNG_getChInfo(tban, channel, &rpmMax, 
				&pwm, &temp, &mode,
				&target, &targetmode), 
		"bigNG_getChInfo");

  rpm = (int) ( (float) rpmMax * (float) pwm / 100.0);
  switch(printmode) {
      case XBAN_FORMAT_STD:
        printf("Ch%d (%s) : %d/%d rpm  pwm=%d temp=%.1f C (%d=%s) tar=%.1f tarmode=%d \n",
               channel,
               tban->chName[channel],
               rpm,
               rpmMax,
               pwm,
               (float)temp/2.0,
               mode,
               mode==0?"A":"M",
	       (float)target/2.0,
	       targetmode);
        break;
      case XBAN_FORMAT_GNUPLOT: {
        struct tm* localTime;
        time_t tt;
        tt = time(NULL);
        localTime = localtime(&tt);
        printf("%d %d %d %d %.1f %.1f\n", (int) tt, rpm, rpmMax, pwm, (float)temp/2.0, (float)target/2.);
        break;
      }
      default:
        printf("Unknown format\n");
  }

  return TBAN_OK;
}


static int cmdPrintMiniNGChInfo(struct TBan* tban, int channel, int printmode) {
  unsigned char temp, calTemp, rpm, maxRpm, cal;

  /* Fetch the data */
  CHECK_RESULT(miniNG_getaSensorTemp(tban, channel, &temp, &calTemp, &cal), "miniNG_getaSensorTemp");
  CHECK_RESULT(miniNG_getChRpm(tban, channel, &rpm, &maxRpm), "miniNG_getChRpm");
  switch(printmode) {
      case XBAN_FORMAT_STD:
        printf("Ch %d : %d/%d temp=%.1f C ; Calibrated temp=%.1f C\n", channel, rpm, maxRpm, (float)temp/2.0, (float)calTemp/2.0);
        break;
      case XBAN_FORMAT_GNUPLOT: {
        struct tm* localTime;
        time_t tt;
        tt = time(NULL);
        localTime = localtime(&tt);
        printf("%d %d %d %.1f %.1f\n", (int) tt, rpm, maxRpm, (float)temp/2.0, (float)calTemp/2.0);
        break;
      }
      default:
        printf("Unknown format\n");
  }

  return TBAN_OK;
}


static int cmdPrintMiniNGChHyst(struct TBan* tban, int channel) {
  unsigned char hysteresis;

  CHECK_RESULT(miniNG_getChHysteresis(tban, channel, &hysteresis), "miniNG_getChHysteresis");
  printf("Ch %d : hysteresis=%d\n", channel, hysteresis);

  return TBAN_OK;
}


static int cmdPrintMiniNGRespCurve(struct TBan* tban, unsigned char ch) {
  unsigned char x[5];
  unsigned char y[5];
  int           j;

  /* Print the result */
  CHECK_RESULT(miniNG_getChCurve(tban, ch, x, y), "miniNG_getChCurve");

  printf("Ch %d response curve\n", ch);
  printf("# : %3s %3s \n", "deg", "pwm");
  for(j=0; j<5; j++) {
    printf("%d : %3d %3d \n", j, x[j], y[j]);
  }
  printf("miniNG curve: ");
  for(j=0; j<5; j++) {
    printf("%d %d ", x[j], y[j]);
  }
  printf("\n");
  return TBAN_OK;
}


static int cmdPrintChHyst(struct TBan* tban, int channel) {
  unsigned char hysteresis;

  CHECK_RESULT(tban_getChHysteresis(tban, channel, &hysteresis), "tban_getChHysteresis");
  printf("Ch %d (%s) : hysteresis=%d\n", channel, tban->chName[channel], hysteresis);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : cmdPrintChSens
 * Description : Print a matrix describing how sensors are assigned to
 *               channels.
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int cmdPrintChSens(struct TBan* tban, int channel) {
  unsigned char dsens, asens, bngsens;
  int i;

  if(bigNG_present(tban)) {
    CHECK_RESULT_EXIT(bigNG_getChSensAssignment(tban, channel, &dsens, &asens, &bngsens), "bigNG_getChSensAssignment");
  } else {
    CHECK_RESULT(tban_getChSensAssignment(tban, channel, &dsens, &asens), "tban_getChSensAssignment");
  }

  printf("Ch %d (%s)\n", channel, tban->chName[channel]);

  /* Print header */
  printf("%6s      %8s %8s", "sensor", "digital", "analog");
  if(bigNG_present(tban)) {
    printf("%8s", "BigNG");
  }
  printf("\n");

  /* Print data matrix */
  for(i=0; i<8; i++) {
    unsigned char binVal = 1 << i;
    /* Print standars digital and analog sensors */
    printf("%d(%3d)      %8s %8s",
           i, binVal,
           (dsens & binVal)?"yes":"no",
           (asens & binVal)?"yes":"no");

    /* Additional column for BigNG */
    if(bigNG_present(tban)) {
      printf(" %8s",
             (i>3) ? "-" : (bngsens & binVal) ? "yes":"no");
    }
    printf("\n");
  }

  /* Print tail (sum) */
  printf("SUM:        %8d %8d",
         dsens,
         asens);
  if(bigNG_present(tban)) {
    printf(" %8d", bngsens);
  }
  printf("\n");
      
  return TBAN_OK;
}



/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int cmdPrintDsInfo(struct TBan* tban, int index, int printmode) {
  unsigned char temp, rawTemp, cal;

  /* Print the result */
  CHECK_RESULT(tban_getdSensorTemp(tban, index, &temp, &rawTemp, &cal), "tban_getdSensorTemp");

  /* Print header and then data */
  printSensorInfo("TBan standard digital sensor", XBAN_FORMAT_STD_HEADER, 0, 0, 0, 0);
  printSensorInfo(tban->dsName[index], printmode, index, temp, rawTemp, cal);

  return TBAN_OK;
}


/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int cmdPrintAsInfo(struct TBan* tban, int index, int printmode) {
  unsigned char temp, rawTemp, cal;

  /* Ask the TBan */
  CHECK_RESULT(tban_getaSensorTemp(tban, index, &temp, &rawTemp, &cal), "tban_getaSensorTemp");

  /* Print header and then data */
  printSensorInfo("TBan standard analog sensor", XBAN_FORMAT_STD_HEADER, 0, 0, 0, 0);
  printSensorInfo(tban->asName[index], printmode, index, temp, rawTemp, cal);

  return TBAN_OK;
}


/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int cmdPrintAsInfoBigNG(struct TBan* tban, int index, int printmode) {
  unsigned char temp, rawTemp, cal, abscal;

  /* Ask the TBan */
  CHECK_RESULT(bigNG_getaSensorTemp(tban, index, &temp, &rawTemp, &cal, &abscal), "bigNG_getaSensorTemp");
  
  /* Print header and then data */
  printSensorInfobigNG("BigNG additional analog sensor", XBAN_FORMAT_STD_HEADER, 0, 0, 0, 0, 0);
  printSensorInfobigNG(tban->bigNG.asName[index], printmode, index, temp, rawTemp, cal, abscal);

  return TBAN_OK;
}

/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int cmdPrintDsInfoBigNG(struct TBan* tban, int index, int printmode) {
  unsigned char temp, rawTemp, cal, abscal;

  /* Ask the TBan */
  CHECK_RESULT(bigNG_getdSensorTemp(tban, index, &temp, &rawTemp, &cal, &abscal), "bigNG_getdSensorTemp");
  
  /* Print header and then data */
  printSensorInfobigNG("BigNG digital sensor", XBAN_FORMAT_STD_HEADER, 0, 0, 0, 0, 0);
  printSensorInfobigNG(tban->dsName[index], printmode, index, temp, rawTemp, cal, abscal);

  return TBAN_OK;
}

/**********************************************************************
 * Name        : cmdPrintAllSensors
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int cmdPrintAllSensors(struct TBan* tban, int printmode) {
  unsigned char temp, rawTemp, cal, abscal;
  int i;

  /* TBan generic analog sensors */
  printSensorInfo("TBan standard analog sensors:", XBAN_FORMAT_STD_HEADER, 0, 0, 0, 0);
  for(i=0; i<TBAN_NUMBER_ANALOG_SENSORS; i++) {
    CHECK_RESULT(tban_getaSensorTemp(tban, i, &temp, &rawTemp, &cal), "tban_getaSensorTemp");
    printSensorInfo(tban->asName[i], printmode, i, temp, rawTemp, cal);
  }

  if(!bigNG_present(tban)){
    /* TBan digital sensors */
    printSensorInfo("TBan standard digital sensors:", XBAN_FORMAT_STD_HEADER, 0, 0, 0, 0);
    for(i=0; i<TBAN_NUMBER_DIGITAL_SENSORS; i++) {
      CHECK_RESULT(tban_getdSensorTemp(tban, i, &temp, &rawTemp, &cal), "tban_getdSensorTemp");
      printSensorInfo(tban->dsName[i], printmode, i, temp, rawTemp, cal);
    }
  }else{
    /* BigNG digital sensors */

    printSensorInfobigNG("BigNG digital sensors:", XBAN_FORMAT_STD_HEADER, 0, 0, 0, 0, 0);
    for(i=0; i<TBAN_NUMBER_DIGITAL_SENSORS; i++) {
      CHECK_RESULT(bigNG_getdSensorTemp(tban, i, &temp, &rawTemp, &cal, &abscal), "bigNG_getdSensorTemp");
      printSensorInfobigNG(tban->dsName[i], printmode, i, temp, rawTemp, cal, abscal);
    }
    
    /* BigNG analog sensors */
    
    printSensorInfobigNG("BigNG analog sensors:", XBAN_FORMAT_STD_HEADER, 0, 0, 0, 0, 0);
    for(i=0; i<BIGNG_NUMBER_ADDITIONAL_ANALOG_SENSORS; i++) {
      CHECK_RESULT(bigNG_getaSensorTemp(tban, i,&temp, &rawTemp, &cal, &abscal), "bigNG_getaSensorTemp");
      printSensorInfobigNG(tban->bigNG.asName[i], printmode, i, temp, rawTemp, cal, abscal);
    }
  }

  /* MiniNG specific sensors */
  if(miniNG_present(tban)) {
    printSensorInfo("MiniNG analog sensors:", XBAN_FORMAT_STD_HEADER, 0, 0, 0, 0);
    for(i=0; i<MINI_NG_NUMBER_ANALOG_SENSORS; i++) {
      CHECK_RESULT(miniNG_getaSensorTemp(tban, i,&temp, &rawTemp, &cal), "miniNG_getaSensorTemp");
      printSensorInfo(tban->miniNG.asName[i], printmode, i, temp, rawTemp, cal);
    }
  }

  return TBAN_OK;
}

  
/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int cmdPrintRespCurve(struct TBan* tban, unsigned char ch) {
  unsigned char x[7];
  unsigned char y[7];
  int           j;

  CHECK_RESULT(tban_getChCurve(tban, ch, x, y), "tban_getChCurve");

  printf("Ch %d (%s) response curve\n", ch, tban->chName[ch]);
  printf("# : %3s %3s \n", "deg", "pwm");
  for(j=0; j<7; j++) {
    printf("%d : %.1f %3d \n", j, (float) x[j]/2.0, y[j]);
  }
  printf("TBan curve: ");
  for(j=0; j<7; j++) {
    printf("%d %d ", x[j]/2, y[j]);
  }
  printf("\n");
  return TBAN_OK;
}





/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static int openDevice(struct TBan* tban) {
#ifndef LINUX
  tban->opened=1;
#else
  int result;
  /* Open the device  */
  result = tban_open(tban);
  if(result != TBAN_OK) {
    return result;
  }
#endif
  return TBAN_OK;
}


/**********************************************************************
 * Name        : 
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
void progressUpdater(void* ptr, int cur, int max) {
  int mode = * ((int*) ptr);
  
  /* Check if the user wants verbose output */
  switch (mode) {
      case 0:
        printf(".");
        break;
      case 1:
        printf("%.f%% ", 100.0 * (float) cur / (float) max);
        break;
  }
  if(cur == max)
    printf("\n");
  fflush(stdout);
}



/**********************************************************************
 * Name        : closeDevice
 * Description : This function closes the device and cleans up
 *               everything. It also tries to remove the locking file if
 *               it is present.
 * Arguments   : -
 * Returning   : -
 **********************************************************************/
static void closeDevice() {
  unsigned char wdenabled;
  unsigned char wd;
  int           result;

  /* If the watchdog was set lets disable it since we will leave the
   * program soon */
  VERBOSE(printf("Check if watchdog is enabled: "));
  VERBOSE(fflush(stdout));
  result = tban_getWatchdog(tban, &wdenabled, &wd);
  VERBOSE(printf("ok\n"));
  if(result == TBAN_OK) {
    printf("wde=%d wd=%d\n", wdenabled, wd);
    if(wdenabled == 1) {
      VERBOSE(printf("Disable watchdog: "));
      VERBOSE(fflush(stdout));
      int result = tban_disableWatchdog(tban);
      if(result != TBAN_OK) {
        printf("RUNTIME ERROR: %s(%d) \"%s\" when: %s\n", tban_strerror(result), result, tban_strerror(result), "tban_disableWatchdog");
      } else {
        VERBOSE(printf("ok\n"));
      }
    }
  } else if(result != TBAN_FW_TOO_OLD) {
    printf("RUNTIME ERROR: %s(%d) \"%s\" when: %s\n", tban_strerror(result), result, tban_strerror(result), "tban_getWatchdog");
  }

  /* Close device */
  VERBOSE(printf("Closing device: "));
  VERBOSE(fflush(stdout));
  (void) tban_close(tban);
  VERBOSE(printf("ok\n"));

  /* Remove file lock if device is locked by this process */
  if(tban->locked == 1)
    if(tban_unlock(tban) != TBAN_OK) {
      printf("Removing lock file failed\n");
      exit(EXIT_SUCCESS);
    }

  /* Remove the structure */
  VERBOSE(printf("Removing allocated memory: "));
  VERBOSE(fflush(stdout));
  (void) tban_free(tban);
  if(tban != NULL)
    free(tban);
  VERBOSE(printf("ok\n"));
}


/**********************************************************************
 * Name        : catchIntSignal
 * Description : Catches the INT (interrupt CTRL-c) signal.
 * Arguments   : -
 * Returning   : -
 **********************************************************************/
static void catchIntSignal(int sig_num) {
  /* re-set the signal handler again to catch INT, for next time */
  signal(SIGINT, catchIntSignal);
  
  /* Close the device */
  closeDevice();
  exit(EXIT_SUCCESS);
}



/**********************************************************************
 * Name        : printHelp
 * Description : 
 * Arguments   : 
 * Returning   : -
 **********************************************************************/
static void printHelp() {
  printf("usage: tbancontrol [<dev> <device_path>] [ <command> [arguments] ]... \n");

  printf("Global commands:\n");
  printf("  help                         \tDisplay this help\n");
  printf("  verbose                      \tMake all commands verbose\n");
  printf("  iterate <nr> <delay>         \tIterate all commands following <nr> of times and <delay> seconds between each iteration\n");
  printf("  pretend                      \tDont try to call the TBan, just simulate\n");
  printf("  dev                          \tChange the default device (/dev/ttyUSB0). Must be located at the\n");
  printf("                               \tbeginning of the command line\n");
  printf("  gnuplot                      \tChange the output from getch and mgetch  to fit gnuplot \n");
  printf("  retry <nr>                   \tDecide how many retries to perform. \n");
  printf("  separator                    \tPrint a separator line.\n");
  printf("  banner <word>                \tPrint a banner line containing the word.\n");
  printf("  sleep <sec>                  \tSleep for some seconds\n");
  printf("  lockwait <sec>               \tWait for device lock to be released. Standard value is 10s.\n");
  printf("  lockfile <filename>          \tLock filename to be used.\n");
  printf("  watchdog                     \tReset the TBan if no response within 10 seconds. Firmware >=2.8 needed\n");

  printf("Maintenance commands:\n");
  printf("  resethw                      \tReset the TBan HW\n");
  printf("  gethwinfo                    \tPrint hardware info (TBan/BigNG/miniNG)\n");
  printf("  ping <mask>                  \tPing sensor\n");
  
  printf("Setter commands:\n");
  printf("  setchmode <ch1>...<ch4>        \tSet the channel mode for all channels (1=manual, 0=auto) \n");
  printf("  setchpwm <ch> <mode>           \tSet pwm for a channel \n");
  printf("  setchinitpwm <ch> <mode>       \tSet inital pwm for a channel \n");
  printf("  setchsens <ch> <dsens> <asens> \tSet sensor assignment (sens values are binary masks for sensors)\n");
  printf("  setchcurve <ch> <temp pwm>*6   \tset response curve for channel (temp-pwm value-pair must be\n");
  printf("                                 \tdefined 6 times) \n");
  printf("  setpwmfreq <freq>              \tSet the PWM frequency\n");
  printf("  setbuz <0|1>                   \tSet the buzzer status\n");
  printf("  setled <0|1>                   \tSet the LED status\n");
  printf("  setscfact <nr> <factor>        \tChange scaling factor \n");
  printf("  setchhyst <nr> <hysteresis>    \tChange the hysteresis settings \n");
  printf("  setmotion <lo> <hi> <err>      \tChange the motion (blockage) detection settings \n");
  printf("  settacho <ch1>...<ch4>         \tSet the blockage recognition mode (1=off, 0=on) \n");

  printf("Getter commands:\n");
  printf("  getstat                      \tDump the whole status vector\n");
  printf("  getchmode <ch>               \tGet the channel mode \n");
  printf("  getch <ch>                   \tGet channel info\n");
  printf("  getchhyst <ch>               \tGet channel hysteresis\n");
  printf("  getchsens <ch>               \tGet channel sensor assignment\n");  
  printf("  getallch                     \tGet info for all channels (Tban/BigNG/miniNG)\n");
  printf("  getallsens                   \tGet info for all sensors (Tban/BigNG/miniNG)\n");
  printf("  getpwmfreq                   \tGet the PWM frequency\n");
  printf("  getchcurve <ch>              \tGet response curve\n");
  printf("  getindex <index>             \tGet value from the TBan status vector (See index definition\n");
  printf("                               \ttable in the TBan documentation) \n");
  printf("  getds <sensor_index>         \tGet digital sensor temp \n");
  printf("  getas <sensor_index>         \tGet analog sensor temp \n");
  
  printf("miniNG specific commands:\n");
  printf("  mgetstat                     \tDump the whole status vector\n");
  printf("  mgetchhyst <ch>              \tGet channel hysteresis\n");
  printf("  mgetch <ch>                  \tGet channel info\n");
  printf("  mgetchcurve <ch>             \tGet response curve\n");
  printf("  msetchcurve <ch> <temp pwm>*6\tset response curve for channel (temp-pwm value-pair must be\n");
  
  printf("bigNG specific commands:\n");
  printf("  bgetoutmode                  \tGet the output mode\n");
  printf("  bsetoutmode <ch1>...<ch4>    \tSet the output mode (%d=PWM %d=analog) \n", BIGNG_OUTPUT_MODE_PWM, BIGNG_OUTPUT_MODE_ANALOG);
  printf("  bgetas <sensor_index>        \tGet analog sensor temp \n");
  printf("  bgetstat                     \tDump the whole BigNG status vector\n");
  printf("  bgetds <sensor_index>        \tGet digital sensor temp \n");
  printf("  bsetchsens <ch> <dsens> <asens> <bngsens> \tSet sensor assignment (sens values are binary masks for sensors)\n");
  printf("  bsetscfactas <nr> <factor>       \tChange scaling factor \n");
  printf("  bsetabsscfactas <nr> <factor>    \tChange absolute scaling factor analog\n");
  printf("  bsetabsscfactds <nr> <factor>    \tChange absolute scaling factor digital\n");
  printf("  bsettargettemp <nr> <target>       \tChange target temperature\n");
  printf("  bsettargetmode <nr> <mode>       \tChange target mode \n");
  printf("  bgetch <ch>                   \tGet channel info\n");

  printf("\nCommand params:\n");
  printf("* All commands are 0-indexed, meaning that channel/sensor 1 should be addressed by 0.\n");
  printf("\n");
  
  printf("General information:\n");
  printf("* The arguments are parsed from left to right and executed sequentially. This means that many commands can\n");
  printf("  be stacked together in the same application call.\n");
  printf("* Keep in mind that sleep() is occasionally needed since the TBan device seems to take some time to handle\n");
  printf("  some requests, for example after a resethw. \n");
  printf("* resethw can sometimes be useful when the TBan seems to hang.\n");
}


static void parseArguments(int argc, char* argv[]) {
  int i;
  int result;
  int pretendmode  = 0;
  int printformat  = 0;

  /***************************************************************
   * Select if watchdog should be used or not
   ***************************************************************/
  int watchdogmode = 0;

  /***************************************************************
   * Vars for iterate command
   * Start value: we only want to repeat those commands occurring after
   *              the iterate command was found..
   * Nr:    The number of times to iterate
   * Delay: The number of seconds to sleep between each iterations
   *        (after all commands has been executed)
   * ***************************************************************/
  int iterateStart = 1;
  int iterateCount = 0;
  int iterateNr=0;
  int iterateDelay=0;

  /* Display help text if no arguments are given */
  if(argc == 1) {
    printHelp();
    return;
  }

  /* So that we know when we start the program */
  startTime = time(NULL);

  /***************************************************************
   * set the INT (Ctrl-C) signal handler to 'catch_int'
   ***************************************************************/
  signal(SIGINT, catchIntSignal);

  /***************************************************************
   * Allocate and init the TBan device. This will basically create
   * the TBan struct and fill it with iseful data.
   ***************************************************************/
  tban = malloc(sizeof(struct TBan));
  CHECK_RESULT_EXIT(tban_init(tban, "/dev/ttyUSB0"), "tban_init");
  CHECK_RESULT_EXIT(bigNG_init(tban), "bigNG_init");
  CHECK_RESULT_EXIT(miniNG_init(tban), "miniNG_init");


  /***************************************************************
   * Set application default values
   ***************************************************************/
  (void) tban_configureLockTimeout(tban, 10);

  /***************************************************************
   * Start parsing the arguments (commands) from the command line.
   ***************************************************************/
  do {
    for(i=iterateStart; i<argc; i++) {
      result  = -1;
    
      /* Let the user choose the device to use */
      if(strcmp(argv[i], "dev")==0) {
        /* This must be at the beginning of the command line, otherwise it
         * will take no effect so tell the user if this is not the case.*/
        if(i != 1) {
          printf("dev argument must be at the beginning of the command line \n");
          closeDevice();
          exit(EXIT_FAILURE);
        }
        CHECK_NUMBER_ARGUMENTS(argc,i, "dev");
        i++;
        /* Free any previously allocated device name */
        if(tban->deviceName != NULL)
          free(tban->deviceName);
        tban->deviceName = malloc(strlen(argv[i])+1);
        (void) strcpy(tban->deviceName, argv[i]);
        continue; /* Continue with the for loop, no need for the rest */
      }

      /***************************************************************
       * These commands are treated somewhat special. They don't need the
       * TBan device to be opened and can use the continue keyword after
       * the functionality has been executed since they only affect the
       * behaviour of the application, no error checking is needed as
       * the other commands below.
       ***************************************************************/

      /* Fake the device using data from a file when updating the status
       * vector */
      if(strcmp(argv[i], "fakedev")==0) {
/*         if(i != 1) { */
/*           printf("fakedev argument must be at the beginning of the command line \n"); */
/*           closeDevice(); */
/*           exit(EXIT_FAILURE); */
/*         } */
        CHECK_NUMBER_ARGUMENTS(argc,i+1, "fakedev");

        /* Copy the TBan and miniNG status string */
        (void) strcpy(fake_tbanName, argv[++i]);
        (void) strcpy(fake_miniNGName, argv[++i]);
        
        /* Fill the TBan struct buffer with data from the file */
        fake_fillData(tban->buf, fake_tbanName);
        fake_fillData(tban->miniNG.buf, fake_miniNGName);

        /* Check if anyone else is using the device right now */
        (void) tban_configureLockFile(tban, "/tmp/.Xban.lock");
        if(tban_checkIfDeviceUsed(tban) != TBAN_OK) {
          printf("Device already in use\n");
          closeDevice();
          exit(EXIT_SUCCESS);
        }

        /* Fake that the device is opened */
        tban->opened=1;

        /* Make sure that the data is at least reasonably sane */
        if(tban_present(tban) != TBAN_OK) {
          printf("Corrupted data in the TBan fakedev files\n");
          closeDevice();
          exit(EXIT_FAILURE);
        }
        
        /* Try to read the configuration file. If we fail and the error
         * code is "file not found" just skip it and continue
         * anyway. Then we will use standard names. If we get more
         * serious errors then we bail out. */
        result = tban_parseConfig(tban, ".tban.conf");
        if((result != TBAN_OK) && (errno!=2)) {
          printf("Error when parsing config file\n");
          closeDevice();
          exit(EXIT_FAILURE);
        }

        continue; /* Continue with the for loop, no need for the rest */
      }

      /* lockwait */
      if(strcmp(argv[i], "lockwait")==0) {
        unsigned int timeout;
        VERBOSE(printf("* Wait for a maximum number of seconds .\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i, "lockwait");
        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &timeout),  "lockwait: Parsing argument 1(locktimeout)");
        tban_configureLockTimeout(tban, timeout);
        continue; /* Continue with the for loop, no need for the rest */
      }
      
      /* lockfile */
      if(strcmp(argv[i], "lockfile")==0) {
        VERBOSE(printf("* Configure the lock file to use.\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i, "lockfile");
        ++i;
        tban_configureLockFile(tban, argv[i]);
        continue; /* Continue with the for loop, no need for the rest */
      }
      
      /* help */
      if(strcmp(argv[i], "help")==0) {
        printHelp();
        continue; /* Continue with the for loop, no need for the rest */
      }

      /* verbose */
      if(strcmp(argv[i], "verbose")==0) {
        verbose=1;
        printf("Verbose mode selected\n");
        continue; /* Continue with the for loop, no need for the rest */
      }

      /* pretend */
      if(strcmp(argv[i], "pretend")==0) {
        pretendmode=1;
        VERBOSE(printf("* Pretend mode selected, no commands will be sent to the TBan device.\n"));
        continue; /* Continue with the for loop, no need for the rest */
      }

      /* sleep */
      if(strcmp(argv[i], "sleep")==0) {
        unsigned char interval;
        CHECK_NUMBER_ARGUMENTS(argc,i, "sleep");
        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &interval),  "sleep: Parsing argument 1(sec)");
        VERBOSE(printf("* sleep for %ds\n", interval));
        (void) sleep(interval);
        continue; /* Continue with the for loop, no need for the rest */
      }

      /* watchdog */
      if(strcmp(argv[i], "watchdog")==0) {
        watchdogmode=1;
        VERBOSE(printf("* Watchdog mode selected. The TBan will be reset if no response for more than 10 seconds.\n"));
        continue; /* Continue with the for loop, no need for the rest */
      }

      /* iterate */
      if(strcmp(argv[i], "iterate")==0) {
        /* Look for two arguments */
        CHECK_NUMBER_ARGUMENTS(argc,i+1, "iterate");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], &iterateNr),  "iterate: Parsing argument 1(iterations)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], &iterateDelay),  "iterate: Parsing argument 2(interval)");
        VERBOSE(printf("* iterate for %d number of time, delay %d between each iteration\n", iterateNr, iterateDelay));
        iterateStart=i+1;
        continue; /* Continue with the for loop, no need for the rest */
      }

      /* retry */
      if(strcmp(argv[i], "retry")==0) {
        /* Look for one argument */
        CHECK_NUMBER_ARGUMENTS(argc,i+1, "retry");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], &nrRetries),  "retry: Parsing argument 1(nrRetries)");
        continue; /* Continue with the for loop, no need for the rest */
      }

      /* separator */
      if(strcmp(argv[i], "separator")==0) {
        printf("\n");
        continue; /* Continue with the for loop, no need for the rest */
      }

      /* banner */
      if(strcmp(argv[i], "banner")==0) {
        /* Look for one argument */
        CHECK_NUMBER_ARGUMENTS(argc,i+1, "banner");
        printf("--------------------------------------------------\n");
        printf("%s\n", argv[i+1]);
        printf("--------------------------------------------------\n");
        i++;
        continue; /* Continue with the for loop, no need for the rest */
      }

      /* gnuplot : Change the output format to fit gnuplot  */
      if(strcmp(argv[i], "gnuplot")==0) {
        VERBOSE(printf("* Adjust output to fit gnuplot.\n"));
        printformat=XBAN_FORMAT_GNUPLOT;
        continue; /* Continue with the for loop, no need for the rest */
      }

      /***************************************************************
       * All commands that are located below this line needs the TBan
       * device to be opened. There are also an error check at the bottom
       * of this section of the code. This will check the result from the
       * function calls made by the commands below.
       ***************************************************************/

      /* Make sure that the device is opened, but let the user select the
       * device fist so this statement must be located after the "dev" */
      if(!tban->opened) {
        int result;
        int __retryCounter=0;

        /* Open the device if noone else is using it */
        VERBOSE(printf("Opening device %s\n", tban->deviceName));
        CHECK_RESULT_EXIT(tban_setProgressCb(tban, progressUpdater, (void*) &verbose), "Failed to set the progress callback function");
        result = openDevice(tban);
        if(result == TBAN_ALREADY_IN_USE) {
          printf("Device already in use by another program\n");
          closeDevice();
          exit(EXIT_FAILURE);
        }
        CHECK_RESULT_EXIT(result, "Failed to open the device");

        /* Try to read the configuration file. If we fail and the error
         * code is "file not found" just skip it and continue
         * anyway. Then we will use standard names. If we get more
         * serious errors then we bail out. */
        result = tban_parseConfig(tban, ".tban.conf");
        if((result != TBAN_OK) && (errno!=2)) {
          printf("Error when parsing config file\n");
          closeDevice();
          exit(EXIT_FAILURE);
        }

        /* Query the device once to make sure that everything works */
        VERBOSE(printf("* Performing initial TBan query\n"));
        result = tban_queryStatus(tban);
        while((result != TBAN_OK) && (__retryCounter<nrRetries)) {
          result = tban_queryStatus(tban);
          VERBOSE(printf("  No contact with TBan, reconnecting [%d/%d]\n", __retryCounter, nrRetries));
          __retryCounter++;
          local_nanosleep(5, 0);
        }
        /* No contact with TBan causes program exit */
        if(result != TBAN_OK) {
          printf("  No contact with TBan\n");
          closeDevice();
          exit(EXIT_SUCCESS);
        }
        VERBOSE(printf("  TBan query ok\n"));

	/* If the module is a BigNG we have to load also the second
	 * status vector */
	__retryCounter=0;

	if(bigNG_present(tban)) {
	  /* Load the second status array */
          VERBOSE(printf("* Performing initial bigNG query\n"));
          result=bigNG_queryStatus(tban);
	  while((result != TBAN_OK) && (__retryCounter<nrRetries)) {
	    result = bigNG_queryStatus(tban);
	    VERBOSE(printf("  No contact with bigNG, reconnecting [%d/%d]\n", __retryCounter, nrRetries));
	    __retryCounter++;
	    local_nanosleep(5, 0);
	  }
          /* No contact with bigNG causes program exit */
          if(result != TBAN_OK) {
            printf("  No contact with bigNG\n");
            closeDevice();
            exit(EXIT_SUCCESS);
          }
          VERBOSE(printf("bigNG query ok\n"));
        }

        /* Try to get the status vector from the miniNG (if present). In
         * the case it is not present we will get an empty vector back
         * (NULL) so it is pretty easy to see if a miniNG is present. */
        {
          int stat;
          VERBOSE(printf("* Performing initial miniNG query\n"));
          stat = miniNG_queryStatus(tban);
          if(stat == TBAN_OK) {
            VERBOSE(printf("  miniNG present\n"));
          } else {
            VERBOSE(printf("  miniNG not present status=%d\n", stat));
          }
        }
      }

      /***************************************************************
       * Kick on the watchdog before each command is sent. No command is
       * allowed to take more than 10 seconds, then it is regarded as an
       * error.
       ***************************************************************/
      if(watchdogmode == 1) {
        if(tban_checkFw(tban, 28) == TBAN_FW_TOO_OLD) {
          printf("Firmware too old. Need 2.8 or later\n");
          watchdogmode=0;
        } else {
          VERBOSE(printf("* Kicking the watchdog\n"));
          PRETEND_RUN(tban_kickWatchdog(tban));
        }
      }
      
      /***************************************************************
       * Commands
       ***************************************************************/

      /* Get hardware information */
      if(strcmp(argv[i], "gethwinfo")==0) {
        VERBOSE(printf("* gethwinfo\n"));
        PRETEND_RUN(cmdPrintHwStatus(tban));
      }

      /* getstat */
      if(strcmp(argv[i], "getstat")==0) {
        VERBOSE(printf("* getstat\n"));
        print_numerical_data(tban->buf, 285);
        result=TBAN_OK;
      }

      /* Get the value for a specific index in the status vector  */
      if(strcmp(argv[i], "getindex")==0) {
        int index;
        VERBOSE(printf("* getindex\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i, "getindex");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &index), "getindex Parsing argument #1(index)");
        PRETEND_RUN(cmdPrintStatusCont(tban, index));
      }

      /* Get information for all channels */
      if(strcmp(argv[i], "getallch")==0) {
        VERBOSE(printf("* getallch\n"));
        PRETEND_RUN(cmdPrintAllChInfo(tban, printformat));
      }

      /* Get digital sensor information  */
      if(strcmp(argv[i], "getds")==0) {
        int index;
        VERBOSE(printf("* getds\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i, "getds");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &index), "getds Parsing argument #1(sensor index)");
        PRETEND_RUN(cmdPrintDsInfo(tban, index, printformat));
      }

      /* Get analog sensor information  */
      if(strcmp(argv[i], "getas")==0) {
        int index;
        VERBOSE(printf("* getas\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i, "getas");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &index), "getas Parsing argument #1(sensor index)");
        PRETEND_RUN(cmdPrintAsInfo(tban, index, printformat));
      }

      /* Get information for all sensors connected to the system  */
      if(strcmp(argv[i], "getallsens")==0) {
        VERBOSE(printf("* getallsens\n"));
        PRETEND_RUN(cmdPrintAllSensors(tban, printformat));
      }

      /* resethw */
      if(strcmp(argv[i], "resethw")==0) {
        VERBOSE(printf("* resethw\n"));
        PRETEND_RUN(tban_resetHardware(tban));
      }

      /* Channel mode */
      if(strcmp(argv[i], "setchmode")==0) {
        unsigned char modeMask;
        int ch0, ch1, ch2, ch3;
        VERBOSE(printf("* setchmode\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+3, "setchmode");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch0), "setchpwm Parsing argument #1(ch1_mode)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch1), "setchpwm Parsing argument #1(ch2_mode)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch2), "setchpwm Parsing argument #1(ch3_mode)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch3), "setchpwm Parsing argument #1(ch4_mode)");
        modeMask = ch0 + 2*ch1 + 4*ch2 + 8*ch3;
        PRETEND_RUN(tban_setChMode(tban, modeMask));
      }

      /* Set PWM */
      if(strcmp(argv[i], "setchpwm")==0) {
        int ch, pwm;
        VERBOSE(printf("* setchpwm\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+1, "setchpwd");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch),  "setchpwm Parsing argument #1(channel)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &pwm), "setchpwm Parsing argument #2(pwm)");
        PRETEND_RUN(tban_setChPwm(tban, ch, pwm));
      }

      /* Set initial PWM */
      if(strcmp(argv[i], "setchinitpwm")==0) {
        int ch, pwm;
        VERBOSE(printf("* setchinitpwm\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+1,"setchinitpwm");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch),  "setchinitpwm Parsing argument #1(channel)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &pwm), "setchinitpwm Parsing argument #2(pwm)");
        PRETEND_RUN(tban_setChInitValue(tban, ch, pwm));
      }

      /* Set sensor assigment */
      if(strcmp(argv[i], "setchsens")==0) {
        int ch, dsens, asens;
        VERBOSE(printf("* setchsens\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+2,"setchsens");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch),  "setchsens Parsing argument #1(channel)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &dsens), "setchsens Parsing argument #2(digital sensor)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &asens), "setchsens Parsing argument #2(analog sensor)");
        PRETEND_RUN(tban_setChSensAssignment(tban, ch, dsens, asens));
      }

      /* Set PWM frequency */
      if(strcmp(argv[i], "setpwmfreq")==0) {
        int freq;
        VERBOSE(printf("* setpwmfreq\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"setpwmfreq");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &freq),  "setchinitpwm: Parsing argument #1(frequency)");
        PRETEND_RUN(tban_setPwmFreq(tban, freq));
      }

      /* Set scaling factory */
      if(strcmp(argv[i], "setscfact")==0) {
        int nr, factor;
        VERBOSE(printf("* setscfact\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+1,"setscfact");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &nr),  "setscfact: Parsing argument #1(nr)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &factor),  "setscfact: Parsing argument #2(factor)");
        PRETEND_RUN(tban_setSensorScaleFact(tban, nr, factor));
      }

      /* Set buzzer status */
      if(strcmp(argv[i], "setbuz")==0) {
        int stat;
        VERBOSE(printf("* setbuz\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"setbuz");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &stat),  "setbuz Parsing argument #1(status)");
        PRETEND_RUN(tban_setBuz(tban, stat));
      }

      /* Set hysteresis */
      if(strcmp(argv[i], "setchhyst")==0) {
        int nr, hysteresis;
        VERBOSE(printf("* setchhyst\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+1,"setchhyst");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &nr),  "setchhyst: Parsing argument #1(nr)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &hysteresis),  "setchhyst: Parsing argument #2(factor)");
        PRETEND_RUN(tban_setChHysteresis(tban, nr, hysteresis));
      }
      
      /* Set hysteresis */
      if(strcmp(argv[i], "setmotion")==0) {
        unsigned char lo, hi, err;
        VERBOSE(printf("* setmotion\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+1,"setmotion");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &lo),  "setmotion: Parsing argument #1(lo)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &hi),  "setmotion: Parsing argument #1(hi)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &err),  "setchhyst: Parsing argument #2(err)");
        PRETEND_RUN(tban_setMotion(tban, lo, hi, err));
      }
      
      /* Set LED status */
      if(strcmp(argv[i], "setled")==0) {
        int stat;
        VERBOSE(printf("* setled\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"setled");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &stat),  "setled Parsing argument #1(status)");
        PRETEND_RUN(tban_setLED(tban, stat));
      }

      /* Set response curve */
      if(strcmp(argv[i], "setchcurve")==0) {
        int ch, j;
        unsigned char temp[7], pwm[7];
        VERBOSE(printf("* setchcurve\n"));

        /* Build the argument list */
        CHECK_NUMBER_ARGUMENTS(argc,i,"setchcurve");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch), "setchinitpwm Parsing argument #1(channel)");
        VERBOSE(printf("Operating on channel=%d\n", ch));
        for(j=0; j<7; j++) {
          CHECK_NUMBER_ARGUMENTS(argc,i+1,"setchcurve");
          CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &(temp[j])), "setchcurve: Parsing argument #1(x-temp)");
          CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &(pwm[j])),  "setchcurve: Parsing argument #2(y-pwm)");
          VERBOSE(printf("temp=%4d ; pwm=%3d\n", temp[j], pwm[j]));
        }
        /* Make sure that the user sets the first temp=0 and the last
         * pwm=100 */
        if((temp[0] != 0) || (pwm[6] != 100)) {
          printf("First temp must be 0 and last pwm must be 100\n");
          result=-1;
        } else {
          /* Vector is correct */
          VERBOSE(printf("Sending the curve to TBan\n"));
          PRETEND_RUN(tban_setChCurve(tban, ch, temp, pwm));
        }
      }

      /* Set tacho */
      if(strcmp(argv[i], "settacho")==0) {
	unsigned char modeMask;
        int ch0, ch1, ch2, ch3;
        VERBOSE(printf("* settacho\n"));
	CHECK_NUMBER_ARGUMENTS(argc,i+3, "settacho");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch0), "settacho Parsing argument #1(ch1_mode)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch1), "settacho Parsing argument #1(ch2_mode)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch2), "settacho Parsing argument #1(ch3_mode)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch3), "settacho Parsing argument #1(ch4_mode)");
        modeMask = ch0 + 2*ch1 + 4*ch2 + 8*ch3;
	PRETEND_RUN(tban_setTacho(tban, modeMask));
      }

      /* Get information for a specific channel */
      if(strcmp(argv[i], "getch")==0) {
        int channel;
        VERBOSE(printf("* getch\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"getch");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &channel), "getch: Parsing argument #1(channel)");
        PRETEND_RUN(cmdPrintChInfo(tban, channel, printformat));
      }

      /* Get channel hysteresis */
      if(strcmp(argv[i], "getchhyst")==0) {
        int channel;
        VERBOSE(printf("* getchhyst\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"getchhyst");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &channel), "getchhyst: Parsing argument #1(channel)");
        PRETEND_RUN(cmdPrintChHyst(tban, channel));
      }

      /* Get channel sensor assignment  */
      if(strcmp(argv[i], "getchsens")==0) {
        int channel;
        VERBOSE(printf("* getchsens\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"getchsens");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &channel), "getchsens: Parsing argument #1(channel)");
        PRETEND_RUN(cmdPrintChSens(tban, channel));
      }

      /* Get response curve for a specific channel */
      if(strcmp(argv[i], "getchcurve")==0) {
        unsigned char ch;
        VERBOSE(printf("* getchcurve\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"getchcurve");
        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &(ch)),  "getchcurve: Parsing argument #1(channel)");
        PRETEND_RUN(cmdPrintRespCurve(tban, ch));
      }

      /* Channel mode */
      if(strcmp(argv[i], "getchmode")==0) {
        unsigned char ch;
        unsigned char mode, startMode;
        VERBOSE(printf("* getchmode\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"getchmode");
        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &(ch)),  "getchmode: Parsing argument 1(channel)");
        PRETEND_RUN(tban_getChMode(tban, ch, &mode, &startMode));
        printf("Ch %d (%s) : Current mode=%s ; Startup mode=%s\n",
               ch,
               tban->chName[ch],
               mode==0?"A":"M",
               startMode==0?"A":"M");
      }

      /* Get PWM frequency */
      if(strcmp(argv[i], "getpwmfreq")==0) {
        unsigned char freq;
        VERBOSE(printf("* getpwmfreq\n"));
        PRETEND_RUN(tban_getPwmFreq(tban, &freq));
        printf("PWM freq=%d\n", freq);
      }

      /* ping: Ping digital sensors */
      if(strcmp(argv[i], "ping")==0) {
        unsigned char sensormask;

        VERBOSE(printf("* ping\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"ping");
        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &sensormask), "ping: Parsing argument #1(sensormask)");

        CHECK_RESULT_EXIT(tban_ping(tban, sensormask), "tban_ping");
        result=TBAN_OK;
      }



      /***************************************************************
       * bigNG commands
       ***************************************************************/
      /* bgetoutmode: Get output mode */
      if(strcmp(argv[i], "bgetoutmode")==0) {
        unsigned char ch[4];
        int index;
        VERBOSE(printf("* bgetoutmode\n"));

        if(bigNG_present(tban)) {
          CHECK_RESULT_EXIT(bigNG_getOutputMode(tban, ch), "bigNG_getOutputMode");
          for(index=0; index<4; index++)
            printf("Channel %d = %s\n",
                   index,
                   ch[index]==BIGNG_OUTPUT_MODE_ANALOG ? "analog" : ch[index]==BIGNG_OUTPUT_MODE_PWM ? "pwm" : "unknown");
          result=TBAN_OK;
        } else {
          result=BIGNG_DEVICE_NOT_FOUND;
        }
      }


      /* bsetoutmode: Set output mode for all channels at the same time */
      if(strcmp(argv[i], "bsetoutmode")==0) {
        unsigned char mode0, mode1, mode2, mode3, modeMask;
        VERBOSE(printf("* bsetoutmode\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+3,"bsetoutmode");

        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &mode0), "bsetoutmode Parsing argument #1(ch0_mode)");
        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &mode1), "bsetoutmode Parsing argument #2(ch1_mode)");
        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &mode2), "bsetoutmode Parsing argument #3(ch2_mode)");
        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &mode3), "bsetoutmode Parsing argument #4(ch3_mode)");
        modeMask = mode0 + 2*mode1 + 4*mode2 + 8*mode3;

        if(bigNG_present(tban)) {
          CHECK_RESULT_EXIT(bigNG_setOutputMode(tban, modeMask), "bigNG_setOutputMode");
          result=TBAN_OK;
        } else {
          result=BIGNG_DEVICE_NOT_FOUND;
        }
      }

      /* Set BigNG sensor assigment */
      if(strcmp(argv[i], "bsetchsens")==0) {
        int ch, dsens, asens, bngsens;
        VERBOSE(printf("* bsetchsens\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+3,"bsetchsens");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch),  "bsetchsens Parsing argument #1(channel)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &dsens), "bsetchsens Parsing argument #2(digital sensor)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &asens), "bsetchsens Parsing argument #2(analog sensor)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &bngsens), "bsetchsens Parsing argument #3(BigNG additional analog sensor)");
        PRETEND_RUN(bigNG_setChSensAssignment(tban, ch, dsens, asens, bngsens));
      }

      
      /* Get analog sensor information  */
      if(strcmp(argv[i], "bgetas")==0) {
        int index;
        VERBOSE(printf("* bgetas\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i, "bgetas");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &index), "bgetas Parsing argument #1(sensor index)");
        PRETEND_RUN(cmdPrintAsInfoBigNG(tban, index, printformat));
      }
      
      /* Get digital sensor information */
      if(strcmp(argv[i], "bgetds")==0) {
        int index;
        VERBOSE(printf("* bgetds\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i, "bgetds");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &index), "bgetds Parsing argument #1(sensor index)");
        PRETEND_RUN(cmdPrintDsInfoBigNG(tban, index, printformat));
      }

      /* Get fan information */
      if(strcmp(argv[i], "bgetch")==0) {
        int channel;
        VERBOSE(printf("* bgetch\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"bgetch");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &channel), "bgetch: Parsing argument #1(channel)");
        PRETEND_RUN(cmdPrintBigNGChInfo(tban, channel, printformat));
      }
              
      /* bgetstat */
      if(strcmp(argv[i], "bgetstat")==0) {
        VERBOSE(printf("* bgetstat\n"));
        print_numerical_data(tban->bigNG.buf, 285);
        result=TBAN_OK;
      }

      /* Set scaling factor for analog sens*/
      if(strcmp(argv[i], "bsetscfactas")==0) {
        int nr, factor;
        VERBOSE(printf("* bsetscfactas\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+1,"bsetscfactas");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &nr),  "bsetscfactas: Parsing argument #1(nr)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &factor),  "bsetscfactas: Parsing argument #2(factor)");
        PRETEND_RUN(bigNG_setAsScalingFact(tban, nr, factor));
      }

      /* Set target mode*/
      if(strcmp(argv[i], "bsettargetmode")==0) {
        int nr, factor;
        VERBOSE(printf("* bsettargetmode\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+1,"bsettargetmode");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &nr),  "bsettargetmode: Parsing argument #1(nr)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &factor),  "bsettargetmode: Parsing argument #2(factor)");
        PRETEND_RUN(bigNG_setChTargetMode(tban, nr, factor));
      }

      /* Set target temp*/
      if(strcmp(argv[i], "bsettargettemp")==0) {
        int nr, factor;
        VERBOSE(printf("* bsettargettemp\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+1,"bsettargettemp");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &nr),  "bsettargettemp: Parsing argument #1(nr)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &factor),  "bsettargettemp: Parsing argument #2(factor)");
        PRETEND_RUN(bigNG_setChTargetTemp(tban, nr, factor));
      }

      /* Set absolute scaling factor for analog sens The factor input is
       * the desidered value times 2, plus 100.  This is the internal
       * BigNG representation of the abs scaling factor, indeed in this
       * way also negative factors are holded with a positive
       * number. However providing the input in this format is not
       * really nice. So this function should be changed in order to
       * accept the desidered value (positive or negative) as input and
       * to convert it to the internal value. *Should be easy to do */
      if(strcmp(argv[i], "bsetabsscfactas")==0) {
        int nr, factor;
        VERBOSE(printf("* bsetabsscfactas\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+1,"bsetabsscfactas");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &nr),  "bsetabsscfactas: Parsing argument #1(nr)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &factor),  "bsetabsscfactas: Parsing argument #2(factor)");
        PRETEND_RUN(bigNG_setAsAbsScalingFact(tban, nr, factor));
      }

      /* Set absolute scaling factor for digital sens The factor input
       * is the desidered value times 2, plus 100.  This is the internal
       * BigNG representation of the abs scaling factor, indeed in this
       * way also negative factors are holded with a positive
       * number. However providing the input in this format is not
       * really nice. So this function should be changed in order to
       * accept the desidered value (positive or negative) as input and
       * to convert it to the internal value. *Should be easy to do */
      if(strcmp(argv[i], "bsetabsscfactds")==0) {
        int nr, factor;
        VERBOSE(printf("* bsetabsscfactds\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i+1,"bsetabsscfactds");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &nr),  "bsetabsscfactds: Parsing argument #1(nr)");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &factor),  "bsetabsscfactds: Parsing argument #2(factor)");
        PRETEND_RUN(bigNG_setDsAbsScalingFact(tban, nr, factor));
      }

      
      /***************************************************************
       * miniNG commands
       ***************************************************************/
      /* mggetstat: Dump the whole miniNG status vector */
      if(strcmp(argv[i], "mgetstat")==0) {
        VERBOSE(printf("* mgetstat\n"));
        print_numerical_data(tban->miniNG.buf, 128);
        result=TBAN_OK;
      }

      /* mgetch: Get information for a specific miniNG channel */
      if(strcmp(argv[i], "mgetch")==0) {
        unsigned char ch;
        VERBOSE(printf("* mgetch\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"mgetch");
        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &ch), "mgetch: Parsing argument #1(channel)");
        PRETEND_RUN(cmdPrintMiniNGChInfo(tban, ch, printformat));
      }
    
      /* mgetchcurve: Get channel response curve */
      if(strcmp(argv[i], "mgetchcurve")==0) {
        unsigned char ch;
        VERBOSE(printf("* mgetchcurve\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"mgetchcurve");
        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &ch), "mgetchcurve: Parsing argument #1(channel)");
        PRETEND_RUN(cmdPrintMiniNGRespCurve(tban, ch));
      }
    
      /* mgetchhyst: Get channel hysteresis */
      if(strcmp(argv[i], "mgetchhyst")==0) {
        unsigned char ch;
        VERBOSE(printf("* mgetchhyst\n"));
        CHECK_NUMBER_ARGUMENTS(argc,i,"mgetchhyst");
        CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &ch), "mgetchhyst: Parsing argument #1(channel)");
        PRETEND_RUN(cmdPrintMiniNGChHyst(tban, ch));
      }
    
      /* Set response curve */
      if(strcmp(argv[i], "msetchcurve")==0) {
        unsigned char ch;
        unsigned char temp[5], pwm[5];
        int  j;

        VERBOSE(printf("* msetchcurve\n"));

        /* Build the argument list */
        CHECK_NUMBER_ARGUMENTS(argc,i,"msetchcurve");
        CHECK_RESULT_EXIT(parseCmdArgument(argv[++i], (int*) &ch), "msetchinitpwm Parsing argument #1(channel)");
        VERBOSE(printf("Operating on channel=%d\n", ch));
        for(j=0; j<5; j++) {
          CHECK_NUMBER_ARGUMENTS(argc,i+1,"msetchcurve");
          CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &(temp[j])), "msetchcurve: Parsing argument #1(x-temp)");
          CHECK_RESULT_EXIT(parseCmdArgumentUC(argv[++i], &(pwm[j])),  "msetchcurve: Parsing argument #2(y-pwm)");
          VERBOSE(printf("temp=%4d ; pwm=%3d\n", temp[j], pwm[j]));
        }
        /* Make sure that the user sets the first temp=0 and the last
         * pwm=100 */
        if((temp[0] != 0) || (pwm[4] != 100)) {
          printf("First temp must be 0 and last pwm must be 100\n");
          result=-1;
        } else {
          /* Vector is correct */
          VERBOSE(printf("Sending the curve to miniNG\n"));
          PRETEND_RUN(miniNG_setChCurve(tban, ch, temp, pwm));
        }
      }

      /***************************************************************
       * Monitor mode
       ***************************************************************/
      /* mon : */
/*       if(strcmp(argv[i], "mon")==0) { */
/*         VERBOSE(printf("* mon\n")); */
/*         PRETEND_RUN(tbancontrolMonitor(tban)); */
/*       } */

    
      /***************************************************************
       * Command execution checks. Make sure that we have actually
       * executed a command and that it succeeded, otherwise lets tell the
       * user.
       ***************************************************************/

      /* BigNG connection test*/
      if(result == BIGNG_DEVICE_NOT_FOUND) {
        printf("RUNTIME ERROR: BigNG device not present in your system while trying to execute command \"%s\"\n", argv[i]);
        closeDevice();
        exit(EXIT_FAILURE);
      }
      
      /* If the result is still -1 we know that none of the commands have
       * executed so we must have an unknown argument. So lets tell the
       * user and exit. */
      if(result == -1) {
        printf("PARSE ERROR: Unknown argument: %s\n", argv[i]);
        closeDevice();
        exit(EXIT_FAILURE);
      }

      /***************************************************************
       * Check the result from the commands. When being here we know that
       * we have executed a command but we need to make sure that it was
       * a success. Otherwise just return a message to the user and
       * exit. Try to be nice and return TBan stuff (close and
       * free). Probably not needed since we exit the program after this
       * is finished but you never know.
       ***************************************************************/
      /* Check result before getting the next command */
      if(result != TBAN_OK) {
        printf("COMMUNICATION ERROR: Command returned: %s (%d) \"%s\"\n", tban_strerror(result), result, tban_strerrordesc(result));
        closeDevice();
        exit(EXIT_FAILURE);
      } else {
        VERBOSE(printf("Result checked ok\n"));
      }

    } /* for */

    /* Prepare for iteration check */
    iterateCount++;
    sleep(iterateDelay);
  } while(iterateCount < iterateNr);



  /***************************************************************
   * All commands executed ok. Lets close the driver.
   ***************************************************************/
  /* Close the devide */
  if(tban->opened==1) {
    closeDevice();
  }

}



int main(int argc, char* argv[]) {
  /* Parse argument from the command line */
  parseArguments(argc, argv);
  return 0;
}

