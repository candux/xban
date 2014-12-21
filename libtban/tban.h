/*****************************************************************************
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Gener Public License for more details.
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
 ** (Communication functions taken from the original XBan project)
 **
 ** 
 ** REQUIREMENTS
 ** ------------
 ** Needs theFDTI kernel module:
 ** -> Device Drivers 
 ** -> USB Support 
 ** -> USB Serial Converter support 
 **    <M> USB FDTI Single Port Serial Driver
 **
 **    
 ** DESCRIPTION
 ** -----------
 ** The t-balancer library handling commonication with mcubed
 ** T-Balancer HW connected through a USB<->Serial interface.
 **
 ** The initial code was taken from the xban project, same as
 ** communication functions and converters.
 **
 ** In short this library provides functions to manage the TBan
 ** communication, such as:
 ** - init
 ** - open
 ** - close
 ** - free
 ** Additionally there are many more functions that are designed to help
 ** application developers to directly communicate with the T-Balancer
 ** hw.
 **
 ** Normally an application should perform the following function
 ** calls in sequence to successfully query the TBan-device for
 ** LED-status (example)
 ** 1. tban_init
 **    Creates the TBan structure
 ** 2. tban_open
 **    Open the device we want to use
 ** 3. tban_queryStatus
 **    Send the query command to the device and reads the result. The
 **    result from this command is stored in the local cache and can be
 **    read by any of the tban_get-functions. Please bear in mind that
 **    additional calls to the tban_queryStatus function needs to be
 **    made if update of the local cache is needed.
 ** 4. tban_getLED
 **    Obtain the result from the locally stored cache.
 ** 5. tban_close
 ** 6. tban_free
 ** 
 ** 
 ** 
 ** REVISION HISTORY
 ** ----------------
 ** 
 ** Date        Comment
 ** =================================================================
 ** 2006-06-12  Inital release (marcus.jagemar@gmail.com)
 ** 2006-06-14  Added timer functions so that we dont ask the TBan hw
 **             every time we want to query the status.
 **             Better error handling (marcus.jagemar@gmail.com)
 ** 2006-06-28  Added functions for
 **             - reading response curve
 **             - resetting the device
 **             - setting channel pwm manually
 **             - setting operational mode
 **             (marcus.jagemar@gmail.com)
 ** 2006-06-30  Added functions for channel mode
 **             Changed function names to be more consistent
 **             (marcus.jagemar@gmail.com)
 ** 2006-07-01  Bug fixes in the response curve code.
 **             Implemented LED and buzzer commands
 **             (marcus.jagemar@gmail.com)
 ** 2006-07-04  First version released (0.1) (marcus.jagemar@gmail.com)
 ** 2006-07-05  Fixed bugs:
 **             - Digital sensors showed too high temp value. Should be
 **               half the previous value. 
 **             Added functions:
 **             - tban_getaSensorTemp: analog sensor getter 
 **             - tban_getScalingFactor: sensor scaling factor getter.
 **             - tban_setSensorScaleFact: sensor scaling factor setter.
 **             - tban_getChHysteresis: channel hysteresis getter.
 **             Renamed functions:
 **             - tban_getFanInfo => tban_getChInfo
 **             - tban_openPort   => tban_open
 **             - tban_closePort  => tban_close
 **             (marcus.jagemar@gmail.com)
 ** 2006-07-06  Added functions:
 **             - tban_getChSensAssignment
 **             Changed functions:
 **             - tban_getHwInfo
 **             (marcus.jagemar@gmail.com)
 ** 2006-07-07  Fixed bugs:
 **             - Raw temp mapping for digital sensor had wong offset.
 **             - Max RPM mapping had wrong offset
 **             Changed functions:
 **             - tban_getHwInfo (ser_buf_len needs an int)
 **             Added function:
 **             - tban_strerrordesc (Displays a more detailed error description)
 **             General improvements
 **             - Cleaned up error messages
 **             (marcus.jagemar@gmail.com)
 ** 2006-07-10  Added functions:
 **             - tban_setChSensAssignment
 **             (marcus.jagemar@gmail.com)
 ** 2006-07-11  Fixed bugs:
 **             - tban_getHwInfo return wrong firmware major version
 **             - tban_getHwInfo return wrong firmware date
 ** 2006-07-11  Misc changes:
 **             - tban_getHwInfo now also returns the app type. This is
 **               better for determining the type of device.
 ** 2006-07-21  General improvements:
 **             - Support for miniNG added.
 **             Fixed bugs:
 **             - tban_readData caused buffer overrun when receiving a
 **               longer buffer than expected.
 **             - tban_getHwInfo switched place of year and month.
 ** 2006-07-25  General improvements:
 **             - Added a callback mechanism that allows the application
 **               to specify a function to be called when large amount
 **               of data is transferred to the TBan.
 ** 2006-07-26 First version after release: libtban-0.3
 **            - tban_gethwinfo (Added timebase information)
 **            Bugfixes
 **            - tban_getChInfo (temperature param fixed, previous
 **              misunderstanding regarding temp units corrected)
 **            - tban_getHwInfo (added warning level)
 **            - tban_readData (segmentation fault detected when
 **                             multiple connections are opened)
 ** 2006-08-01 Added function:
 **            - tban_parseConfig (Reads configuration options from a
 **              config file)
 ** 2006-08-23 First version after release: libtban-0.4
 **            Fixed bugs
 **            - Removed LINUX|NON_LINUX macros. These made tbancontrol
 **              to not set up communication correctly if the OSTYPE
 **              variable was not set to LINUX (Olivier Vandorpe)
 **            - Shorten the local recieve buffer in tban_readdata to
 **              avoid read error. (Olivier Vandorpe)
 ** 2006-09-01 First version after release: tbancontroller-0.4
 **            Added commands:
 **            - tban_setChHysteresis
 **            General improvements:
 **            - Changed the tban timeout from 1 sec to 5 sec to avoid
 **              communication problems on some devices. (Olivier Vandorpe)
 ** 2006-10-02 Added functions
 **            - tban_ping
 ** 2006-10-03 First version after release: tbancontroller-0.5
 **            General improvements:
 **            - Added general FW version functionality, different
 **              versions of FW support different functions.
 **            - Added fw version check for tban_getWatchdog
 **            Added functions:
 **            - tban_present (Checks if the currently stored vector
 **                            looks like a valid one, ie sanity check)
 **            - tban_setMotion (Set all motion or blockage parameters)
 ** 2006-10-23 Added functions:
 **            - tban_lock
 **            - tban_checkIfDeviceUsed
 **            - tban_configureLockFile
 **            - tban_configureLockTimeout
 ** 2006-11-22 Added functions:
 **  	       - tban_kickWatchdog
 **  	       - tban_disableWatchdog
 ** 2007-03-08 Added internal functions to check firmware version when
 **            using certain functions.
 ** 2007-03-11 First version after release: libtban-0.7
 ** 2007-07-11 Added error messages.
 **            Added code to remove lock file when closing device.
 **
 *****************************************************************************/


