############################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2013
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################
include ../../../config.inc

SRCS = PgTest.cpp


LDFLAGS = -pthread -L../../../build  -L../../../third-party/Processlib/build -L../sdk/usr/lib
LDLIBS = -llimacore  -lprocesslib -lflycapture

CXXFLAGS = -I../include \
           -I../../../hardware/include \
           -I../../../control/include \
           -I../../../common/include \
           -I../sdk/usr/include/flycapture \
           -I../../../control/software_operation/include \
	   -I../../../third-party/Processlib/core/include \
	   -I../../../third-party/Processlib/tasks/include \
	   -I../../../third-party/Sps/Include \
            --std=c++0x -fPIC -g -DUSE_GIGE -Wall -pthread -g

test-progs = pgtest

all: 	$(test-progs)

pgtest:		PgTest.o ../src/PointGrey.o
	$(CXX) $(LDFLAGS) -o $@ $+ $(LDLIBS)

clean:
	rm -f *.o *.P pgtest

%.o : %.cpp
	$(COMPILE.cpp) -MD $(CXXFLAGS) -o $@ $<
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

-include $(SRCS:.cpp=.P)
