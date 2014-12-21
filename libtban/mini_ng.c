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
 ** Filename:        mini_ng.c
 ** Initial author:  marcus.jagemar@gmail.com
 **
 ** 
 ** DESCRIPTION
 ** -----------
 ** This is the implementation of the TBan library. Please see the
 ** file tban.h for more information.
 **
 ** 
 *****************************************************************************/
 
#include "mini_ng.h"
#include "common.h"
#include "tban_hw_def.h"

/*****************************************************************************
 * How long should the application wait after each command has been sent
 * to the miniNG. According to mCubed 600ms should be enough. The unit
 * is nano seconds.
 *****************************************************************************/
#define MINI_NG_COMMAND_DELAY_S  0            /* seconds */
#define MINI_NG_COMMAND_DELAY_NS 250000000    /* nano seconds */


/*****************************************************************************
 * MiniNG command codes
 *****************************************************************************/
#define MINI_NG_SETCURVE1       0x30
#define MINI_NG_SETCURVE2       0x40


/*****************************************************************************
 * NOTE: Compared to the documentation all values in these tables are
 * incremented by 1 since we use a different indexing system. In this
 * lib we start the resulting vector with the "100" value. In the
 * documentation this value is omitted.
 *****************************************************************************/

#define MINI_NG_STATUS             2
#define MINI_NG_JUMPER             3
#define MINI_NG_POT1               4
#define MINI_NG_POT2               5

#define MINI_NG_UEBERTEMP1        18
#define MINI_NG_UEBERTEMP2        19

#define MINI_NG_TIMEBASE          57

#define MINI_NG_HYSTERESE_CH1     58
#define MINI_NG_HYSTERESE_CH2     59

/* Frame information */
#define MINI_NG_START_TWI         1
#define MINI_NG_END_TWI           62

/* Pass through buffer in TBan. These are the values that are currently
 * being sent to the miniNG. When these are all 0 we can safely deduce
 * that the TBan is ready for new frames. */
#define MINI_NG_TBAN_BUFFER_B1    63
#define MINI_NG_TBAN_BUFFER_B2    64
#define MINI_NG_TBAN_BUFFER_B3    65

/* Channel calibration */
#define MINI_NG_ABSCAL1 	  77
#define MINI_NG_ABSCAL2           78


/* Channel response curve mappings */
static int miniNG_getChCurveXMap[]        = { 20, 30 };
static int miniNG_getChCurveYMap[]        = { 25, 35 };

/* RPM reading mappings */
static int miniNG_getChRpmMap[]           = { 44, 46 };
static int miniNG_getChMaxRpmMap[]        = { 45, 47 };

/* Temp reading mappings */
static int miniNG_getChTempMap[]          = { 6, 7 };
static int miniNG_getChCalTempMap[]       = { 8, 9 };



/**********************************************************************
 * Name        : miniNG_init
 * Description : Initialise the miniNG. This function is mainly
 * 		 performing admin stuff since it is always called when
 * 		 starting up a client, i.e. it is called even if a miniNG
 * 		 is not present in the system.
 * Arguments   : tban = To operate on.
 * Returning   : TBAN_OK
 **********************************************************************/
int miniNG_init(struct TBan* tban)  {
  int i;
 
  for(i=0; i<MINI_NG_NUMBER_ANALOG_SENSORS; i++) {
    char string[16];
    sprintf(string, "miniNG-AS%d", i);
    tban->miniNG.asName[i]  = malloc(strlen(string)+1);
    tban->miniNG.asDescr[i] = malloc(strlen(string)+1);
    strcpy(tban->miniNG.asName[i], string);
    strcpy(tban->miniNG.asDescr[i], string);
  }
  for(i=0; i<MINI_NG_NUMBER_CHANNELS; i++) {
    char string[16];
    sprintf(string, "miniNG-ch%d", i);
    tban->miniNG.chName[i]  = malloc(strlen(string)+1);
    tban->miniNG.chDesc[i] = malloc(strlen(string)+1);
    strcpy(tban->miniNG.chName[i], string);
    strcpy(tban->miniNG.chDesc[i], string);
  }
  return TBAN_OK;
}


/**********************************************************************
 * Name        : miniNG_present
 * Description : Checks if the currently connected device is a
 *               miniNG. Should probably be used before calling any other
 *               function in this file since they will assume that the
 *               data they are working on is actually from a miniNG.
 * Arguments   : tban = The TBan struct to work on
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int miniNG_present(struct TBan* tban) {
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Check if the miniNG is present in the current setup */
  if((tban->miniNG.buf[0] != 100) ||
     (tban->miniNG.buf[MINI_NG_START_TWI] != 253) ||
     (tban->miniNG.buf[MINI_NG_END_TWI] != 254)) {
    return MINING_NOT_PRESENT;
  }

  return MINING_PRESENT;
}


/**********************************************************************
 * Name        : miniNG_strstat
 * Description : Convert the miniNG integer status message to a human
 *               readable format.
 * Arguments   : code = The error code to convert.
 * Returning   : The stringular error text of the error or NULL if not
 *               found in the array of known faults.
 **********************************************************************/