#ifndef __TBAN_H
#define __TBAN_H

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include "tban_hw_def.h"


/*****************************************************************************
 * Error codes
 *****************************************************************************/
#define TBAN_OK                     0x00
#define TBAN_ERROR                  0x01
#define TBAN_INDEX_OUT_OF_BOUNDS    0x02
#define TBAN_VALUE_OUT_OF_BOUNDS    0x03
#define TBAN_NOT_OPENED             0x04
#define TBAN_NOT_IMPLEMENTED        0x05
#define TBAN_FW_TOO_OLD             0x06

/* Faulty argument error messages */
#define TBAN_STRUCT_NULL_PTR        0x30
#define TBAN_VALUE_NULL_PTR         0x31
#define TBAN_BUF_NULL_PTR           0x32
#define TBAN_VECTOR_TO_SMALL        0x33

/* Runtime error message  */
#define TBAN_CANNOT_MALLOC          0x40
#define TBAN_CORRUPT_DATA           0x41

/* File operation error messages. Check errno to see why these failed */
#define TBAN_EOPEN                  0x50
#define TBAN_ECLOSE                 0x51
#define TBAN_ESEND                  0x52 
#define TBAN_ERECEIVE               0x53
#define TBAN_ESIGACTION             0x54
#define TBAN_ESIGEMPTYSET           0x55

