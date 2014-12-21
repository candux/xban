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
 ** Filename:        mini_ng.h
 ** Initial author:  marcus.jagemar@gmail.com
 **
 ** 
 ** CURRENTLY WORKING STUFF
 ** -----------------------
 ** - Query miniNG status 
 ** - Get channel temperature
 ** - Get channel response curve
 **
 ** 
 ** REMAINING ISSUES
 ** ----------------
 ** - This code only works on one miniNG, the second cannot be
 **   communicated with. Should be quite simple to add support for this
 **   but needs some restructuring in the parameter sending to
 **   functions.
 ** 
 ** MARKETING INFO FROM MCUBED
 ** --------------------------
 ** Just put here so you can get an idea of what the BigNG supports
 ** compared to the older ones.
 ** 
 ** Taken from: http://www.t-balancer.com/english/index.htm
 **
 ** Separate control of 2 fan channels
 **  * 50W per channel with PWM, combined 100W
 **  * 20W per channel with analogue, combined 25W
 **  * 2 analogue sensors
 **  * kick-start for water pumps
 **  * configuration with jumpers and potentiometers
 **  * free configuration with T-Balancer and its software
 **  * integrated overheat protection in analogue operation
 ** 
 ** The heart of the miniNG is a powerfull microprocessor with
 ** integrated permanent memory. The processor enables a lot of
 ** configuration possibilities and the connection to the
 ** T-Balancer. All configurations are saved to the memory.
 ** 
 ** The miniNG works completely autonom and can also be operated without
 ** the T-Balancer. But if you own a T-Balancer you can make all
 ** configurations with Software and you can create new profiles and
 ** save them to the miniNG.
 **
 **
 ** 
 ** REVISION HISTORY
 ** ----------------
 ** 
 ** Date        Comment
 ** =================================================================
 ** 2006-07-21  General improvements:
 **             - Support for miniNG added.
 **             Fixed bugs:
 **             - bug in tban_readData fixed when receiving a longer
 **               buffer than expected. Caused buffer overrun problem.
 **             Added functions:
 **             - miniNG_queryStatus
 **             - miniNG_getChTemp
 **             - miniNG_getChCurve
 ** 2006-07-25 Added functions:
 **            - miniNG_getChRpm
 **            - miniNG_setChCurve
 **            - miniNG_getChHysteresis
 **            General improvements:
 **            - Added callback function when sending large amount of
 **              data to the miniNG.
 ** 2006-07-26 First version after release: libtban-0.3
 ** 2006-07-28 General improvements
 **  	       - Add better delay handling when sending frames to the
 **  	         miniNG. Removed the static delay and implemented a
 **  	         check for B1-B3 bytes in the system response.
 **  	       - miniNG_gethwinfo (Added timebase information)
 **
 *****************************************************************************/

#ifndef __MINI_NG_H
#define __MINI_NG_H

#include "tban.h"
#include "tban_hw_def.h"


/*****************************************************************************
 * BigNG present (or not) constants
 *****************************************************************************/
#define MINING_PRESENT                    1
#define MINING_NOT_PRESENT                0


/*****************************************************************************
 * Status values
 *****************************************************************************/
typedef struct  {
  unsigned char code;
  char* text;
} MiniNG_statusTextStruct;
 
static const MiniNG_statusTextStruct MiniNG_statusText[] = {
  { 0,  "nothing" },
  { 1,  "remote control active channel A" },
  { 2,  "remote control active channel B" },
  { 3,  "remote control active both channels" },
  { 4,  "no sensor found at channel A" },
  { 5,  "no sensor found at channel B" },
  { 6,  "no sensor found on both channels" },
  { 7,  "RPM fail at channel A" },
  { 8,  "RPM fail at channel B" },
  { 9,  "RPM fail at both channels" },
  { 10, "fan block at channel A" },
  { 11, "fan block at channel B " },
  { 12, "fan block at both channels" },
  { 13, "channel 1 overtemp" },
  { 14, "channel 2 overtemp" },
  { 15, "both channels overtemp" }
};




/*****************************************************************************
 * Exported MiniNG functions
 *****************************************************************************/

/* miniNG functions */
int miniNG_init(struct TBan* tban);
int miniNG_queryStatus(struct TBan* tban);
int miniNG_getHwInfo(struct TBan* tban, unsigned char* status, unsigned char* jumper, unsigned char* pot1, unsigned char* pot2, unsigned char* timebase);

/* Error management functions */
char* miniNG_strstat(unsigned int code);

/* Channel getters */
int miniNG_getaSensorTemp(struct TBan* tban, int index, unsigned char* temp, unsigned char* rawTemp, unsigned char* cal);
int miniNG_getChRpm(struct TBan* tban, int index, unsigned char* rpm, unsigned char* rpmMax);
int miniNG_getChCurve(struct TBan* tban, int index, unsigned char x[], unsigned char y[]);
int miniNG_getChOverTemp(struct TBan* tban, int index, unsigned char* temp);
int miniNG_getChHysteresis(struct TBan* tban, unsigned char index, unsigned char* hysteresis);

/* Channel setters */
int miniNG_setChCurve(struct TBan* tban, int nr, unsigned char x[], unsigned char y[]);

#endif /* __MINI_NG_H */



