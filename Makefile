TARGETS = Line

PROF_TARGETS := $(TARGETS:%=%.prof)

TEST_TARGETS = Test

# results dumped by cilkview objects
VIEWS = linedemo

# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = gtest

# Normal target.
HDRS = \
       CollisionWorld.h \
       GraphicStuff.h \
       IntersectionDetection.h \
       Line.h \
       LineDemo.h \
       Vec.h
SRCS = $(HDRS:.h=.cpp)

# Files used when building for profiling.
PROF_HDRS = $(filter-out GraphicStuff.h,$(HDRS))
PROF_SRCS = $(PROF_HDRS:.h=.cpp)

# Unit test target files.
TEST_HDRS = $(filter-out Line.h,$(PROF_HDRS)) \
	    $(GTEST_DIR)/include/gtest/*.h \
	    $(GTEST_DIR)/include/gtest/internal/*.h
TEST_SRCS = $(filter-out Line.cpp,$(PROF_SRCS)) Test.cpp

CC      := g++
CILK    := icc
# icc does not support newest gcc compiled  tr1 tuples
#http://code.google.com/p/googletest/issues/detail?id=100
CFLAGS  := -Wall -g -I/usr/X11R6/include/ -I/afs/csail.mit.edu/proj/courses/6.172/cilkutil/include -DGTEST_HAS_TR1_TUPLE=0
LDFLAGS := -lXext -lX11 -lm 
ARFLAGS := r

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

OUTPUTS := $(VIEWS:%=%.csv) $(VIEWS:%=%.plt) $(TARGETS:%=%.cv.out) cilkview.out

all: $(TARGETS) $(TEST_TARGETS)

# We only need TEST_TARGETS here, but we want to catch compile errors in other
# files and build modes, so we depend on more stuff.
test: all $(PROF_TARGETS)
	./Test


$(TARGETS) : % : $(SRCS) $(HDRS) .buildmode
	$(CILK) -o $@ $(CFLAGS) $(SRCS) $(LDFLAGS)

$(PROF_TARGETS) %.prof : $(PROF_SRCS) $(VTHDRS) .buildmode
	$(CILK) -o $@ $(CFLAGS) -DPROFILE_BUILD $(PROF_SRCS) -lm -p 

# We use the PROFILE_BUILD define to cut the graphics and cilk dependencies.
# Perhaps that define should be renamed.
$(TEST_TARGETS) : % : $(TEST_SRCS) $(TEST_HDRS) gtest.a .buildmode
	$(CILK) -o $@ $(CFLAGS) -DPROFILE_BUILD -I$(GTEST_DIR)/include \
		$(TEST_SRCS) gtest.a -lpthread -lm 

clean:
	$(RM) $(TARGETS) $(PROF_TARGETS) $(TEST_TARGETS) $(OUTPUTS) \
		*.stdout *.stderr gtest.a *.o

## Google Test's targets.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS)
	$(CXX) -I$(GTEST_DIR)/include -I$(GTEST_DIR) $(CFLAGS) -c \
		$(GTEST_DIR)/src/gtest-all.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^