/* Locking mechanism. Will stop multiple users from using the HW at the
 * same time hopefully locking the other part out until the first one is
 * finished. */
#define TBAN_CANNOT_CREATE_LOCKFILE       0x60
#define TBAN_ALREADY_IN_USE               0x61
#define TBAN_CANNOT_DELETE_LOCK_FILE      0x62
#define TBAN_LOCK_FILE_CHANGE_NOT_ALLOWED 0x63

/* Config file error messages */
#define TBAN_CONFIG_FILE_ERROR      0xf0


/*****************************************************************************
 * Constants
 *****************************************************************************/
/* */
#define TBAN_FALSE          0x00
#define TBAN_TRUE           0x01


/*****************************************************************************
 * TBan commands
 * These defines are taken directly from the excel-sheet that was
 * supplied together with the T-Balancer.
 *****************************************************************************/
/* One byte commands */
#define TBAN_SER_LED_EIN         0x01
#define TBAN_SER_LED_AUS         0x02
#define TBAN_SER_BUZ_EIN         0x03
#define TBAN_SER_BUZ_AUS         0x04
#define TBAN_SER_SOURCE1         0x05 /* Primary source */
#define TBAN_SER_SOURCE2         0x06 /* Alternative source (miniNG...) */

/* Value handling commands (2 byte commands) */
#define TBAN_SER_SET1           0x11
#define TBAN_SER_SET2           0x12
#define TBAN_SER_SET3           0x13
#define TBAN_SER_SET4           0x14
#define TBAN_SER_FREQ           0x15
#define TBAN_SER_MAN            0x16
#define TBAN_SER_INIT1          0x17
#define TBAN_SER_INIT2          0x18
#define TBAN_SER_INIT3          0x19
#define TBAN_SER_INIT4          0x1A

/* Sensor scaling factor */
#define TBAN_SER_OVER           0x21
#define TBAN_SER_SKF            0x22

#define TBAN_SER_MAKE_ABGL      0x35
#define TBAN_SER_REQUEST        0x36
#define TBAN_SER_REQUEST_1      0x38
#define TBAN_SER_REQUEST_2      0x37

/* Blockage functions */
#define TBAN_SER_ERR_UP         0x4A
#define TBAN_SER_ERR_GRENZ      0x4B
#define TBAN_SER_ERR_DOWN       0x4C

/* Channel response curves */
#define TBAN_SER_SET_KANAL1     0x50
#define TBAN_SER_SET_KANAL2     0x60
#define TBAN_SER_SET_KANAL3     0x70
#define TBAN_SER_SET_KANAL4     0x80

#define TBAN_SER_SET_MAX        0x90
#define TBAN_SER_SET_HYS        0xA0
#define TBAN_SER_SET_WARN       0xB0
#define TBAN_SER_SET_ZUORD      0xC0
#define TBAN_SER_SET_ZUORA      0xE0

#define TBAN_ENTER_UPDATE       0xD0
#define TBAN_SER_WERT_SH        0xD8

/* Sensor assignment for channels (this is the base address) */
#define TBAN_SER_SET_ZUORD      0xC0
#define TBAN_SER_SET_ZUORA      0xE0

