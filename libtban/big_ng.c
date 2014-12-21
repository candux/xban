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
 ** Filename:        big_ng.c
 ** Initial author:  marcus.jagemar@gmail.com
 **
 ** 
 ** DESCRIPTION
 ** -----------
 ** This is one part of the implementation of the TBan library. Please
 ** see the file tban.h for more information.
 **
 ** 
 *****************************************************************************/
 
#include "big_ng.h"
#include "common.h"



/*****************************************************************************
 * Command constants
 *****************************************************************************/

/* Setting the output mode for all channels in one call */
#define BIGNG_SER_OUTMODE          0x1C

/* Sensor assignment */
#define TBAN_SER_SET_ZUORNG	   0xE4

/* BigNG specific commands */
#define BIGNG_SER_SKF		   0x40
#define BIGNG_SER_ADDFK_AS         0x44
#define BIGNG_SER_ADDFK_DS         0x94
#define BIGNG_SER_ZT               0x30
#define BIGNG_SER_MODE             0x3A

/*****************************************************************************
 * System answer index values
 *****************************************************************************/

/* Output mode */
#define BIGNG_OUT_MODE         136 /* Code reviewed verif ok 2006-10-02 */

/* Overtemp indication */
#define BIGNG_SYS_OT           145 /* Code reviewed verif ok 2006-10-02 */

/* Sensor - channel assignement */
#define BIGNG_DSENS_ASSIGN             45
#define BIGNG_ASENS_ASSIGN             49
#define BIGNG_SPECIFIC_SENS_ASSIGN    164

/* BigNG analog sensors */
#define BIGNG_AS_CALIBRATED_VALUE     260
#define BIGNG_AS_RAW_VALUE            256
#define BIGNG_AS_SCALING_FACTOR       129
#define BIGNG_AS_ABS_SCALING_FACTOR   142
#define BIGNG_DS_ABS_SCALING_FACTOR   128
#define BIGNG_DS_SCALING_FACTOR        19
#define BIGNG_DS_CALIBRATED_VALUE     238
#define BIGNG_DS_RAW_VALUE            208
#define BIGNG_TARGET_TEMP             118
#define BIGNG_TARGET_MODE             122
#define BIGNG_PWM                     137
#define BIGNG_TEMP                    252
#define BIGNG_MODE                    101

static int bigNG_getChMaxPwmMapping[]   = { 148, 150, 152, 154 };


/**********************************************************************
 * Name        : bigNG_init
 * Description : Initialise the BigNG. This function is mainly
 *               performing admin stuff since it is always called when
 *               starting up a client, i.e. it is called even if a BigNG
 *               is not present in the system.
 * Arguments   : tban = To operate on.
 * Returning   : TBAN_OK
 **********************************************************************/
int bigNG_init(struct TBan* tban)  {
  int i;
 
  for(i=0; i<BIGNG_NUMBER_ADDITIONAL_ANALOG_SENSORS; i++) {
    char string[16];
    sprintf(string, "BigNG-AS%d", i);
    tban->bigNG.asName[i]  = malloc(strlen(string)+1);
    tban->bigNG.asDescr[i] = malloc(strlen(string)+1);
    strcpy(tban->bigNG.asName[i], string);
    strcpy(tban->bigNG.asDescr[i], string);
  }
  return TBAN_OK;
}




/**********************************************************************
 * Name        : bigNG_present
 * Description : Checks if the currently connected device is a
 *               BigNG. Should probably be used before calling any other
 *               function in this file since they will assume that the
 *               data they are working on is actually from a BigNG.
 * Arguments   : tban = The TBan struct to work on
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int bigNG_present(struct TBan* tban) {
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* */
  if(tban->buf[TBAN_INFO_TYPE] == TBAN_DEVICE_TYPE_BIGNG)
    return BIGNG_PRESENT;
  else
    return BIGNG_NOT_PRESENT;
}




