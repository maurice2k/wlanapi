COMPILER := g++
EXECUTABLE := example_list_aps
LIBS := dbus-glib-1
INCLUDES := /usr/include/dbus-1.0 /usr/lib/dbus-1.0/include /usr/include/glib-2.0 /usr/lib/glib-2.0/include
CFLAGS := -O3 -DDBUS

SOURCE := example_list_aps.cpp library/wlanapi.cpp library/wlanapi_exception.cpp

all : $(EXECUTABLE)

build : $(EXECUTABLE)

clean :
	rm -f $(EXECUTABLE)

$(EXECUTABLE) :
	$(COMPILER) $(CFLAGS) $(SOURCE) -o $(EXECUTABLE) $(addprefix -l,$(LIBS)) $(addprefix -I,$(INCLUDES))
	strip $(EXECUTABLE)