/* MiniNG commands */
#define TBAN_SER_MINI_S1        0xF0    /* Commandcode (mini_NG) */
#define TBAN_SER_MINI_S2        0xF1    /* Primary value */
#define TBAN_SER_MINI_S2_2      0xF2    /* Secondary value */
#define TBAN_SER_MINI_SEND1     0xF3    /* Flush command to miniNG #1 */
#define TBAN_SER_MINI_SEND2     0xF4    /* Flush command to miniNG #2 */

/* USB watchdog function */
#define USB_WATCHDOG_ON	        0xF5	/* Resets and switch on the USB
                                         * conenction controller, each
                                         * REQUEST will reset controller
                                         * (timeout 10s) */
#define USB_WATCHDOG_OFF        0xF6	/* switch off the USB connection
                                         * controller (quit program) */


/* To be used when pinging the digital sensors */
#define TBAN_SENS_PING          0xFA


/*****************************************************************************
 *  SER_REQUEST result vector index
 *  These are definitions of the index values that can be used when
 *  calling the tban_getValue() function.
 *  
 *  Please observe that the index numbers have been decreased by one
 *  since the vector is o-indexed (useful to know if you look in the
 *  status vector definition document)
 *****************************************************************************/

/* Motion estimation */
#define TBAN_MES_CH_DOWN_EE         9
#define TBAN_MES_CH_GRENZ_EE       10
#define TBAN_MES_CH_UP_EE          11
#define TBAN_MES_OVERIDEBEWEGUNG   12
#define TBAN_MES_BETRIEB          160
#define TBAN_MES_LEVELCHANGECNT   177

/* LED and buzzer */
#define TBAN_LED_ENABLE     17
#define TBAN_BUZ_ENABLE     18

/* Maximum temp point on the response curve */
#define TBAN_TEMP_MaxGrenz0  33
#define TBAN_TEMP_MaxGrenz1  34
#define TBAN_TEMP_MaxGrenz2  35
#define TBAN_TEMP_MaxGrenz3  36

/* Over temperature limits */
#define TBAN_TEMP_MAXWARN0   41
#define TBAN_TEMP_MAXWARN1   42
#define TBAN_TEMP_MAXWARN2   43
#define TBAN_TEMP_MAXWARN3   44

/* */
#define TBAN_TARGETCONTROL_TIMEBASE  126

/* Warning level */
#define TBAN_WARN_LEVEL              181

/* Hardware information */
#define TBAN_INFO_APP      268 /* Device recognition number */
#define TBAN_INFO_SER1     269 /* Internal series */
#define TBAN_INFO_SER2     270 /* Internal series */
#define TBAN_INFO_DATE     271 /* Firmware date */
#define TBAN_INFO_TYPE     272 /* Hardware version */
#define TBAN_INFO_VER      273 /* Firmware version */
#define TBAN_INFO_PROT     274 /* Protocol version */
#define TBAN_SER_BUFFERLN  275 /* Serial buffer length */
#define TBAN_TWI_WERTSH    276 /* Value sending to SensorHub */

/* Watchdog functions */
#define TBAN_WD_COUNTER       277 /* Code reviewed verif ok 2006-10-02 */
#define TBAN_WD_ENABLED       278 /* Code reviewed verif ok 2006-10-02 */


/*****************************************************************************
 * Callback function prototype to be used with tban_setProgressCb()
 * Argument 1: A pointer to data defined by the application when
 *             creating the callback. Will be passed through the TBan
 *             library unaltered.
 * Argument 2: The current value
 * Argument 3. The Maximum value (So the percentage can be decided)
 *****************************************************************************/
typedef void (tban_progressCb)(void*, int, int);




/*****************************************************************************
 * Main structure for holding BigNG related data
 *****************************************************************************/
struct BigNG {
  
  /* Holds the BigNG status vector (second status vector)*/
  unsigned char buf[285];

  /* The time of the last query made. This can be used to decide if an
   * additional query is needed to update the local cache buffer. */
  time_t lastQuery;