/**********************************************************************
 * Name        : bigNG_getOutputMode
 * Description : Get the output mode for a channel
 * Arguments   : tban = The TBan struct to work on
 *               mode = The output mode array. (1 for analog Output, 0
 *                      for PWM output)
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int bigNG_getOutputMode(struct TBan* tban, unsigned char mode[]) {
  unsigned char modeval;
  unsigned int mask;
  int i;

  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  
  CHECK_RESULT(tban_getValue(tban, BIGNG_OUT_MODE, &modeval));

  for(i=0; i<4; i++) {
    mask = 1<<i;
    mode[i]=(modeval & mask) > 0 ? BIGNG_OUTPUT_MODE_ANALOG : BIGNG_OUTPUT_MODE_PWM;
  }
    
  return TBAN_OK;
}


  
/**********************************************************************
 * Name        : bigNG_getOvertemp
 * Description : Get the overtemp indication.
 * Arguments   : tban = The TBan struct to work on
 *               ot   = Overtemp indication
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int bigNG_getOvertemp(struct TBan* tban, unsigned char* ot) {
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(ot == NULL)
    return TBAN_VALUE_NULL_PTR;

  CHECK_RESULT(tban_getValue(tban, BIGNG_SYS_OT, ot));

  return TBAN_OK;
}


  

/**********************************************************************
 * Name        : bigNG_getChSensAssignment
 * Description : Get the sensor assignment for a specified channel.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The channel index (0-indexed)
 *               dsens   = The digital sensor map
 *               asens   = The analog sensor map
 *               bngsens = Extra sensors specific for bigNG (Only the four
 *                         lowest bits are used)
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int bigNG_getChSensAssignment(struct TBan* tban, unsigned char index, unsigned char* dsens, unsigned char* asens, unsigned char* bngsens) {
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  if(index >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((asens == NULL) || (dsens == NULL) || (bngsens == NULL))
    return TBAN_VALUE_NULL_PTR;

  /* Fetch all sensor information (The standars ones that are same as
   * TBan classic and the specific ones for BigNG) */
  CHECK_RESULT(tban_getValue(tban, BIGNG_DSENS_ASSIGN+index, dsens));
  CHECK_RESULT(tban_getValue(tban, BIGNG_ASENS_ASSIGN+index, asens));
  CHECK_RESULT(tban_getValue(tban, BIGNG_SPECIFIC_SENS_ASSIGN+index, bngsens));

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
int bigNG_setChSensAssignment(struct TBan* tban, unsigned char index, unsigned char dsens,
                              unsigned char asens, unsigned char bngsens) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if(bngsens > 15)
    return TBAN_VALUE_OUT_OF_BOUNDS;

  /* Build command (digital and analog assignment) */
  sndBuf[0] = TBAN_SER_SET_ZUORD + index;
  sndBuf[1] = dsens;
  sndBuf[2] = TBAN_SER_SET_ZUORA + index;
  sndBuf[3] = asens;
  sndBuf[4] = TBAN_SER_SET_ZUORNG + index;
  sndBuf[5] = bngsens;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 6));
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}


/**********************************************************************
 * Name        : bigNG_setOutputMode
 * Description : Set the frequency used when adjusting the pwm
 * Arguments   : tban     = The TBan struct to work on
 *               modemask = the mask to apply (1 for analog Output, 0
 *                          for PWM output). Since we have 4 channels
 *                          this value must be between 0 and 15
 *                          (decimal).
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int bigNG_setOutputMode(struct TBan* tban, unsigned char modemask) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(modemask > 15)
    return TBAN_VALUE_OUT_OF_BOUNDS;
  
  /* Send the manual mode command */
  sndBuf[0] = BIGNG_SER_OUTMODE;
  sndBuf[1] = modemask;

  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  
  /* Update progress */
  tban_updateProgress(tban, 1, 1);

  return TBAN_OK;
}


