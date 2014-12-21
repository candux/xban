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
 ** Filename:        tban.c
 ** Initial author:  marcus.jagemar@gmail.com
 **
 ** 
 ** DESCRIPTION
 ** -----------
 ** This is the implementation of the TBan library. Please see the
 ** tban.h file for more information.
 **
 ** 
 *****************************************************************************/

#include "tban.h"
#include "common.h"

/* For pid handling */
#include <unistd.h>

/* Linux */
#include <sys/signal.h>
#include <sys/types.h>

/* The default receive buffer size. */
#define TBAN_BUFSIZE   300


/* Indicates if we have data to receive. This actually means that the IO
 * callback function has been called. */
static int tban_dataAvailable = TBAN_FALSE;


/*****************************************************************************
 * Debug printf
 *****************************************************************************/
//#define DEBUG_ENABLED

/* If debug selected enable the code within the brackets, otherwise just
 * discard it. */
#ifndef NDEBUG
  #define DEBUG(s) s
#else
  #define DEBUG(s)
#endif


/*****************************************************************************
 * How long should the application wait after each command has been sent
 * to the TBan. According to mCubed 25ms should be enough. The unit is
 * nano seconds.
 *****************************************************************************/
#define TBAN_COMMAND_DELAY 25000000


/*****************************************************************************
 * Maps channel/sensor index number to a specific value in the buffer
 * read from the driver. PLease note that this array is 0-indexed while
 * the document supplied by mcubed is 1-indexed so the values are not
 * equivalent.
 *****************************************************************************/

/* Channel read mappings */
static int tban_getChPwmMapping[]      = { 137, 138, 139, 140 };
static int tban_getChMaxPwmMapping[]   = { 148, 150, 152, 154 };
static int tban_getChTempMapping[]     = { 252, 253, 254, 255 };
static int tban_getHysteresisMap[]     = { 37, 38, 39, 40 };

/* Channel map for sensor assignment */
static int tban_getDsensorAssignMap[]  = { 45, 46, 47, 48 };
static int tban_getAsensorAssignMap[]  = { 49, 50, 51, 52 };

/* Channel response curve mappings */
static int tban_getChCurveXMap[]       = { 53, 59, 65, 71 };
static int tban_getChCurveYMap[]       = { 77, 83, 89, 95 };

/* Channel mode mappings (automatic or manual mode) */
static int tban_getChModeMap[]         = { 101, 102, 103, 104 };
static int tban_getChStartModeMap[]    = { 5, 6, 7, 8 };

/* Sensor reading mappings */
static int tban_getDsensorMapping[]    = { 238, 239, 240, 241, 242, 243, 244, 245 };
static int tban_getDrawSensorMapping[] = { 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221 };
static int tban_getAsensorMapping[]    = { 246, 247, 248, 249, 250, 251 };
static int tban_getArawSensorMapping[] = { 225, 226, 227, 228, 229, 230 };

/* Scaling factor for sensors */
static int tban_getDScalingFactorMap[]  = { 19, 20, 21, 22, 23, 24, 25, 26 };
static int tban_getAScalingFactorMap[]  = { 27, 28, 29, 30, 31, 32 };


/*****************************************************************************
 * Error definitions
 *****************************************************************************/
typedef struct {
  int   code;
  char* text;
  char* description;
} TBan_errorMapStruct;

static const TBan_errorMapStruct TBan_errorMap[] = {
  { TBAN_OK,                   "TBAN_OK",                  "Command executed ok" },
  { TBAN_ERROR,                "TBAN_ERROR",               "Unknown error" },
  { TBAN_INDEX_OUT_OF_BOUNDS,  "TBAN_INDEX_OUT_OF_BOUNDS", "The channel/sensor index is out of bounds" },
  { TBAN_VALUE_OUT_OF_BOUNDS,  "TBAN_VALUE_OUT_OF_BOUNDS", "The value supplied as an argument to the function is out of bounds" },
  { TBAN_NOT_OPENED,           "TBAN_NOT_OPENED",          "TBan hardware is not open" },
  { TBAN_NOT_IMPLEMENTED,      "TBAN_NOT_IMPLEMENTED",     "Function not implemented yet" },
  { TBAN_FW_TOO_OLD,           "TBAN_FW_TOO_OLD",          "The currently loaded firmware is too old for the function that was trying to execute. Please upgrade" },

  { TBAN_STRUCT_NULL_PTR,      "TBAN_STRUCT_NULL_PTR",     "The TBan structure tried to operate on is NULL. Need to call the tban_init() before." },
  { TBAN_VALUE_NULL_PTR,       "TBAN_VALUE_NULL_PTR",      "The value pointer supplied to the function is NULL" },
  { TBAN_BUF_NULL_PTR,         "TBAN_BUF_NULL_PTR",        "The buffer rpointer supplied to the function is NULL" },
  { TBAN_VECTOR_TO_SMALL,      "TBAN_VECTOR_TO_SMALL",     "The resulting vector is too small" },
  
  { TBAN_CANNOT_MALLOC,        "TBAN_CANNOT_MALLOC",       "malloc couldn't allocate memory" },
  { TBAN_CORRUPT_DATA,         "TBAN_CORRUPT_DATA",        "The query vector is corrupt and unusable until a correct update is made to it." },
  { TBAN_EOPEN,                "TBAN_EOPEN",               "open function call failed" },
  { TBAN_ECLOSE,               "TBAN_ECLOSE",              "close function call failed" },
  { TBAN_ESEND,                "TBAN_ESEND",               "send function call failed" },
  { TBAN_ERECEIVE,             "TBAN_ERECEIVE",            "Timeout when receiving data"},
  { TBAN_ESIGACTION, 	       "TBAN_ESIGACTION",          "Error when installing the serial communication handler (sigaction)" }, 
  { TBAN_ESIGEMPTYSET,         "TBAN_ESIGEMPTYSET"         "Error when clearing the sig set" },

  { TBAN_CANNOT_CREATE_LOCKFILE,       "TBAN_CANNOT_CREATE_LOCKFILE",       "Cannot create the lock file" },
  { TBAN_ALREADY_IN_USE,               "TBAN_ALREADY_IN_USE",               "The TBan is already in use by another program, timeout reached" },
  { TBAN_CANNOT_DELETE_LOCK_FILE,      "TBAN_CANNOT_DELETE_LOCK_FILE",      "Cannot delete the lock file" }

};



/*****************************************************************************
 * Warning Level
 *****************************************************************************/
typedef struct {
  int   code;
  char* text;
  char* description;
} TBan_warningMapStruct;

static const TBan_warningMapStruct TBan_warningMap[] = {
  { 0,	"Normal",    	       "Normal" },
  { 1,	"Startup",   	       "Starting up" },
  { 3,	"USB reset", 	       "USB has been reset" },
  { 4,	"HeatsinkWarn",        "Heatsink onboard warning" },
  { 5,	"SensorAssignment",    "Sensor assignment" },
  { 8,	"HeatsinkCritical",    "Heatsink onboard critical - Reaction: after 10s all to PWM, after 20s all to 100%" },
  { 10,	"FanBlock",            "One of the fans are blocked" },
  { 12,	"Flow",                "Flowmeter range" },
  { 18,	"TempCritical",        "Critical temperature reached" },
  { 19,	"SwitchOffWarn",       "Switch off warning" },
  { 20,	"SwitchOff",           "Switching off" }
};



/**********************************************************************
 * 
 * LOCAL FUNCTIONS
 * 
 **********************************************************************/

/**********************************************************************
 * Name        : intToBaud
 * Description : Convert integer representation of baud rate to the
 *               internal constant defintion.
 * Arguments   : Baud rate.
 * Returning   : The constant describing the required baud rate.
 **********************************************************************/
