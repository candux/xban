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
 ** Filename:    tban_hw_def.h
 ** Initial author:  marcus.jagemar@gmail.com
 **
 **
 ** DESCRIPTION
 ** -----------
 ** This file will simply hold HW definitions that should be globally
 ** visible throughout the whole XBan project, such as number of
 ** channels and sensors for all different types of devices.
 ** 
 ** 
 ** REVISION HISTORY
 ** ----------------
 ** 
 ** Date   Rev    Comment
 ** =================================================================
 ** 2006-10-06 First created
 ** 
 ** 
 *****************************************************************************/

#ifndef __TBAN_HW_DEF_H
#define __TBAN_HW_DEF_H


/*****************************************************************************
 * Classic TBan (SL/XL) hardware parameters
 *****************************************************************************/
#define TBAN_NUMBER_ANALOG_SENSORS   6
#define TBAN_NUMBER_DIGITAL_SENSORS  8
#define TBAN_NUMBER_CHANNELS         4


/*****************************************************************************
 * MiniNG hardware parameters
 *****************************************************************************/
#define MINI_NG_NUMBER_ANALOG_SENSORS         2
#define MINI_NG_NUMBER_CHANNELS               2


/*****************************************************************************
 * BigNG hardware parameters
 * 
 * The number of standard analog sensors supported by the BigNG is the
 * same as original TBan but with a number of additional ones using a
 * separate interface. This means that the total number of analog
 * sensors are 10 and digital sensors are still 8.
 *****************************************************************************/
#define BIGNG_NUMBER_ADDITIONAL_ANALOG_SENSORS  4

/* Other sensors */
#define BIGNG_NUMBER_FLOWMETERS                 2




#endif /* __TBAN_HW_DEF_H */