  /* Sensor names (read from the config file) */
  char* asName[BIGNG_NUMBER_ADDITIONAL_ANALOG_SENSORS];
  char* asDescr[BIGNG_NUMBER_ADDITIONAL_ANALOG_SENSORS];
} BigNG;


/*****************************************************************************
 * 
 *****************************************************************************/
struct MiniNG {
  /* Local cache of status data from the miniNG */
  unsigned char buf[128];
  /* The time of the last query made. This can be used to decide if an
   * additional query is needed to update the local cache buffer. */
  time_t lastQuery;
  
  /* Sensor names (read from the config file) */
  char* asName[MINI_NG_NUMBER_ANALOG_SENSORS];
  char* asDescr[MINI_NG_NUMBER_ANALOG_SENSORS];

  /* Channel names */
  char* chName[MINI_NG_NUMBER_CHANNELS];
  char* chDesc[MINI_NG_NUMBER_CHANNELS];
} MiniNG;


/*****************************************************************************
 * Main TBan structure
 * This structure is the heart of the implentation and contains most of
 * the information needed when communicating with the T-Balancer
 * device. It can usually be regarded as a "handle" since it is used by
 * all function declared below.
 *****************************************************************************/
struct TBan {
  /* Port settings */
  int   port;
  int   timeout;
  int   baudrate;
  int   databits;
  int   stopBits;
  char* deviceName;
  
  /* Local cache of status data from the TBan */
  unsigned char* buf;

  /* Indicates if the device has been opened or not. */
  int opened;

  /* The time of the last query made. This can be used to decide if an
   * additional query is needed to update the local cache buffer. */
  time_t lastQuery;
  
  /* Old settings for the serial port. Stored before messing around with
   * new settings. */
  struct termios oldtio;

  /* Keep all miniNG data close at hands. Never know when needed*/
  struct MiniNG miniNG;

  /* BigNG data */
  struct BigNG bigNG;
  
  /* Progress callback function */
  tban_progressCb* progressCb;
  void* progressCbPtr;

  /* Sensor names (read from the config file) */
  char* dsName[TBAN_NUMBER_DIGITAL_SENSORS];
  char* asName[TBAN_NUMBER_ANALOG_SENSORS];
  char* chName[TBAN_NUMBER_CHANNELS];
  char* dsDescr[TBAN_NUMBER_DIGITAL_SENSORS];
  char* asDescr[TBAN_NUMBER_ANALOG_SENSORS];
  char* chDescr[TBAN_NUMBER_CHANNELS];

  /* Lock file params */
  char* lockfile;
  int locked;
  int lockTimeout;

} TBan;



/*****************************************************************************
 * Hardware identifications constants
 * These are the values returned by the tban_getHwInfo() function call.
 *****************************************************************************/
typedef enum { TBAN_NO_DEVICE=0, TBAN_DEVICE_TYPE_TBAN=0x10, TBAN_CLASSIC=0x20, TBAN_DEVICE_TYPE_BIGNG=0x30 } TBan_deviceType;
typedef enum { TBAN_APP_TYPE_TBAN=0x11, TBAN_APP_TYPE_BIGNG=0x21 } TBan_appType;


/*****************************************************************************
 * Exported TBan functions
 *****************************************************************************/

/* Config file handling */
int tban_parseConfig(struct TBan* tban, char* filename);

/* USB<->Serial management functions */
int tban_init(struct TBan* tban, char* deviceString);
int tban_free(struct TBan* tban);
int tban_open(struct TBan* tban);
int tban_close(struct TBan* tban);

/* Error management functions */
char* tban_strerror(int code);
char* tban_strerrordesc(int code);
char* tban_strwarn(int code);
char* tban_strwarndesc(int code);

/* Maintenance functions */
int tban_resetHardware(struct TBan* tban);
int tban_setProgressCb(struct TBan* tban, tban_progressCb* func, void* ptr);
int tban_ping(struct TBan* tban, unsigned char pingmask);
int tban_configureLockFile(struct TBan* tban, char lockfile[]);
int tban_configureLockTimeout(struct TBan* tban, int interval);
int tban_checkIfDeviceUsed(struct TBan* tban);
int tban_unlock(struct TBan* tban);
int tban_checkFw(struct TBan* tban, unsigned char fw);