char* miniNG_strstat(unsigned int code) {
  int i;
  for(i=0; i<sizeof(MiniNG_statusText); i++) {
    if(MiniNG_statusText[i].code==code) {
      return MiniNG_statusText[i].text;
    }
  }
  return NULL;
}



/**********************************************************************
 * Name        : miniNG_sendCommand
 * Description : Send a miniNG command
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
int miniNG_sendCommand(struct TBan* tban, unsigned char* sndBuf, int cmdLen) {
  int result = tban_sendCommand(tban, sndBuf, cmdLen);
  if(result == TBAN_OK) {
    /* Wait a while so that the command can safelt be clocked into the
     * miniNG. Please note that since the tban_sendCommand() function
     * also waits for a short while we shouldn't really need this long
     * delay but typically for miniNG we need 600ms and the tban only
     * needs 25ms so we disregard this fact for the time being. */
    local_nanosleep(MINI_NG_COMMAND_DELAY_S, MINI_NG_COMMAND_DELAY_NS);
  }
  return result;
}



/**********************************************************************
 * Name        : miniNG_getValue
 * Description : Return the status for the index supplied. This function
 *               call requires that miniNG_queryStatus has been called
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
int miniNG_getValue(struct TBan* tban, int index, unsigned char* value) {
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(value == NULL)
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Return a value to the caller */
  *value = tban->miniNG.buf[index];
  return TBAN_OK;
}



/**********************************************************************
 * Name        : miniNG_queryStatus
 * Description : Query the status vector from the miniNG. This is
 *               equivalent to the tban_queryStatus function for the
 *               generic TBan.
 * Arguments   : tban = The TBan structure.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 *               TBAN_ERECEIVE
 *               TBAN_CORRUPT_DATA
 **********************************************************************/
int miniNG_queryStatus(struct TBan* tban) {
  unsigned char sndBuf[8];
  unsigned char buf[285];

  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Change the USB serial address to the miniNG (second port) and send
   * the query command */
  sndBuf[0] = TBAN_SER_SOURCE2;
  sndBuf[1] = TBAN_SER_REQUEST;
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));

  /* Receive the result from the HW */
  CHECK_RESULT(tban_readData(tban, buf, 285));
  memcpy(tban->miniNG.buf, buf, 128);

  /* Make some simple checks on the returned vector. Like that it
   * contains the value "100" in the first position */
  if((tban->miniNG.buf[0] != 100) ||
     (tban->miniNG.buf[MINI_NG_START_TWI] != 253) ||
     (tban->miniNG.buf[MINI_NG_END_TWI] != 254)) {
    return TBAN_CORRUPT_DATA;
  }

  /* Update the time stamp for the last update, but only if we suceeded
   * with the update */
  tban->miniNG.lastQuery = time(NULL);

  return TBAN_OK;
}



/**********************************************************************
 * Name        : miniNG_getaSensorTemp
 * Description : Get the temperature for the given sensor. Please note
 * 		 that the temperature needs to be divided by two to get
 * 		 the actual temp.
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
int miniNG_getaSensorTemp(struct TBan* tban, int index, unsigned char* temp, unsigned char* rawTemp, unsigned char* cal) {
  /* argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= MINI_NG_NUMBER_ANALOG_SENSORS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((temp == NULL) || (rawTemp == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get the temperature of the digital sensor */
  CHECK_RESULT(miniNG_getValue(tban, miniNG_getChTempMap[index], temp));
  CHECK_RESULT(miniNG_getValue(tban, miniNG_getChCalTempMap[index], rawTemp));
  CHECK_RESULT(miniNG_getValue(tban, MINI_NG_ABSCAL1+index, cal));
  
  return TBAN_OK;
}



/**********************************************************************
 * Name        : miniNG_getChRpm
 * Description : Get the rpm count for the given channel
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The analog sensor index number (0-indexed)
 *               rpm     = The current rpm
 *               rpmMax  = The maximum rpm
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int miniNG_getChRpm(struct TBan* tban, int index, unsigned char* rpm, unsigned char* rpmMax) {
  /* argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= MINI_NG_NUMBER_ANALOG_SENSORS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((rpm == NULL) || (rpmMax == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get the temperature of the digital sensor */
  CHECK_RESULT(miniNG_getValue(tban, miniNG_getChRpmMap[index], rpm));
  CHECK_RESULT(miniNG_getValue(tban, miniNG_getChMaxRpmMap[index], rpmMax));

  return TBAN_OK;
}


/**********************************************************************
 * Name        : miniNG_getChHysteresis
 * Description : Get the channel hysteresis
 * Arguments   : tban       = The TBan struct to work on
 *               index      = The fan index (0-indexed)
 *               hysteresis = The hysteresis value
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int miniNG_getChHysteresis(struct TBan* tban, unsigned char index, unsigned char* hysteresis) {
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if(index >= MINI_NG_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if(hysteresis == NULL)
    return TBAN_VALUE_NULL_PTR;

  /* Get Hysteresis for the selected channel  */
  CHECK_RESULT(miniNG_getValue(tban, MINI_NG_HYSTERESE_CH1+index, hysteresis));

  return TBAN_OK;
}