/**********************************************************************
 * Name        : bigNg_getaSensorTemp
 * Description : Get the sensor temp information for BigNG specific
 *               sensors, i.e. four additional sensors that can be
 *               attached to the BigNG additional t the ones supported
 *               by the original TBan.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The analog sensor index number (0-indexed)
 *               temp    = The double calibrated temperature value
 *               rawTemp = The double raw temperature value
 *               cal     = The relative calibration times 100
 *               abscal  = The double absolute calibration plus 100
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int bigNG_getaSensorTemp(struct TBan* tban, int index, unsigned char* temp, unsigned char* rawTemp, unsigned char* cal, unsigned char* abscal) {
  /* argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= BIGNG_NUMBER_ADDITIONAL_ANALOG_SENSORS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((temp == NULL) || (rawTemp == NULL) || (cal == NULL) || (abscal==NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get the temperature of the analog sensor */
  CHECK_RESULT(tban_getValue(tban, BIGNG_AS_CALIBRATED_VALUE + index, temp));
  CHECK_RESULT(tban_getValue(tban, BIGNG_AS_RAW_VALUE + index, rawTemp));
  CHECK_RESULT(tban_getValue(tban, BIGNG_AS_SCALING_FACTOR + index, cal));
  CHECK_RESULT(bigNG_getValue(tban, BIGNG_AS_ABS_SCALING_FACTOR + index, abscal));

  return TBAN_OK;

}


/**********************************************************************
 * Name        : bigNg_getdSensorTemp
 * Description : Get the sensor temp information for BigNG digital
 *               sensors
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The analog sensor index number (0-indexed)
 *               temp    = The double calibrated temperature value
 *               rawTemp = The double raw temperature value
 *               cal     = The relative calibration times 100
 *               abscal  = The double absolute calibration plus 100
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/

int bigNG_getdSensorTemp(struct TBan* tban, int index, unsigned char* temp, unsigned char* rawTemp, unsigned char* cal, unsigned char * abscal) {
  /* argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= TBAN_NUMBER_DIGITAL_SENSORS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((temp == NULL) || (rawTemp == NULL) || (cal==NULL) || (abscal==NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get the temperature of the digital sensor */
  CHECK_RESULT(tban_getValue(tban, BIGNG_DS_CALIBRATED_VALUE + index, temp));
  CHECK_RESULT(tban_getValue(tban, BIGNG_DS_RAW_VALUE + index, rawTemp));
  CHECK_RESULT(tban_getValue(tban, BIGNG_DS_SCALING_FACTOR + index, cal));
  CHECK_RESULT(bigNG_getValue(tban, BIGNG_DS_ABS_SCALING_FACTOR + index, abscal));

  return TBAN_OK;
}

/**********************************************************************
 * Name        : bigNG_queryStatus
 * Description : Query the BigNG about the current status. This will
 *               effectively update the local cache with fresh
 *               information about the second status vector. 
 * Arguments   : tban = The TBan structure.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 *               TBAN_ERECEIVE
 *               TBAN_CORRUPT_DATA
 **********************************************************************/

int bigNG_queryStatus(struct TBan* tban) {
  unsigned char sndBuf[8];
  
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;
  
  /* Send the query command. Make sure that the command is sent to the
   * TBan itself and not to the add-on modules such as miniNG. */
  sndBuf[0] = TBAN_SER_SOURCE2;
  sndBuf[1] = TBAN_SER_REQUEST;
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  
  /* Receive the result from the HW */
  CHECK_RESULT(tban_readData(tban, tban->bigNG.buf, 285));

  if(bigNG_dataPresent(tban) != TBAN_OK) {
    return TBAN_CORRUPT_DATA;
  }
  
  /*Update the time stamp for the last update, but only if we suceeded
   * with the update */
  tban->bigNG.lastQuery = time(NULL);

  return TBAN_OK;
}

/**********************************************************************
 * Name        : bigNG_dataPresent
 * Description : Checks the data in the second status vector
 * Arguments   : tban  = The TBan structure
 * Returning   : TBAN_OK
 *               TBAN_CORRUPT_DATA
 *               TBAN_STRUCT_NULL_PTR
 **********************************************************************/
int bigNG_dataPresent(struct TBan *tban){
 /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;

  /* Make some simple checks on the returned vector. Like that it
   * contains the value "100" in the first position */
  if(tban->bigNG.buf[0] != 100) {
    return TBAN_CORRUPT_DATA;
  }
  return TBAN_OK;
}