static long intToBaud(int Baud) {
  switch (Baud) {
      case 38400:
      default:
        return B38400;
      case 19200:
        return B19200;
      case 9600:
        return B9600;
      case 4800:
        return B4800;
      case 2400:
        return B2400;
      case 1800:
        return B1800;
      case 1200:
        return B1200;
      case 600:
        return B600;
      case 300:
        return B300;
      case 200:
        return B200;
      case 150:
        return B150;
      case 134:
        return B134;
      case 110:
        return B110;
      case 75:
        return B75;
      case 50:
        return B50;
  }  /* end of switch baud_rate */
}


/**********************************************************************
 * Name        : intToDataBits
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static long intToDataBits(int Data_Bits) {

  switch (Data_Bits) {
      case 8:
      default:
        return CS8;
      case 7:
        return CS7;
      case 6:
        return CS6;
      case 5:
        return CS5;
  }  /* end of switch data_bits */
}


/**********************************************************************
 * Name        : intToStopBits
 * Description : 
 * Arguments   : 
 * Returning   : 
 **********************************************************************/
static long intToStopBits(int Stop_Bits) {
  switch (Stop_Bits) {
      case 1:
      default:
        return 0;
      case 2:
        return CSTOPB;
  }  /* end of switch stop bits */
}


/**********************************************************************
 * Name        : checktimeout
 * Description : Check if the timeout has expired.
 * Arguments   : starttime = The time when the timer started.
 *               timeout   = How many seconds should pass from the start
 *                           of the timer.
 * Returning   : TBAN_TRUE  = Within the timeout (has not expired)
 *               TBAN_FALSE = Greater than the timeoute (has expired)
 **********************************************************************/
static int checktimeout(time_t starttime, int timeout) {
  if (time(NULL)-starttime < timeout) {
    return TBAN_TRUE;
  } else {
    return TBAN_FALSE;
  }
}



/**********************************************************************
 * Name        : tban_signal_handler_IO
 * Description : The signal handler for IO events on the USB-serial
 *               line. 
 * Arguments   : status
 * Returning   : none
 **********************************************************************/
static void tban_signal_handler_IO (int status) {
  (void) status;
  /* printf("!!! DATA AVAILABLE !!!\n"); */
  tban_dataAvailable = TBAN_TRUE;
}



/**********************************************************************
 * Name        : tban_fileExists 
 * Description : Check if a file exists, if so lets parse the first
 * 		 bytes of the file to see if we can get the pid located
 * 		 in  it. If so return the pid.
 * Arguments   : filename = The filename to operate on.
 * Returning   : 0  = no file exists
 * 		 >0 = A file exists and the pid in it is returned.
 **********************************************************************/
static int tban_fileExists(char filename[]) {
  FILE *fp = fopen(filename,"r");
  if(fp != NULL) {
    /* The file exists */
    char buf[16];
    int  numberBytes;
    int  value;

    /* Read contents of the file */
    memset(buf, 0, 16);
    numberBytes = fread(buf, sizeof(char), 16, fp);

    /* try to convert it to a number (pid) */
    errno=0;
    value = atoi(buf);
    if((value==0) && (errno != 0)) {
      printf("Error converting number value=%d errno=%d\n", value, errno);
      return 0;
    }

    /* Close the open file and return */
    fclose(fp);
    return value;
  }

  /* No file is present */
  return 0;
}



/**********************************************************************
 * Name        : createLockFile
 * Description : Create a lock file.
 * Arguments   : filename = The lock file name
 * 		 string   = The string to be put in the lockfile.
 * Returning   : TBAN_OK
 * 		 TBAN_CANNOT_CREATE_LOCKFILE
 **********************************************************************/
static int createLockFile(char* filename, char string[]) {
  FILE* lock;
  int result;
  
  lock = fopen(filename, "wb+");
  if(lock == NULL) {
    printf("Unable to create file(%s) => NULL (errno=%d) %s\n",
           filename,
           errno, strerror(errno));
    return TBAN_CANNOT_CREATE_LOCKFILE;
  }
  
  result = fwrite(string, sizeof(char), strlen(string), lock);
  if(result == 0)
    return TBAN_CANNOT_CREATE_LOCKFILE;
  
  /* Close and flush data to disk */
  result = fclose(lock);
  if(result != 0)
    return TBAN_CANNOT_CREATE_LOCKFILE;
  
  return TBAN_OK;
}



/**********************************************************************
 * 
 * EXPORTED FUNCTIONS
 * 
 **********************************************************************/

/**********************************************************************
 * Name        : tban_lock
 * Description : Check if a lock file is already present. If
 * Arguments   : -
 * Returning   : TBAN_OK
 * 		 TBAN_STRUCT_NULL_PTR
 * 		 TBAN_ALREADY_IN_USE
 * 		 TBAN_CANNOT_CREATE_LOCKFILE
 **********************************************************************/
