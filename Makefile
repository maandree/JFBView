SHEAR_CORRECTION = 0
HEIGHT_CORRECTION = 0

CXXFLAGS := -Wall -O3 -std=c++11
CPPFLAGS := -DSHEAR_CORRECTION=$(SHEAR_CORRECTION) -DHEIGHT_CORRECTION=$(HEIGHT_CORRECTION)
LIBS := -lpthread -lncurses -lfitz -lfreetype -ljbig2dec -ljpeg -lz -lopenjpeg
JFBVIEW_LIBS := $(LIBS) -lImlib2

SRCS := $(wildcard *.cpp)


all: jfbview

.PHONY: clean
clean:
	-rm -f *.o *.d jfbview jfbpdf

%.d: %.cpp
	@$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $(CPPFLAGS) $< \
	  | sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
	  [ -s $@ ] || rm -f $@'
-include $(SRCS:.cpp=.d)

jfbview: $(SRCS:.cpp=.o)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS) $(JFBVIEW_LIBS)

jfbpdf: $(SRCS)
	$(CXX) -DJFBVIEW_NO_IMLIB2 -DJFBVIEW_PROGRAM_NAME=\"JFBPDF\" \
	  -DJFBVIEW_BINARY_NAME=\"$@\" $(CXXFLAGS) -o $@ $^ $(LDFLAGS) \
	  $(LDLIBS) $(LIBS)
