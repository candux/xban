################################################################################
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#  
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#  
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#  
#   
#   FILE INFORMATION
#   ----------------
#   Filename:        Makefile
#   Initial author:  marcus.jagemar@gmail.com
#  
#   
#   DESCRIPTION
#   -----------
#   This is one part of the implementation of the TBan library. Please
#   see the file tban.h for more information.
################################################################################


################################################################################
# Build commands
################################################################################
SHELL	= /bin/sh

################################################################################
# Compilation and linker flags
# If we try to compile this at something else than Linux we may need to
# remove some functions.
################################################################################
CFLAGS = -Wall -pedantic -ggdb -D LINUX -I../libtban 
LFLAGS = -D LINUX -I../libtban -L../libtban -ltban

################################################################################
# What should we remove when cleaning out the tbancontrol program?
CLEAN = *.o tbancontrol
################################################################################


################################################################################
# Rules
################################################################################
tbancontrol: tbancontrol.c ll.o ../libtban/libtban.a
	$(CC) -o tbancontrol ll.o tbancontrol.c $(LFLAGS)

ll.o: ll.c ll.h
	$(CC) $(CFLAGS) -c ll.c

clean:
	/bin/rm -f $(CLEAN)