/**********************************************************************
 * Name        : bigNG_getValue
 * Description : Return the status (in the second vector) for the index 
 *               supplied. This function
 *               call requires that bigNG_queryStatus has been called
 *               before.
 * Arguments   : tban  = The TBan structure
 *               index = The index in the second status vector read from the
 *               BigNG  HW.
 *               value = The value to be returned to the caller, the
 *                       corresponding value to the index supplied.
 * Returning   : TBAN_OK
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_NOT_OPENED
 **********************************************************************/
int bigNG_getValue(struct TBan* tban, int index, unsigned char* value) {
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(value == NULL)
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Return a value to the caller */
  *value = tban->bigNG.buf[index];
  return TBAN_OK;
}

/**********************************************************************
 * waine
 * Name        : bigNG_setAsScalingFact
 * Description : Set the analog sensor scaling factor.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The sensor index
 *               fact    = The scaling factor to use for this sensor.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 **********************************************************************/
int bigNG_setAsScalingFact(struct TBan* tban, unsigned char index, unsigned char fact) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= BIGNG_NUMBER_ADDITIONAL_ANALOG_SENSORS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
   
  /* Send the manual mode command */
  sndBuf[0] = BIGNG_SER_SKF+index;
  sndBuf[1] = fact;
  
  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  
  /* Update progress */
  tban_updateProgress(tban, 1, 1);
  
  return TBAN_OK;
}

/**********************************************************************
 * waine
 * Name        : bigNG_setAsAbsScalingFact
 * Description : Set the analog sensor absolute scaling factor.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The sensor index
 *               fact    = The absolute scaling factor to use for this sensor.
 *                         Actually the expected value is 2*factor+100, where 'factor' 
 *                         is signed but the resulting value is always positive.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 **********************************************************************/
int bigNG_setAsAbsScalingFact(struct TBan* tban, unsigned char index, unsigned char fact) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= BIGNG_NUMBER_ADDITIONAL_ANALOG_SENSORS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
   
  /* Send the manual mode command */
  sndBuf[0] = BIGNG_SER_ADDFK_AS+index;
  sndBuf[1] = fact;
  
  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  
  /* Update progress */
  tban_updateProgress(tban, 1, 1);
  
  return TBAN_OK;
}


/**********************************************************************
 * Name        : bigNG_setDsAbsScalingFact
 * Description : Set the digital sensor absolute scaling factor.
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The sensor index
 *               fact    = The absolute scaling factor to use for this sensor.
 *                         Actually the expected value is 2*factor+100, where 'factor' 
 *                         is signed but the resulting value is always positive.
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 **********************************************************************/
int bigNG_setDsAbsScalingFact(struct TBan* tban, unsigned char index, unsigned char fact) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >=  TBAN_NUMBER_DIGITAL_SENSORS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  
  /* Send the manual mode command */
  sndBuf[0] = BIGNG_SER_ADDFK_DS+index;
  sndBuf[1] = fact;
  
  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  
  /* Update progress */
  tban_updateProgress(tban, 1, 1);
  
  return TBAN_OK;
}


/**********************************************************************
 * Name        : bigNG_setChTargetTemp
 * Description : Set the channel target temperature for target working mode (PID).
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The channel index
 *               fact    =  The target temperature
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 **********************************************************************/
int bigNG_setChTargetTemp(struct TBan* tban, unsigned char index, unsigned char target) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >=  TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
    
  /* Send the manual mode command */
  sndBuf[0] = BIGNG_SER_ZT+index;
  sndBuf[1] = 2*target;
  
  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  
  /* Update progress */
  tban_updateProgress(tban, 1, 1);
  
  return TBAN_OK;
}


