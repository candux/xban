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
 ** Filename:        big_ng.h
 ** Initial author:  marcus.jagemar@gmail.com
 **
 ** 
 ** MARKETING INFO FROM MCUBED
 ** --------------------------
 ** Just put here so you can get an idea of what the BigNG supports
 ** compared to the older ones.
 ** 
 ** Taken from: http://www.t-balancer.com/english/support.htm
 **
 ** The T-Balancer bigNG is the first 4-channel controller for air and
 ** watercooling with patented Dual Mode Technology (DMT). You can
 ** choose whether you want to power each channel analog or with
 ** PWM. The T-Balancer bigNG delivers pure analog voltage up to 20W per
 ** channel (40W with PWM). This way you can even connect water pumps or
 ** Peltiers. An integrated heatsink ensures a cool running.
 ** 
 ** The bigNG will be delivered by standard with 4 thin analogue sensors
 ** and 2 precise digital sensors. Due to the high absolute accuracy of
 ** the digital sensors you can calibrate the rest of your system to
 ** it. Using all available extension you can control all together 10
 ** analogue sensors, 8 digital sensors and 2 flowmeters.
 ** 
 ** Mounting is very flexible: in a free 3.5" bay, in a free chassis
 ** slot or anywhere with the orange fluorescent acrylic glass.
 ** 
 ** What can the T-Balancer bigNG control? The bigNG can read up to 10
 ** analogue sensors, 8 digital sensors, 2 flowmeters and the speed of 4
 ** fans. It can control fans, powerful waterpumps, CCFLs and even
 ** Peltiers. It can be connected to the PC with USB. The RPM signals
 ** can also be connected. With the Extension Set analogue, the PC can
 ** also be shut-off in a case of emergency.
 ** 
 ** Which extensions exist for the T-Balancer bigNG?  Extension set
 ** digital (6 more digital sensors), extension set analogue (6 more
 ** analogue sensors, emergency PC power off, connector for 2
 ** flowmeter), 2 T-Balancer miniNG (more fan channels), Waterkit (with
 ** flowmeter and water temp probes), filling-leve meter (in
 ** development), Multi-IO with 16 In- and Outputs (in development)
 ** 
 ** The T-Balancer bigNG offers numerous safety systems: overheat
 ** protection, acoustic alarm, optic alarm, software alarms, sensor
 ** loss control, blockage recognition, self-sustaining system
 ** 
 ** With Software (via USB2.0) the T-Balancer can be adjusted and
 ** configured in an easy way. Many Plug-Ins (iMON, Speedfan, MBM,
 ** JaLCD, LCDC, Samurize) are provided for the T-Balancer software. The
 ** software also enables long-term monitoring, an integrated USB
 ** watch-dog ensures a permanent USB connection.
 ** 
 ** Package content: control unit, 2 digital sensors (address #0-#1), 4
 ** analogue sensors, RPM signal cable, USB cable (internal), acrylic
 ** case, slot sheet, 3.5" bay holders, CD with software
 ** 
 ** Specifications:
 ** dimensions: 88x88x16mm
 ** weight: 150g
 ** max. current: 7A
 ** max. power: 80W (PWM)/20W (analogue)
 ** power per channel: 40W (PWM)/20W (analogue)
 **
 ** 
 ** REVISION HISTORY
 ** ----------------
 ** 
 ** Date        Comment
 ** =================================================================
 ** 2006-09-26  File created
 ** 2006-09-28  Added basic functions to read stuff from the BigNG
 ** 		- bigNG_present
 ** 		- bigNG_getWatchdog
 ** 		- bigNG_getOutputMode
 ** 		- bigNG_getOvertemp
 ** 		- bigng_getChSensAssignment
 ** 		- bigNG_setOutputMode
 ** 2006-10-03  General improvements
 ** 		- bigNG_getChSensAssignment modified to support BigNG
 ** 		  specific sensors.
 ** 2006-10-06  General improvements
 ** 		- added function bigNG_init to be executed upon startup,
 ** 		  also init the sensor names that are specific to the
 ** 		  BigNG.
 ** 2007-03-07  Added functions (Wainer Vandelli)
 **             - bigNG_getaSensorTemp
 **             - bigNG_setAsScalingFact
 **             - bigNG_setAsAbsScalingFact
 **             - bigNG_setDsAbsScalingFact
 **             - bigNG_setChTargetTemp
 **             - bigNG_setChTargetMode
 ** 		
 *****************************************************************************/



/* Muliple inclusion safeguard */
#ifndef __BIG_NG_H
#define __BIG_NG_H

#include "tban.h"
#include "tban_hw_def.h"


/*****************************************************************************
 * BigNG output modes
 *****************************************************************************/
#define BIGNG_OUTPUT_MODE_PWM            0
#define BIGNG_OUTPUT_MODE_ANALOG         1


/*****************************************************************************
 * BigNG present (or not) constants
 *****************************************************************************/
#define BIGNG_PRESENT                    1
#define BIGNG_NOT_PRESENT                0


/*****************************************************************************
 * BigNG max target modes
 *****************************************************************************/
#define  BIGNG_MAX_TARGET_MODE           6

/*****************************************************************************
 * Exported BigNG functions
 *****************************************************************************/

/* Misc functions */
int bigNG_init(struct TBan* tban);
int bigNG_present(struct TBan* tban);
int bigNG_queryStatus(struct TBan* tban);
int bigNG_dataPresent(struct TBan *tban);

/* Getter functions */
int bigNG_getOutputMode(struct TBan* tban, unsigned char mode[]);
int bigNG_getOvertemp(struct TBan* tban, unsigned char* ot);
int bigng_getChSensAssignment(struct TBan* tban, unsigned char index, unsigned char* sens);
int bigNG_getaSensorTemp(struct TBan* tban, int index, unsigned char* temp, unsigned char* rawTemp, unsigned char* cal, unsigned char* abscal);
int bigNG_getValue(struct TBan* tban, int index, unsigned char* value);
int bigNG_getdSensorTemp(struct TBan* tban, int index, unsigned char* temp, unsigned char* rawTemp, unsigned char* cal, unsigned char * abscal);
int bigNG_getChInfo(struct TBan* tban, int index, unsigned int* rpmMax, unsigned char* pwm, unsigned char* resTemp, unsigned char* mode, unsigned char* target, unsigned char* targetmode);


/* Setter functions */
int bigNG_setOutputMode(struct TBan* tban, unsigned char modemask);
int bigNG_setAsScalingFact(struct TBan* tban, unsigned char index, unsigned char fact);
int bigNG_setAsAbsScalingFact(struct TBan* tban, unsigned char index, unsigned char fact);
int bigNG_setDsAbsScalingFact(struct TBan* tban, unsigned char index, unsigned char fact);
int bigNG_setChTargetTemp(struct TBan* tban, unsigned char index, unsigned char target);
int bigNG_setChTargetMode(struct TBan* tban, unsigned char index, unsigned char mode);

#endif /* __BIG_NG_H */