/**********************************************************************
 * Name        : miniNG_getChOverTemp
 * Description : Get the over temperature limit for the given channel.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The analog sensor index number (0-indexed)
 *               temp    = The double value for overtemp.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int miniNG_getChOverTemp(struct TBan* tban, int index, unsigned char* temp) {
  /* argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= MINI_NG_NUMBER_ANALOG_SENSORS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if(temp == NULL)
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get the current over temperature defined for the channel */
  CHECK_RESULT(miniNG_getValue(tban, MINI_NG_UEBERTEMP1+index, temp));

  return TBAN_OK;
}



/**********************************************************************
 * Name        : miniNG_getChCurve
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
int miniNG_getChCurve(struct TBan* tban, int index, unsigned char x[], unsigned char y[]) {
  int i;

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= MINI_NG_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((x == NULL) || (y == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  x[0]=0;
  y[5]=100;

  /* Obtain the response curve for the fan */
  for(i=0; i<5; i++) {
    unsigned char value;
    /* Get the temperature */
    CHECK_RESULT(miniNG_getValue(tban, miniNG_getChCurveXMap[index]+i, &value));
    x[i+1] = value/2;
    
    /* Get the requested pwm */
    CHECK_RESULT(miniNG_getValue(tban, miniNG_getChCurveYMap[index]+i, &(y[i])));
  }

  return TBAN_OK;
}



/**********************************************************************
 * Name        : miniNG_getHwInfo
 * Description : Get basic hardware information for the miniNG
 *               connected.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The fan index (0-indexed)
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int miniNG_getHwInfo(struct TBan* tban, unsigned char* status, unsigned char* jumper, unsigned char* pot1, unsigned char* pot2, unsigned char* timebase) {
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if((status == NULL) || (jumper == NULL) ||
     (pot1 == NULL) || (pot2 == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get all info */
  CHECK_RESULT(miniNG_getValue(tban, MINI_NG_STATUS, status));
  CHECK_RESULT(miniNG_getValue(tban, MINI_NG_JUMPER, jumper));
  CHECK_RESULT(miniNG_getValue(tban, MINI_NG_POT1, pot1));
  CHECK_RESULT(miniNG_getValue(tban, MINI_NG_POT2, pot2));
  CHECK_RESULT(miniNG_getValue(tban, MINI_NG_TIMEBASE, timebase));

  return TBAN_OK;
}



/**********************************************************************
 * Name        : miniNG_setChCurve
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
int miniNG_setChCurve(struct TBan* tban, int nr, unsigned char x[], unsigned char y[]) {
  unsigned char sndBuf[16];
  int result;
  int i;

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(nr > MINI_NG_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((x == NULL) || (y == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Build the communication string */
  for(i=0; i<5; i++) {
    /* Calculate the function to use. The base address for the channel
     * is MINI_NG_SETCURVE1 and the next channel is located 0x10 above. */
    unsigned char base = MINI_NG_SETCURVE1 + (nr * 0x10);

    /* Add the command to the vector */
    sndBuf[0] = TBAN_SER_MINI_S1;
    sndBuf[1] = base+i;

    /* x-axis : temperature */
    sndBuf[2] = TBAN_SER_MINI_S2;
    sndBuf[3] = 2*x[i]; /* Needs to be doubled since each step is 0.5
                         * degrees */

    /* y-axis : pwm */
    sndBuf[4] = TBAN_SER_MINI_S2_2;
    sndBuf[5] = y[i];

    /* Flush the send buffer to miniNG */
    sndBuf[6] = TBAN_SER_MINI_SEND1;
    
    /* Send it */
    CHECK_RESULT(miniNG_sendCommand(tban, sndBuf, 7));

    /* Poll the miniNG and wait here until we see that the internal
     * pass-through buffer is 0. This means that the frame sent above
     * has been handled by the miniNG and that we can continue to send
     * the next frame. */
    result = miniNG_queryStatus(tban);
    while((tban->miniNG.buf[MINI_NG_TBAN_BUFFER_B1] != 0) ||
          (tban->miniNG.buf[MINI_NG_TBAN_BUFFER_B2] != 0) ||
          (tban->miniNG.buf[MINI_NG_TBAN_BUFFER_B3] != 0)) {
      /* Frame not transmitted yet, lets wait a while and update the
       * status vector for new checks. */
      local_nanosleep(MINI_NG_COMMAND_DELAY_S, MINI_NG_COMMAND_DELAY_NS);
      result = miniNG_queryStatus(tban);
    }

    /* Update progress */
    tban_updateProgress(tban, i,4);
  }
  
  /* Set the curve */
  return TBAN_OK;
}




