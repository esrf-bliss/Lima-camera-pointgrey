pointgrey-objs = PointGreyCamera.o \
	PointGreyInterface.o \
	PointGreyDetInfoCtrlObj.o \
	PointGreySyncCtrlObj.o

SRCS = $(pointgrey-objs:.o=.cpp) 

CXXFLAGS += -I../include -I../../../hardware/include -I../../../common/include \
			-I/usr/include/flycapture \
			-fPIC -g -DUSE_GIGE

all:	PointGrey.o

PointGrey.o:	$(pointgrey-objs)
	$(LD) -o $@ -r $+

clean:
	rm -f *.o *.P

%.o : %.cpp
	$(COMPILE.cpp) -MD $(CXXFLAGS) -o $@ $<
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

-include $(SRCS:.cpp=.P)
