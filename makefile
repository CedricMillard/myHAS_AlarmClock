CC=g++
CFLAGS = -I.
LIBS = -lncurses -lcurl -lb64 -lwiringPi -pthread /home/admin/Dev/C/myHAS/build/myHAS_Lib
DEPS = myHAS_DisplaySegments.h myHAS_DisplayScreen.h myHAS_Displays.h myHAS_SoundDriver.h myHAS_Alarm.h myHAS_Environment.h myHAS_Inputs.h myHAS_InputKeyboard.h myHAS_InputButtons.h
ODIR=../build/obj
BDIR=../build

_OBJ = myHAS_AlarmClock.o myHAS_DisplaySegments.o myHAS_DisplayScreen.o myHAS_Displays.o myHAS_SoundDriver.o myHAS_Alarm.o myHAS_Environment.o myHAS_Inputs.o myHAS_InputKeyboard.o myHAS_InputButtons.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

All: $(BDIR)/myHAS_AlarmClock

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BDIR)/myHAS_AlarmClock: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