static int tban_lock(struct TBan* tban) {
  pid_t pid;
  char s[16];
  int  result;
  int value;
    
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Check if there already exists a lock file */
  value = tban_fileExists(tban->lockfile);
  if(value > 0) {
    /* Yes there is a file, lets check if the pid located inside it is
     * valid. Maybe the process created it is sloppy and didn't clean up
     * after itself... */
    result = getsid(value);
    /* Check if the pid in the file is really pointing to a running
     * process. Maybe the process that created the file is sloppy and
     * didn't remove the file? Anyhow it the getsid() function returns
     * error then we know that the process doesn't exists and we can
     * remove the file and try again. */
    if((result == -1) && (errno=ESRCH)) {
      /* No valid pid in the file */
      result = tban_unlock(tban);
      if(result != TBAN_OK) {
        return TBAN_LOCK_FILE_CHANGE_NOT_ALLOWED;
      }
    } else {
      /* The pid in the lock file is valid */
      return TBAN_ALREADY_IN_USE;
    }
  } /* if */

  /* Put the calling pid in the file */
  pid =  getpid();
  if(pid == -1)
    return TBAN_ERROR;
  memset(s, 0, 16);
  sprintf(s, "%d", pid);
  result = createLockFile(tban->lockfile, s);
  if(result != TBAN_OK)
    return result;

  /* Toggle the lock inside the tban struct */
  tban->locked = 1;

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_unlock
 * Description : Unlock the TBan hadware, i.e. remove the locking file.
 * Arguments   : -
 * Returning   : TBAN_OK
 * 		 TBAN_STRUCT_NULL_PTR
 * 		 TBAN_CANNOT_CREATE_LOCKFILE
 **********************************************************************/
int tban_unlock(struct TBan* tban) {
  int result;

  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Delete the lock file */
  result = unlink(tban->lockfile);
  if(result != 0)
    return TBAN_CANNOT_DELETE_LOCK_FILE;

  /* Toggle the lock inside the tban struct */
  tban->locked = 0;

  return TBAN_OK;
}

/**********************************************************************
 * Name        : tban_configureLockFile
 * Description : Set the lock file name
 * Arguments   : filename
 * Returning   : TBAN_OK
 * 		 TBAN_STRUCT_NULL_PTR
 **********************************************************************/
int tban_configureLockFile(struct TBan* tban, char lockfile[]) {
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* If we have a filename already specified lets remove it. */
  if(tban->lockfile != NULL)
    free (tban->lockfile);

  tban->lockfile = malloc(strlen(lockfile)+1);
  strcpy(tban->lockfile, lockfile);
  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_configureLockInterval
 * Description : Set the timeout value for lock checking, how long time
 * 		 should the application "hang" when waiting for the lock
 * 		 file to be removed.
 * Arguments   : timeout : in seconds
 * Returning   : TBAN_OK
 * 		 TBAN_STRUCT_NULL_PTR
 **********************************************************************/
int tban_configureLockTimeout(struct TBan* tban, int timeout) {
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  tban->lockTimeout = timeout;
  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_checkIfDeviceUsed
 * Description : Check if the TBan is currently in use by looking for
 * 		 the lockfile. If this is not present this function will
 * 		 create a new one and signal the calling function that
 * 		 it is ok to use the TBan. An error is returned if the
 * 		 device is not available.
 * Arguments   : -
 * Returning   : TBAN_OK (If a new lock file has been created)
 * 		 TBAN_ALREADY_IN_USE (If the device is already allocated
 * 		 		      by another application)
 **********************************************************************/
int tban_checkIfDeviceUsed(struct TBan* tban) {
  time_t t1, t2;
  int result;

  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Get the current time (comparison base) */
  t1 = time(NULL);

  /* Check the lock file a  number of times until the timeout is
   * reached. Then signal to the caller that the device is still
   * locked. */
  for(;;) {
    result = tban_lock(tban);
    t2 = time(NULL);
    /* If we reached the timeout exit the loop */
    if(((t2 - t1) > tban->lockTimeout) || (result == TBAN_OK))
      break;
    /* Sleep for some time to let the other part finish with the TBan */
    local_nanosleep(1, 0);
  }

  /* If we reach this point and the result is anything but success we
   * know that the TBan is still locked. Lets tell the caller. */
  if(result != TBAN_OK) {
    return TBAN_ALREADY_IN_USE;
  }

  /* TBan ready to be opened */
  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_strerror
 * Description : Convert the TBAN integer error message to a human
 * 		 readable format according to the definition in this
 * 		 file.
 * Arguments   : code = The error code to convert.
 * Returning   : The stringular error text of the error or NULL if not
 * 		 found in the array of known faults.
 **********************************************************************/
char* tban_strerror(int code) {
  int i;
  for(i=0; i<sizeof(TBan_errorMap); i++) {
    if(TBan_errorMap[i].code==code) {
      return TBan_errorMap[i].text;
    }
  }
  return NULL;
}



/**********************************************************************
 * Name        : tban_strerrordesc
 * Description : Convert the TBAN integer error message to a description.
 * Arguments   : code = The error code to convert.
 * Returning   : The stringular description of the error or NULL if not
 * 		 found in the array of known faults.
 **********************************************************************/
char* tban_strerrordesc(int code) {
  int i;
  
  for(i=0; i<sizeof(TBan_errorMap); i++) {
    if(TBan_errorMap[i].code==code) {
      return TBan_errorMap[i].description;
    }
  }
  return NULL;
}


/**********************************************************************
 * Name        : tban_strwarn
 * Description : Convert the TBAN integer warning message to a human
 * 		 readable format according to the definition in this
 * 		 file.
 * Arguments   : code = The status code to convert.
 * Returning   : The stringular error text of the error or NULL if not
 * 		 found in the array of known faults.
 **********************************************************************/
char* tban_strwarn(int code) {
  int i;
  for(i=0; i<sizeof(TBan_warningMap); i++) {
    if(TBan_warningMap[i].code==code) {
      return TBan_warningMap[i].text;
    }
  }
  return NULL;
}

/**********************************************************************
 * Name        : tban_strwarndesc
 * Description : Convert the TBAN integer warning message to a human
 * 		 readable format according to the definition in this
 * 		 file.
 * Arguments   : code = The status code to convert.
 * Returning   : The stringular error description of the error or NULL
 * 		 if not found in the array of known faults.
 **********************************************************************/
char* tban_strwarndesc(int code) {
  int i;
  for(i=0; i<sizeof(TBan_warningMap); i++) {
    if(TBan_warningMap[i].code==code) {
      return TBan_warningMap[i].description;
    }
  }
  return NULL;
}


/**********************************************************************
 * Name        : tban_init
 * Description : Initialise the tban structure. Allocate memory for
 *               receive buffer and reset all parameters to sane
 *               values.
 * Arguments   : tban   = The TBan structure.
 *               devStr = The path to the device. For example
 *                        ("/dev/ttyUSB0".
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_CANNOT_MALLOC
 **********************************************************************/
int tban_init(struct TBan* tban, char* devStr) {
  int i;
  char local_lockfile[] = "/tmp/xban.lock";

  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(devStr == NULL)
    return TBAN_VALUE_NULL_PTR;

  /* No data has been received yet since the device is not opened. */
  tban->opened = 0;
  tban_dataAvailable = TBAN_FALSE;

  /* No query has been made yet */
  tban->lastQuery = 0;

  /* Set standard communication params */
  tban->port       = 0;
  tban->timeout    = 10;
  tban->baudrate   = 19200;
  tban->databits   = 8;
  tban->stopBits   = 0;
  tban->deviceName=malloc(strlen(devStr)+1);
  if(tban == NULL)
    return TBAN_CANNOT_MALLOC;
  (void) strcpy(tban->deviceName, devStr);

  /* Allocate and clear the receive buffer */
  tban->buf = malloc(TBAN_BUFSIZE);
  if(tban->buf == NULL) {
    free(tban->deviceName);
    return TBAN_CANNOT_MALLOC;
  }
  (void) memset(tban->buf, 0, sizeof(tban->buf));

  /* Digital sensor default names */
  for(i=0; i<TBAN_NUMBER_DIGITAL_SENSORS; i++) {
    char string[16];
    sprintf(string, "DS%d", i);
    tban->dsName[i]  = malloc(strlen(string)+1);
    tban->dsDescr[i] = malloc(strlen(string)+1);
    strcpy(tban->dsName[i], string);
    strcpy(tban->dsDescr[i], string);
  }

  /* Analog sensor default names */
  for(i=0; i<TBAN_NUMBER_ANALOG_SENSORS; i++) {
    char string[16];
    sprintf(string, "AS%d", i);
    tban->asName[i]  = malloc(strlen(string)+1);
    tban->asDescr[i] = malloc(strlen(string)+1);
    strcpy(tban->asName[i], string);
    strcpy(tban->asDescr[i], string);
  }

  /* Channel default names */
  for(i=0; i<TBAN_NUMBER_CHANNELS; i++) {
    char string[16];
    sprintf(string, "CH%d", i);
    tban->chName[i]  = malloc(strlen(string)+1);
    tban->chDescr[i] = malloc(strlen(string)+1);
    strcpy(tban->chName[i], string);
    strcpy(tban->chDescr[i], string);
  }

  /* Lockfile default params */
  tban->lockfile = malloc(strlen(local_lockfile)+1);
  strcpy(tban->lockfile, local_lockfile);
  tban->locked = 0;
  tban->lockTimeout = 10;

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_free
 * Description : Free all memory belonging to the tban structure.
 * Arguments   : tban = The TBan structure to be freed.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 **********************************************************************/
int tban_free(struct TBan* tban) {

  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  
  /* Freeing memory belonging to receive buffer */
  if(tban->buf == NULL) {
    return TBAN_STRUCT_NULL_PTR;
  }

  /* Free device string data */
  if(tban->deviceName != NULL) {
    free(tban->deviceName);
    tban->deviceName = NULL;
  }

  /* Free buffer data */
  if(tban->buf != NULL) {
    free(tban->buf);
    tban->buf = NULL;
  }

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_setProgressCb
 * Description : Set the progress callback function that will be called
 * 		 whenever needed. For example when sending large amount
 * 		 of data the needed delays will cause the program to
 * 		 hang for a while, then the callback can display some
 * 		 useful user-response.
 * Arguments   : tban = The TBan struct to use when communicating
 *		 cb   = The application callback (or null if disable)
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 **********************************************************************/
int tban_setProgressCb(struct TBan* tban, tban_progressCb* func, void* ptr) {
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  tban->progressCb = func;
  tban->progressCbPtr = ptr;
  
  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_updateProgress
 * Description : Update the progress for the calling application (if the
 * 		 callback function has been set)
 * Arguments   : tban = The TBan struct to use when communicating
 *		 cur = The current value
 *		 max = The maximum value
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 **********************************************************************/
void tban_updateProgress(struct TBan* tban, int cur, int max) {
  tban_progressCb* func = tban->progressCb;
  if(func != NULL) {
    func(tban->progressCbPtr, cur, max);
  }
}




/**********************************************************************
 * Name        : tban_sendCommand
 * Description : Send a TBan command (either a one-byte command or a
 *               two-byte one)
 * Arguments   : tban   = The TBan struct to use when communicating
 *               sndBuf = The buffer containing the command.
 *               cmdLen = The length of the command (usually 1 or 2
 *                        bytes) 
 * Returning   : TBAN_OK
 *               TBAN_ESEND
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_BUF_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_sendCommand(struct TBan* tban, unsigned char* sndBuf, int cmdLen) {
  int result, i;

  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(sndBuf == NULL)
    return TBAN_BUF_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  DEBUG(
    printf("tban_sendCommand: 0x");
    for (i = 0; i < cmdLen; i++) {
      printf("%02x", sndBuf[i]);
    }
    printf("\n");
  )

  /* Write data to port */
  result = write(tban->port, sndBuf, cmdLen);
  if(result == -1)
    return TBAN_ESEND;
  else {
    local_nanosleep(0, TBAN_COMMAND_DELAY);
    return TBAN_OK;
  }
}


/**********************************************************************
 * Name        : tban_readData
 * Description : Read data from the TBan unit using the device attached
 *               to earlier. 
 * Arguments   : tban     = The TBan device to operate on.
 *               expected = The maximum number of bytes to read from the
 *                          TBan device.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_ERECEIVE
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_readData(struct TBan* tban, unsigned char* buf, int expected) {
  /* Temp receive buffer. */
  unsigned char   local_buf[32] = "";
  /* The number of bytes the last read returned */
  int             bytesread;
  /* Keeps track of the current position in memory when the data is to be
   * dumped. */
  int             currdest;
  time_t          starttime;

  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if(buf == NULL)
    return TBAN_BUF_NULL_PTR;

  /* Get starttime for checking of timeout */
  starttime = time(NULL);
  DEBUG(printf("starttime=%d\n", (int) starttime));
  
  /* Wait for data to arrive in the queue. The signal function defined
   * in the tban_openPort function toggles the tban_dataAvailable flag
   * when this happens. Otherwise just hang on and wait for the timeout
   * to expire. */
  tban_dataAvailable = TBAN_FALSE;
  while ((tban_dataAvailable == TBAN_FALSE) && checktimeout(starttime, tban->timeout)) {
    local_nanosleep(1,0);
  }

  /* Nothing received within the timeout so lets signal an error to the
   * user */
  if(!checktimeout(starttime,tban->timeout)) {
    return TBAN_ERECEIVE;
  }

  /* Read data from port. But keep it within the limits of the temp
   * buffer. It will becopied to the correct buffer later on.  */
  redo_read:
  currdest = 0;
  bytesread = read(tban->port, local_buf, sizeof(local_buf));
  tban_dataAvailable = TBAN_FALSE;
  if(bytesread == -1) {
    local_nanosleep(1,0);
    goto redo_read;
  }

  /* Loop until (if the expected parameter is <> 0) the expected amount of
   * data is returned, else until timeout. */
  while (((currdest < expected) || (expected == 0)) && (checktimeout(starttime, tban->timeout))) {
    /* This doesn't work if we doesn't have NPTL enabled for glibc */

    /* Lets try to avoid overwrite problems in the supplied receive
     * buffer */
    if((currdest+bytesread) >= expected) {
      bytesread = expected-currdest;
    }
    
    DEBUG(printf("before memcpy\n"));
    (void) memcpy(buf+currdest, &local_buf, bytesread);
    /* Advance the pointer */
    currdest += bytesread;
    if (currdest < expected) {
      DEBUG(printf("-- %d bytes read of the expected %d \n", currdest, expected));
      /* */      tban_dataAvailable = TBAN_TRUE;
      while ((tban_dataAvailable == TBAN_FALSE) && (checktimeout(starttime, tban->timeout))) {
        local_nanosleep(1,0);
      }
      if (checktimeout(starttime, tban->timeout)) {
        redo_read2:
        bytesread = read(tban->port, local_buf, sizeof(local_buf));
        if(bytesread == -1) {
          local_nanosleep(1,0);
          if(checktimeout(starttime, tban->timeout)) {
            DEBUG(printf("goto redo_read2\n"));
            goto redo_read2;
          }
        }

        tban_dataAvailable = TBAN_FALSE;
      }
    }
  }

  /* Make sure we return the correct value depending on the timeout
   * values */
  if (!checktimeout(starttime, tban->timeout)) {
    DEBUG(printf("TBAN_ERECEIVE\n"));
    return TBAN_ERECEIVE;
  } else {
    DEBUG(printf("-- %d bytes read of the expected %d \n", currdest, expected));
    DEBUG(printf("TBAN_OK\n"));
    return TBAN_OK;
  }
}



/**********************************************************************
 * Name        : tban_flushData
 * Description : Calling this function causes data in the queue to be
 *               discarded.
 * Arguments   : tban = The TBan struct to work on.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_EOPEN
 **********************************************************************/
int tban_flushData(struct TBan* tban) {
  time_t          starttime;
  unsigned char* buf;
  int bytesread;

  buf = malloc(128);

  /* Get starttime for checking of timeout */
  starttime = time(NULL);
  while ((tban_dataAvailable == TBAN_FALSE) && checktimeout(starttime, tban->timeout)) {
    local_nanosleep(1,0);
  }

  /* Nothing received within the timeout so lets signal an error to the
   * user */
  if(!checktimeout(starttime,tban->timeout)) {
    printf("Nothing to flush \n");
    free(buf);
    return TBAN_OK;
  }

  tban_dataAvailable = TBAN_TRUE;
  while(tban_dataAvailable == TBAN_TRUE) {
    tban_dataAvailable = TBAN_FALSE;
    bytesread = read(tban->port, buf, sizeof(buf));
    printf("Flushed %d bytes\n", bytesread);
    local_nanosleep(1,0);
  }

  free(buf);

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_open
 * Description : Open the TBan port for usage. This basically just opens
 *               the device file (for example "/dev/ttyUSB0") for
 *               reading and writing and links a serial handler to be
 *               called when there are data to be read.
 * Arguments   : tban = The TBan struct to work on.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_EOPEN
 **********************************************************************/
int tban_open(struct TBan* tban) {
  /* The initialisation-snippet is *not very* loosely based on "3.3
   * Asynchronous Input" from the "Serial-Programing-HOWTO.html" at
   * http://linuxdoc.org.. Had no real reason to change it much. */

  /* Definition of signal action */
  struct sigaction saio;
  struct termios   newtio;
  int              result;

  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened != 0)
    return TBAN_EOPEN;

  /* Make sure that noone else is using the device. After this function
   * call has been called and TBAN_OK is returned the device is
   * allocated for this application. */
  result = tban_checkIfDeviceUsed(tban);
  if(result != TBAN_OK)
    return result;

  /* Open the device */
  result = open(tban->deviceName, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if(result < 0)
    return TBAN_EOPEN;
  tban->port = result;

  /* Install the asynchronous serial handler, i.e. the callback function
   * that will be called when there are data on the serial port. */
  saio.sa_handler = tban_signal_handler_IO;
  result = sigemptyset(&saio.sa_mask);
  if(result != 0)
    return TBAN_ESIGEMPTYSET;
  saio.sa_flags = 0;
#ifndef DRYRUN
  saio.sa_restorer = NULL;
#endif

  result = sigaction(SIGIO, &saio, NULL);
  if(result != 0)
    return TBAN_ESIGACTION;

  /* Allow the process to receive SIGIO */
  result = fcntl(tban->port, F_SETOWN, getpid());
  if(result != 0)
    return TBAN_EOPEN;
  
  /* Make the file descriptor asynchronous (the manual page says only
   * O_APPEND and O_NONBLOCK, will work with F_SETFL...) */
#ifndef DRYRUN
  result = fcntl(tban->port, F_SETFL, FASYNC);
#endif
  if(result != 0)
    return TBAN_EOPEN;

  /* save current port settings */  
  result = tcgetattr(tban->port, &(tban->oldtio)); 
  if(result != 0)
    return TBAN_EOPEN;

  /* Set new port settings for canonical input processing */
  newtio.c_cflag = intToBaud(tban->baudrate)
    | CRTSCTS
    | intToDataBits(tban->databits)
    | intToStopBits(tban->stopBits)
    | CLOCAL
    | CREAD;
  newtio.c_iflag     = IGNPAR;
  newtio.c_oflag     = 0;
  newtio.c_lflag     = 0;
  newtio.c_cc[VMIN]  = 1;
  newtio.c_cc[VTIME] = 0;
  result = tcflush(tban->port, TCIFLUSH);
  if(result != 0)
    return TBAN_EOPEN;

  result = tcsetattr(tban->port, TCSANOW, &newtio);
  if(result != 0)
    return TBAN_EOPEN;

  /* Indicate that the port is now opened */
  tban->opened = 1;

  /* End initialisation   */
  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_close
 * Description : Close the TBan port for further use.
 * Arguments   : tban = The TBan structure to operate on.
 * Returning   : TBAN_OK
 *               TBAN_NOT_OPENED
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_ECLOSE
 **********************************************************************/
int tban_close(struct TBan* tban) {
  int result;
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened==0)
    return TBAN_NOT_OPENED;

  /* Reset port settings */
  result = tcsetattr(tban->port,TCSANOW, &(tban->oldtio));
  if(result != 0)
    return TBAN_ECLOSE;
  
  result = close(tban->port);
  if(result != 0)
    return TBAN_ECLOSE;

  /* Indicate that the port is closed */
  tban->opened=0;
  
  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_queryStatus
 * Description : Query the TBan about the current status. This will
 *               effectively update the local cache with fresh
 *               information. 
 * Arguments   : tban = The TBan structure.
 *               The tban->buf must be allocated and be large enough to
 *               handle the resulting vector (285 bytes).
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 *               TBAN_ERECEIVE
 *               TBAN_BUF_NULL_PTR
 *               TBAN_CORRUPT_DATA
 **********************************************************************/
int tban_queryStatus(struct TBan* tban) {
  unsigned char sndBuf[8];

  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if(tban->buf == NULL)
    return TBAN_BUF_NULL_PTR;

  /* Send the query command. Make sure that the command is sent to the
   * TBan itself and not to the add-on modules such as miniNG. */
  sndBuf[0] = TBAN_SER_SOURCE1;
  sndBuf[1] = TBAN_SER_REQUEST;
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  
  /* Receive the result from the HW */
  CHECK_RESULT(tban_readData(tban, tban->buf, 285));

  /* Make some simple checks on the returned vector. Like that it
   * contains the value "100" in the first position */
  if(tban_present(tban) != TBAN_OK) {
    return TBAN_CORRUPT_DATA;
  }

  /* Update the time stamp for the last update, but only if we suceeded
   * with the update */
  tban->lastQuery = time(NULL);

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_present
 * Description : Check if the TBan is actually present in the system
 * Arguments   : tban  = The TBan structure to operate on.
 * Returning   : TBAN_OK
 * 		 TBAN_STRUCT_NULL_PTR
 *               TBAN_CORRUPT_DATA
 **********************************************************************/
int tban_present(struct TBan* tban) {
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Make some simple checks on the returned vector. Like that it
   * contains the value "100" in the first position */
  if(tban->buf[0] != 100) {
    return TBAN_CORRUPT_DATA;
  }
  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_getHwInfo
 * Description : Retrieve the hardware information, such as device
 *               number, firmware version, date and so on.
 * Arguments   : tban  = The TBan structure to operate on.
 *		 type = The type of device (see TBan_deviceType)
 *		 app  = The type of device (see TBan_appType)
 *		 fw_major = 2 for TBan, 3 for bigNG
 *		 fw_minor = incremental revision number
 *		 fw_year  = FW release year
 *		 fw_month = FW release month
 *		 protocol_version = Supported protocol. (26 is 2.6, 28
 * 		                    is 2.8 and so on)
 *		 ser_buf_len = The length of the serial buffer
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_ERECEIVE
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getHwInfo(struct TBan* tban,
                   TBan_deviceType* type,
                   TBan_appType* app,
                   unsigned char* fw_major,
                   unsigned char* fw_minor,
                   unsigned char* fw_year,
                   unsigned char* fw_month,
                   unsigned char* protocol_version,
                   unsigned int*  ser_buf_len,
                   unsigned char* warn,
                   unsigned char* timebase) {
  /*Some local vars */
  unsigned char fw;

  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if((fw_major == NULL) || (fw_minor == NULL) ||
     (fw_year == NULL) || (fw_month == NULL) ||
     (protocol_version == NULL) || (ser_buf_len == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Determine the type of device we are dealing with */
  *type = tban->buf[TBAN_INFO_TYPE];

  /* App */
  *app = tban->buf[TBAN_INFO_APP];
  
  /* Firmware version */
  fw = tban->buf[TBAN_INFO_VER];
  *fw_major = (fw & (255-15)) >> 4; /* High nibble */
  *fw_minor = fw & 15;              /* Low nibble */

  /* Firmware date */
  fw = tban->buf[TBAN_INFO_DATE];
  *fw_month  = (fw & (255-15)) >> 4; /* High nibble */
  *fw_year   = fw & 15;              /* Low nibble */

  /* Protocol */
  *protocol_version = tban->buf[TBAN_INFO_PROT];

  /* Serial buffer len. This should always be 285 */
  *ser_buf_len = 256+(tban->buf[TBAN_SER_BUFFERLN]);

  /* Warning level */
  *warn = (tban->buf[TBAN_WARN_LEVEL]);

  /* timebase */
  *timebase = (tban->buf[TBAN_TARGETCONTROL_TIMEBASE]);

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_getPwmFreq
 * Description : Retrieve the hardware PWM frequency.
 * Arguments   : tban  =
 *               freq = The frequency
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getPwmFreq(struct TBan* tban, unsigned char* freq) {
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(freq == NULL)
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Return data */
  *freq = tban->buf[4];

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_getChOvertemp
 * Description : Get the over temp settings for the selected channel.
 * Arguments   : tban  =
 *               freq = The frequency
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getChOvertemp(struct TBan* tban, unsigned char channel, unsigned char* ot) {
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(ot == NULL)
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if(channel >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  CHECK_RESULT(tban_getValue(tban, TBAN_TEMP_MAXWARN0+channel, ot));
  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_getValue
 * Description : Return the status for the index supplied. This function
 *               call requires that tban_queryStatus has been called
 *               before.
 * Arguments   : tban  = The TBan structure
 *               index = The index in the status vector read from the
 *               TBan HW.
 *               value = The value to be returned to the caller, the
 *                       corresponding value to the index supplied.
 * Returning   : TBAN_OK
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getValue(struct TBan* tban, int index, unsigned char* value) {
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(value == NULL)
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Return a value to the caller */
  *value = tban->buf[index];
  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_getChInfo
 * Description : Get fan information
 * Arguments   : tban    = The TBan struct
 *               index   = The fan index number (0-indexed)
 *               rpmMax  = The maximum rpm possible for the current
 *                         fan.
 *               pwm     = The percentage of maximum throttle.
 *               resTemp = The temperatur for the current fan
 *                         (Centigrade)
 *               mode    = 0:automatic mode ; 1=manual mode
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getChInfo(struct TBan* tban, int index, unsigned int* rpmMax, unsigned char* pwm, unsigned char* resTemp, unsigned char* mode) {
  unsigned char hb, lb;
/*   float         ftemp; */
/*   unsigned char value; */
  
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((rpmMax == NULL) || (pwm == NULL) || (resTemp == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get the maximum pwm */
  CHECK_RESULT(tban_getValue(tban, tban_getChMaxPwmMapping[index], &lb));
  CHECK_RESULT(tban_getValue(tban, tban_getChMaxPwmMapping[index]+1, &hb));

  /* Calculate the maximum rpm  */
  *rpmMax = (unsigned int) (((float) 256*hb + lb) * (float) 10.5);

  /* Get the pwm */
  if(index >= TBAN_NUMBER_CHANNELS) {
    return TBAN_INDEX_OUT_OF_BOUNDS;
  }

  /* Get the pwm */
  CHECK_RESULT(tban_getValue(tban, tban_getChPwmMapping[index], pwm));

  /* The pwm needs to be corrected to be valid according to the max-rpm */
  (*pwm) = 2*(*pwm);

  /* Get temp. Strangely enough the european product TBan returns the
   * temperature in Farenheit so we need to convert it to Celsius... */
  CHECK_RESULT(tban_getValue(tban, tban_getChTempMapping[index], resTemp));

  /* Get mode */
  CHECK_RESULT(tban_getValue(tban, tban_getChModeMap[index], mode));

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_getDSensorScaleFact
 * Description : Get the scaling factor for a certain sensor.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The digital sensor index number (0-indexed)
 *               factor  = The scaling factor.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getDSensorScaleFact(struct TBan* tban, int index, unsigned char* factor) {
  /* argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= (TBAN_NUMBER_DIGITAL_SENSORS))
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if(factor == NULL)
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get the scaling factor for the selected sensor */
  CHECK_RESULT(tban_getValue(tban, tban_getDScalingFactorMap[index], factor));

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_getdSensorTemp
 * Description : Get the status for a digital sensor. Please note that
 *               the temperature needs to be divided by two to get the
 *               actual temp.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The digital sensor index number (0-indexed)
 *               temp    = The double calibrated temperature value
 *               rawTemp = The double raw temperature value
 *               cal     = The calibration value
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getdSensorTemp(struct TBan* tban, int index, unsigned char* temp, unsigned char* rawTemp, unsigned char* cal) {
  /* argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= TBAN_NUMBER_DIGITAL_SENSORS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((temp == NULL) || (rawTemp == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get the temperature of the digital sensor */
  CHECK_RESULT(tban_getValue(tban, tban_getDsensorMapping[index],    temp));
  CHECK_RESULT(tban_getValue(tban, tban_getDrawSensorMapping[index], rawTemp));
  CHECK_RESULT(tban_getValue(tban, tban_getDScalingFactorMap[index], cal));

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_getaSensorTemp
 * Description : Get the status for an analog sensor. Please note that
 *               the temperature needs to be divided by two to get the
 *               actual temp.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The analog sensor index number (0-indexed)
 *               temp    = The double calibrated temperature value
 *               rawTemp = The double raw temperature value
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getaSensorTemp(struct TBan* tban, int index, unsigned char* temp, unsigned char* rawTemp, unsigned char* cal) {
  /* argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= TBAN_NUMBER_ANALOG_SENSORS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((temp == NULL) || (rawTemp == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get the temperature of the digital sensor */
  CHECK_RESULT(tban_getValue(tban, tban_getAsensorMapping[index],    temp));
  CHECK_RESULT(tban_getValue(tban, tban_getArawSensorMapping[index], rawTemp));
  CHECK_RESULT(tban_getValue(tban, tban_getAScalingFactorMap[index], cal));

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_getLED
 * Description : Get the LED status.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The digital sensor index number (0-indexed)
 *               led     = LED status (1=on, 0=off)
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getLED(struct TBan* tban, unsigned char* led) {
  /* argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(led == NULL)
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get LED status */
  CHECK_RESULT(tban_getValue(tban, TBAN_LED_ENABLE, led));

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_getBuzzer
 * Description : Get the LED status.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The digital sensor index number (0-indexed)
 *               led     = LED status (1=on, 0=off)
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getBuzzer(struct TBan* tban, unsigned char* buz) {
  /* argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(buz == NULL)
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get LED status */
  CHECK_RESULT(tban_getValue(tban, TBAN_BUZ_ENABLE, buz));

  return TBAN_OK;
}




/**********************************************************************
 * Name        : tban_getChCurve
 * Description : Get the response curve for the selected fan.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The fan index (0-indexed)
 *               x       = A vector of temperature values. Each element
 *                         in this vector corresponds to one in the
 *                         other vector (y) supplied to this function.
 *               y       = A vector of requested PWM values.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getChCurve(struct TBan* tban, int index, unsigned char x[], unsigned char y[]) {
  int i;

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((x == NULL) || (y == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  
  /* Obtain the response curve for the fan. For the first point the pwm
   * is always 0. */
  for(i=0; i<6; i++) {
    /* Get the temperature */
    CHECK_RESULT(tban_getValue(tban, tban_getChCurveXMap[index]+i, &(x[i])));

    /* Get the requested pwm */
    CHECK_RESULT(tban_getValue(tban, tban_getChCurveYMap[index]+i, &(y[i])));
  }

  /* Get the maximum temp value. For this point the pwm is always 100%  */
  CHECK_RESULT(tban_getValue(tban, TBAN_TEMP_MaxGrenz0+index, &(x[6])));
  y[6]=100;

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_setChCurve
 * Description : Set the response curve for a particular channel.
 * Arguments   : tban    = The TBan struct to work on
 *               nr      = The channel index (0-indexed)
 *               x       = A vector of temperature values. Each element
 *                         in this vector corresponds to one in the
 *                         other vector (y) supplied to this function.
 *               y       = A vector of requested PWM values.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_VECTOR_TO_SMALL
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setChCurve(struct TBan* tban, int nr, unsigned char x[], unsigned char y[]) {
  unsigned char sndBuf[4];
  int i;

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(nr > TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((x == NULL) || (y == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Build the communication string */
  for(i=0; i<6; i++) {
    unsigned char base = TBAN_SER_SET_KANAL1 + (nr * 16);
    /* x-axis : temperature */
    sndBuf[0] = base+i;
    sndBuf[1] = 2*x[i]; /* Needs to be doubled since each step is 0.5 degrees */
    /* y-axis : pwm */
    sndBuf[2] = base+6+i;
    sndBuf[3] = y[i];
    /* Send it */
    CHECK_RESULT(tban_sendCommand(tban, sndBuf, 4));
    /* Update progress */
    tban_updateProgress(tban, i,6);
  }

  /* Set the temp value for last response curve point */
  sndBuf[0] = TBAN_SER_SET_MAX+nr;
  sndBuf[1] = 2*x[6];
  /* Send it */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  /* Update progress */
  tban_updateProgress(tban, i,6);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_setChHysteresis
 * Description : Set the channel hysteresis
 * Arguments   : tban       = The TBan struct to work on
 *               hysteresis =
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setChHysteresis(struct TBan* tban, unsigned char nr, unsigned char hysteresis) {
  unsigned char sndBuf[8];

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if(nr >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;

  /* Send the scaling factor command together with the new value */
  sndBuf[0] = TBAN_SER_SET_HYS + nr;
  sndBuf[1] = hysteresis;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_resetHardware
 * Description : Reset the TBan hardware. This means that the fans will
 * 		 increase rpm to maximum for a couple of seconds and
 * 		 then return to the previous mode and pwm. If using
 * 		 automatic mode before reset it will continue to be
 * 		 automatic.
 * Arguments   : tban = The TBan struct to work on.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_resetHardware(struct TBan* tban) {
  unsigned char sndBuf[8];

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Send the TBAN init command */
  sndBuf[0] = TBAN_SER_MAKE_ABGL;
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 1));

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_setChMode
 * Description : Set the operational mode for the channel.
 * Arguments   : tban    = The TBan struct to work on
 *               mode    = The mask for operaational mode (0=auto
 *                         1=manual). For example "31 = 1111" means
 *                         manual mode for all channels
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setChMode(struct TBan* tban, unsigned char modeMask) {
  unsigned char sndBuf[8];

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if(modeMask > 15)
    return TBAN_INDEX_OUT_OF_BOUNDS;

  /* Send the manual mode command */
  sndBuf[0] = TBAN_SER_MAN;
  sndBuf[1] = modeMask;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_setSensorScaleFact
 * Description : Set the channel scaling factor. This can prove useful
 * 		 to avoid oscillation if the temperature is fluctuating
 * 		 for a particular sensor.
 * Arguments   : tban    = The TBan struct to work on
 *               factor  = The scaling factor to use for this channel.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setSensorScaleFact(struct TBan* tban, unsigned char nr, unsigned char factor) {
  unsigned char sndBuf[8];

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if(nr >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;

  /* Send the scaling factor command together with the new value */
  sndBuf[0] = TBAN_SER_SKF + nr;
  sndBuf[1] = factor;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_setLED
 * Description : Set the LED status.
 * Arguments   : tban = The TBan struct to work on.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setLED(struct TBan* tban, unsigned char status) {
  unsigned char sndBuf[2];

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Send the TBAN init command */
  sndBuf[0] = status == 0 ? TBAN_SER_LED_AUS : TBAN_SER_LED_EIN;
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 1));
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_setBuz
 * Description : Set the buzzer status.
 * Arguments   : tban = The TBan struct to work on.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setBuz(struct TBan* tban, unsigned char status) {
  unsigned char sndBuf[2];

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Send the TBAN init command */
  sndBuf[0] = status == 0 ? TBAN_SER_BUZ_AUS : TBAN_SER_BUZ_EIN;
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 1));
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_getChannelMode
 * Description : Get the operational mode for the channel.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The fan index (0-indexed)
 *               mode    = Operational mode (0=auto 1=manual)
 *               startMode = The active mode when restarting the
 *               	     device.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getChMode(struct TBan* tban, unsigned char index, unsigned char* mode, unsigned char* startMode) {
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if(index >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if(mode == NULL)
    return TBAN_VALUE_NULL_PTR;

  /* Get mode */
  CHECK_RESULT(tban_getValue(tban, tban_getChModeMap[index], mode));
  CHECK_RESULT(tban_getValue(tban, tban_getChStartModeMap[index], startMode));

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_getChHysteresis
 * Description : Get the channel hysteresis
 * Arguments   : tban       = The TBan struct to work on
 *               index      = The fan index (0-indexed)
 *               hysteresis = The hysteresis value
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getChHysteresis(struct TBan* tban, unsigned char index, unsigned char* hysteresis) {
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if(index >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if(hysteresis == NULL)
    return TBAN_VALUE_NULL_PTR;

  /* Get Hysteresis for the selected channel  */
  CHECK_RESULT(tban_getValue(tban, tban_getHysteresisMap[index], hysteresis));

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_getChSensAssignment
 * Description : Get the sensor assignment for a specified channel.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The channel index (0-indexed)
 *               dsens   = The digital sensor map
 *               asens   = The analog sensor map
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getChSensAssignment(struct TBan* tban, unsigned char index, unsigned char* dsens, unsigned char* asens ) {
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if(index >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((dsens == NULL) || (asens == NULL))
    return TBAN_VALUE_NULL_PTR;

  /* Get sensor assignment */
  CHECK_RESULT(tban_getValue(tban, tban_getDsensorAssignMap[index], dsens));
  CHECK_RESULT(tban_getValue(tban, tban_getAsensorAssignMap[index], asens));

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_getMotionSettings
 * Description : Get the motion settings.
 *		 Put in short words, get all parameters that handle the
 *		 motion detection present in the Tban package. This will
 *		 in fact detect if any of the channels are stuck (the
 *		 fans do not spin) and increas the PWM to a level where
 *		 they start to spin, if they don't do that lets alarm
 *		 the user.
 * 
 * 		 Response from mCubed support page:
 * 		 Motion detection for PWM based controllers is very hard
 * 		 and because of this there was a water glass modell
 * 		 implemented:
 * 		 Every time the controller see a motion of a fan the
 * 		 internal counter (MES_LevelChangeCnt) will be increased
 * 		 (MES_ChUpEE) untill the max boarder is reached
 * 		 (MES_ChGrenzEE). In periodic intervalls the counter
 * 		 will be decreased automatically (MES_ChDownEE) untill
 * 		 0.
 * 		 This is only done if the motion recognition ot the
 * 		 channel was not overrided/disabled
 * 		 (MES_OverideBewegung)
 * 		 If the motion level is highter then 0 the MES_Betrieb
 * 		 flag is 1 and the controller will act on a blocking fan
 * 		 through increasing its output power and if this doesn't
 * 		 help through an alarm message.
 * 		 
 * Arguments   : tban      = The TBan struct to work on
 * 		 timeConst = Decrease the rotation counter "current"
 * 		 	     every time this time have passed.
 * 		 maxLimit  = Do not increase the counter "current"
 * 		 	     higher than this value.
 * 		 incrValue = Increase the counter "current" with this
 * 		 	     value everytime every time a fan rotation
 * 		 	     is detected.
 * 		 override  = Do not use the motion detection.
 * 		 rotate    = Flag deciding whether the fans are actually
 * 		 	     rotating for the channel.
 * 		 current   = The current (volatile) value of the motion
 * 		 	     estimation function. This is the value that
 * 		 	     will be increased by "incrValue" every time
 * 		 	     a fan rotation is detected.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_getMotionSettings(struct TBan* tban, unsigned char* timeConst, unsigned char* maxLimit,
                           unsigned char* incrValue, unsigned char override[], unsigned char rotate[],
                           unsigned char current[]) {
  int i;

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if((timeConst == NULL) || (maxLimit == NULL) || (incrValue == NULL) || (override == NULL) || (rotate == NULL) || (current == NULL))
    return TBAN_VALUE_NULL_PTR;

  /* Get values  */
  CHECK_RESULT(tban_getValue(tban, TBAN_MES_CH_DOWN_EE,  timeConst));
  CHECK_RESULT(tban_getValue(tban, TBAN_MES_CH_GRENZ_EE, maxLimit));
  CHECK_RESULT(tban_getValue(tban, TBAN_MES_CH_UP_EE,    incrValue));

  /* Get channel specific parameters */
  for(i=0; i<4; i++) {
    CHECK_RESULT(tban_getValue(tban, TBAN_MES_OVERIDEBEWEGUNG+i, &(override[i])));
    CHECK_RESULT(tban_getValue(tban, TBAN_MES_BETRIEB+i,         &(rotate[i])));
    CHECK_RESULT(tban_getValue(tban, TBAN_MES_LEVELCHANGECNT+i,  &(current[i])));
  }

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_setMotion
 * Description : Set motion (blockage) parameters
 * Arguments   : tban    = The TBan struct to work on
 *               lower   = Lower value for blockage recognition
 *               upper   = Upper value for blockage recognition
 *               error   = Values for error detection during blockage
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setMotion(struct TBan* tban, unsigned char lower, unsigned char upper, unsigned char error) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if((lower ==  0) || (upper == 0) || (error == 0))
    return TBAN_VALUE_NULL_PTR;
  
  /* Build command */
  sndBuf[0] = TBAN_SER_ERR_UP;
  sndBuf[1] = upper;
  sndBuf[2] = TBAN_SER_ERR_GRENZ;
  sndBuf[3] = error;
  sndBuf[4] = TBAN_SER_ERR_DOWN;
  sndBuf[5] = lower;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 6));

  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_setChSensAssignment
 * Description : Set the channel assignment for a specific channel.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The fan index (0-indexed)
 *               dsens   = The digital sensor map
 *               asens   = The analog sensor map
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setChSensAssignment(struct TBan* tban, unsigned char index, unsigned char dsens, unsigned char asens) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  
  /* Build command (digital and analog assignment) */
  sndBuf[0] = TBAN_SER_SET_ZUORD + index;
  sndBuf[1] = dsens;
  sndBuf[2] = TBAN_SER_SET_ZUORA + index;
  sndBuf[3] = asens;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 4));
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_setChPwm
 * Description : Manually set the pwm for a specified channel.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The fan index (0-indexed)
 *               pwm     = The pwm to set for the channel. Must be in
 *               	   the allowed interval (0..100)
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setChPwm(struct TBan* tban, unsigned char index, unsigned char pwm) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if(pwm > 100)
    return TBAN_VALUE_OUT_OF_BOUNDS;
  
  /* Send the manual mode command */
  sndBuf[0] = TBAN_SER_SET1 + index;
  sndBuf[1] = pwm;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_setChInitValue
 * Description : Manually set the initial value for a channel. This is
 * 		 the pwm that will be used when restarting the TBan in
 * 		 manual mode.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The fan index (0-indexed)
 *               pwm     = The pwm to set for the channel. Must be in
 *               	   the allowed interval (0..100)
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setChInitValue(struct TBan* tban, unsigned char index, unsigned char pwm) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if(pwm > 100)
    return TBAN_VALUE_OUT_OF_BOUNDS;

  /* Send the manual mode command */
  sndBuf[0] = TBAN_SER_INIT1 + index;
  sndBuf[1] = pwm;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_setPwmFreq
 * Description : Set the frequency used when adjusting the pwm
 * Arguments   : tban    = The TBan struct to work on
 *               freq    = Frequency (140 is default)
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setPwmFreq(struct TBan* tban, unsigned char freq) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Send the manual mode command */
  sndBuf[0] = TBAN_SER_FREQ;
  sndBuf[1] = freq;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_ping
 * Description : 
 * Arguments   : tban     = The TBan struct to work on.
 *               pingmask = The mask to apply
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_ping(struct TBan* tban, unsigned char pingmask) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Send the manual mode command */
  sndBuf[0] = TBAN_SENS_PING;
  sndBuf[1] = pingmask;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_getWatchdog
 * Description : Get watchdog settings.
 * 
 *		 Comments from mCubed:
 * 		 USB Wathdog is an internal timer that checks if
 * 		 communication works normal and resets the communication
 * 		 chip if no command arrives in a specific time (<10s) To
 * 		 activate the function send USB_WATCHDOG_ON or
 * 		 USB_WATCHDOG_OFF for deativating it. For permanent
 * 		 refresh/reset of the watchdeog timer send
 * 		 USB_WATCHDOG_ON or SER_REQUEST_1 or SER_REQUEST_2 in
 * 		 intervalls <10s, so no rest will occur. If a watchdog
 * 		 reset is in progress the status LED with flash
 * 		 rapidly, after 3 reset attemps the function will be
 * 		 disabled automatically.
 *
 * 		 Yes, it is present in the standard TBAN firmware 2.8 -
 * 		 but it will only work with the newer classic boards
 * 		 (its a additional hardware connection which is not
 * 		 present in the old XL and SL4 boards).
 * 
 * Arguments   : tban = The TBan struct to work on
 *   		 wdenabled = If the watchdog is enabled or not
 *   		 wd        = The actual value of the watchdog. 
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 *               TBAN_FW_TOO_OLD
 **********************************************************************/
int tban_getWatchdog(struct TBan* tban, unsigned char* wdenabled, unsigned char* wd) {
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if((wdenabled == NULL) ||(wd == NULL))
    return TBAN_VALUE_NULL_PTR;

  /* Check if supported by the device, must have firmware 2.8 or later
   * to use this function */
  CHECK_RESULT(tban_checkFw(tban, 28));

  /* Fetch values */
  CHECK_RESULT(tban_getValue(tban, TBAN_WD_ENABLED, wdenabled));
  CHECK_RESULT(tban_getValue(tban, TBAN_WD_COUNTER, wd));
  
  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_kickWatchdog
 * Description : Kick on the watchdog, the first time this is done the
 * 		 watchdog functionality will be enabled. All subsequent
 * 		 calls resets the watchdog timer. Currently it is set 10
 * 		 seconds before the TBan device is restarted.
 * Arguments   : tban = The TBan struct to work on.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 *               TBAN_FW_TOO_OLD
 **********************************************************************/
int tban_kickWatchdog(struct TBan* tban) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Check if supported by the device, must have firmware 2.8 or later
   * to use this function */
  CHECK_RESULT(tban_checkFw(tban, 28));

  /* Send the manual mode command */
  sndBuf[0] = USB_WATCHDOG_ON;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 1));
  
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_disableWatchdog
 * Description : Disable watchdog functionality. This should be used
 * 		 when exiting the application or when watchdog
 * 		 functionality is not needed anymore.
 * Arguments   : tban = The TBan struct to work on.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 *               TBAN_FW_TOO_OLD
 **********************************************************************/
int tban_disableWatchdog(struct TBan* tban) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Check if supported by the device, must have firmware 2.8 or later
   * to use this function */
  CHECK_RESULT(tban_checkFw(tban, 28));

  /* Send the manual mode command */
  sndBuf[0] = USB_WATCHDOG_OFF;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 1));
  
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : tban_setTacho
 * Description : Enable/disable the tacho signalling. This is the
 * 		 ability to detect fan motion. See
 * 		 tban_getMotionSettings() for more information of what
 * 		 can be read from the TBan in this case. When setting
 * 		 this to "1" the current value in
 * 		 tban_getMotionSettings() will start to count.
 * Arguments   : tban      = The TBan struct to work on.
 *               tachomask = The mask to apply
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int tban_setTacho(struct TBan* tban, unsigned char tachomask) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Send the manual mode command */
  sndBuf[0] = TBAN_SER_OVER;
  sndBuf[1] = tachomask;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}


/**********************************************************************
 * Name        : tban_checkFw
 * Description : Get the firmware version
 * Arguments   : tban = The TBan struct to work on.
 * 		 fw   = The FW version on integer format,
 * 		 	i.e. major-number*10+minor-number.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_FW_TOO_OLD
 **********************************************************************/
int tban_checkFw(struct TBan* tban, unsigned char fw) {
  unsigned char currentfw, fw_major, fw_minor;

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Get firmware version */
  currentfw = tban->buf[TBAN_INFO_VER];
  fw_major = (currentfw & (255-15)) >> 4; /* High nibble */
  fw_minor = currentfw & 15;              /* Low nibble */

  /* Compare them */
  if((fw_major*10+fw_minor) >=fw)
    return TBAN_OK;
  else
    return TBAN_FW_TOO_OLD;
}


