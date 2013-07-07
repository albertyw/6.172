TARGETS = nbodies_loops nbodies_symmetric nbodies_nolocks
TARGETS32 := $(TARGETS:%=%.32)
TARGETS64 := $(TARGETS:%=%.64)
ALLTARGETS := $(TARGETS64)

# results dumped by cilkview objects
VIEWS = nbodies

COMMONHDR := common.h
COMMONSRC := common.cpp

CC      := icc
INCLUDES = -I/afs/csail.mit.edu/proj/courses/6.172/cilkutil/include
CFLAGS  := -g -Werror -I include $(INCLUDES)
LDFLAGS := -lpng -lz -ltbb 

OLDMODE := $(shell cat .buildmode 2> /dev/null)
ifeq ($(DEBUG),1)
CFLAGS := -DDEBUG -O0 $(CFLAGS)
ifneq ($(OLDMODE),debug)
$(shell echo debug > .buildmode)
endif
else
CFLAGS := -O3 $(CFLAGS)
ifneq ($(OLDMODE),nodebug)
$(shell echo nodebug > .buildmode)
endif
endif

CFLAGS32 := $(CFLAGS) -m32
CFLAGS64 := $(CFLAGS) -m64
LDFLAGS32 := $(LDFLAGS) -L lib/i386
LDFLAGS64 := $(LDFLAGS) -L lib/x86_64

OUTPUTS := $(VIEWS:%=%.csv) $(VIEWS:%=%.plt) $(ALLTARGETS:%=%.cv.out) cilkview.out

all: $(ALLTARGETS)

.buildmode:
	touch .buildmode

%.32 : %.cpp $(COMMONSRC) $(COMMONHDR) .buildmode
	$(CC) -o $@ $(CFLAGS32) $< $(COMMONSRC) $(LDFLAGS32)

%.64 : %.cpp $(COMMONSRC) $(COMMONHDR) .buildmode
	$(CC) -o $@ $(CFLAGS64) $< $(COMMONSRC) $(LDFLAGS64)

# publish movie to CSAIL webpage
publish:
	mkdir -p $(HOME)/public_html/nbodies
	cp nbodies.html $(HOME)/public_html/nbodies/index.html
	cp *.png $(HOME)/public_html/nbodies/

clean:
	rm -f $(ALLTARGETS) $(OUTPUTS) *.stdout *.stderr *.png