/**********************************************************************
 * Name        : bigNG_setChTargetMode
 * Description : Set the channel target working mode (PID).
 * Arguments   : tban    = The TBan struct to work on
 *               index   = The channel index
 *               fact    = The target working mode
 *                         0 --> controlled by the response curve
 *                         1..5--> target mode (PID) with predefined settings
 *                                 (I don't know which is the difference between 
 *                                  these settings. One may try to look at Windows 
 *                                  software, but I do not have Windows, or to ask
 *                                  on the TBalancer forum)
 *                         6 --> Target mode with user settings 
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 **********************************************************************/
int bigNG_setChTargetMode(struct TBan* tban, unsigned char index, unsigned char mode) {
  unsigned char sndBuf[8];
  
  /* Argument sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >=  TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if(mode > BIGNG_MAX_TARGET_MODE)
    return TBAN_INDEX_OUT_OF_BOUNDS;

  /* Send the manual mode command */
  sndBuf[0] = BIGNG_SER_MODE+index;
  sndBuf[1] = mode;
  
  /* Perform the command */
  CHECK_RESULT(tban_sendCommand(tban, sndBuf, 2));
  
  /* Update progress */
  tban_updateProgress(tban, 1, 1);
  
  return TBAN_OK;
}


/**********************************************************************
 * Name        : bigNG_getChInfo
 * Description : Get fan information
 * Arguments   : tban    = The TBan struct
 *               index   = The fan index number (0-indexed)
 *               rpmMax  = The maximum rpm possible for the current
 *                         fan.
 *               pwm     = The percentage of maximum throttle.
 *               resTemp = The temperatur for the current fan
 *                         (Centigrade)
 *               mode    = 0:automatic mode ; 1=manual mode
 *               target  = The target temperature
 *               targetmode = The target working mode
 * Returning   : TBAN_OK
 *               TBAN_STRUCT_NULL_PTR
 *               TBAN_VALUE_NULL_PTR
 *               TBAN_INDEX_OUT_OF_BOUNDS
 *               TBAN_NOT_OPENED
 **********************************************************************/
int bigNG_getChInfo(struct TBan* tban, int index, unsigned int* rpmMax, unsigned char* pwm, unsigned char* resTemp, unsigned char* mode, unsigned char* target, unsigned char* targetmode) {
  unsigned char hb, lb;
  
  /* Sanity check */
  if(tban == NULL)
    return TBAN_STRUCT_NULL_PTR;
  if(index >= TBAN_NUMBER_CHANNELS)
    return TBAN_INDEX_OUT_OF_BOUNDS;
  if((rpmMax == NULL) || (pwm == NULL) || (resTemp == NULL) || (mode == NULL)
     || (target == NULL) || ( targetmode == NULL))
    return TBAN_VALUE_NULL_PTR;
  if(tban->opened == 0)
    return TBAN_NOT_OPENED;

  /* Get the maximum pwm */
  CHECK_RESULT(tban_getValue(tban, bigNG_getChMaxPwmMapping[index], &lb));
  CHECK_RESULT(tban_getValue(tban, bigNG_getChMaxPwmMapping[index]+1, &hb));

  /* Calculate the maximum rpm  */
  *rpmMax = (unsigned int) (((float) 256*hb + lb) * (float) 10.5);

  /* Get the pwm */
  if(index >= TBAN_NUMBER_CHANNELS) {
    return TBAN_INDEX_OUT_OF_BOUNDS;
  }

  /* Get the pwm */
  CHECK_RESULT(tban_getValue(tban, BIGNG_PWM + index, pwm));

  /* The pwm needs to be corrected to be valid according to the max-rpm */
  (*pwm) = 2*(*pwm);

  /* Get temp. Strangely enough the european product TBan returns the
   * temperature in Farenheit so we need to convert it to Celsius... */
  CHECK_RESULT(tban_getValue(tban, BIGNG_TEMP + index, resTemp));

  /* Get mode */
  CHECK_RESULT(tban_getValue(tban, BIGNG_MODE + index, mode));
  CHECK_RESULT(tban_getValue(tban, BIGNG_TARGET_TEMP + index, target));
  CHECK_RESULT(tban_getValue(tban, BIGNG_TARGET_MODE + index, targetmode));

  return TBAN_OK;
}