/* Update the local cache (Needed before any of the getters are called) */
int tban_queryStatus(struct TBan* tban);

/* Watchdog commands */
int tban_getWatchdog(struct TBan* tban, unsigned char* wdenabled, unsigned char* wd);
int tban_kickWatchdog(struct TBan* tban);
int tban_disableWatchdog(struct TBan* tban);

/* Getter functions */
int tban_present(struct TBan* tban);
int tban_getFwVersion(struct TBan* tban, unsigned char* fw_major, unsigned char* fw_minor);
int tban_getValue(struct TBan* tban, int index, unsigned char* value);
int tban_getHwInfo(struct TBan* tban, TBan_deviceType* type, TBan_appType* app, unsigned char* fw_major, unsigned char* fw_minor, unsigned char* fw_year, unsigned char* fw_month, unsigned char* protocol_version, unsigned int* ser_buf_len, unsigned char* warn, unsigned char* timebase);
int tban_getPwmFreq(struct TBan* tban, unsigned char* freq);
int tban_getLED(struct TBan* tban, unsigned char* led);
int tban_getBuzzer(struct TBan* tban, unsigned char* buz);
int tban_getMotionSettings(struct TBan* tban, unsigned char* timeConst, unsigned char* maxLimit, unsigned char* incrValue, unsigned char override[], unsigned char rotate[], unsigned char current[]);

/* Setter functions */
int tban_setBuz(struct TBan* tban, unsigned char status);
int tban_setLED(struct TBan* tban, unsigned char status);
int tban_setPwmFreq(struct TBan* tban, unsigned char freq);
int tban_setTacho(struct TBan* tban, unsigned char pingmask);
int tban_setMotion(struct TBan* tban, unsigned char lower, unsigned char upper, unsigned char error);

/* Sensor getters */
int tban_getaSensorTemp(struct TBan* tban, int index, unsigned char* temp, unsigned char* rawTemp, unsigned char* cal);
int tban_getdSensorTemp(struct TBan* tban, int index, unsigned char* temp, unsigned char* rawTemp, unsigned char* cal);
int tban_getDSensorScaleFact(struct TBan* tban, int index, unsigned char* factor);

/* Sensor setters */
int tban_setSensorScaleFact(struct TBan* tban, unsigned char nr, unsigned char factor);

/* Channel getters */
int tban_getChHysteresis(struct TBan* tban, unsigned char index, unsigned char* hysteresis);
int tban_getChMode(struct TBan* tban, unsigned char index, unsigned char* mode, unsigned char* startMode);
int tban_getChCurve(struct TBan* tban, int index, unsigned char x[], unsigned char y[]);
int tban_getChInfo(struct TBan* tban, int index, unsigned int* rpmMax, unsigned char* pwm, unsigned char* resTemp, unsigned char* mode);
int tban_getChSensAssignment(struct TBan* tban, unsigned char index, unsigned char* dsens, unsigned char* asens );
int tban_getChOvertemp(struct TBan* tban, unsigned char channel, unsigned char* ot);

/* Channel setters */
int tban_setChMode(struct TBan* tban, unsigned char modeMask);
int tban_setChCurve(struct TBan* tban, int index, unsigned char x[], unsigned char y[]);
int tban_setChPwm(struct TBan* tban, unsigned char index, unsigned char pwm);
int tban_setChInitValue(struct TBan* tban, unsigned char index, unsigned char pwm);
int tban_setChSensAssignment(struct TBan* tban, unsigned char index, unsigned char dsens, unsigned char asens);
int tban_setChHysteresis(struct TBan* tban, unsigned char nr, unsigned char hysteresis);

#endif /* __TBAN_H*/


