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
 ** Filename:        common.c
 ** Initial author:  marcus.jagemar@gmail.com
 **
 ** 
 ** DESCRIPTION
 ** -----------
 ** Common functions used by the libtban project.
 **
 ** 
 *****************************************************************************/

#ifndef __COMMON_H
#define __COMMON_H

/*****************************************************************************
 * Check if the result from the function called is anything else than
 * TBAN_OK. If so return from the function with the obtained return
 * value.
 *****************************************************************************/
#define CHECK_RESULT(STRING) { \
                               int result=STRING; \
                               if(result != TBAN_OK) { \
                                 return result; \
                               } \
                             }

/**********************************************************************
 * Name        : local_nanosleep
 * Description : Sleep for some time
 * Arguments   : sec  = Seconds
 *               nano = Nanoseconds
 * Returning   : none
 **********************************************************************/
static void local_nanosleep(long sec, long nano) {
  struct timespec delay;
  /* Set delay settings for nanosleep */
  delay.tv_sec  = sec;
  delay.tv_nsec = nano;
  (void) nanosleep(&delay, NULL);
}


/*****************************************************************************
 * Declaration of functions needed by all subcomponents within the XBan
 * project
 *****************************************************************************/
void tban_updateProgress(struct TBan* tban, int cur, int max);
int tban_sendCommand(struct TBan* tban, unsigned char* sndBuf, int cmdLen);
int tban_readData(struct TBan* tban, unsigned char* buf, int expected);



#endif /* COMMON_H */

