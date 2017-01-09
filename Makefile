# Copyright (c) 2016 Mirants Lu. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file. 

#--------------------------------------------------------------------
# Uncomment exactly one of the lines labelled (1), (2), and (3) below 
# to switch between compilation modes.
#
# (1) Production use (optimized mode)
      OPT ?= -O2 -DNDEBUG
# (2) Debug mode, w/full line-level debugging symbols
#     OPT ?= -g2
# (3) Profiling mode: opt, but w/debugging symbols
#     OPT ?= -O2 -g2 -DNDEBUG
#--------------------------------------------------------------------

# detect what platform we're building on
$(shell CC="$(CC)" CXX="$(CXX)" TARGET_OS="$(TARGET_OS)" \
  ./build_detect_platform build_config.mk .)

# this  file is generated by the previous line to set build flags and sources
include build_config.mk

TESTS = \

UTILS = \
	paxos/paxos_test \

# Put the object files in a subdirectory, but the application at the top of 
# the object dir
PROGNAMES := $(notdir $(TESTS) $(UTILS))

CFLAGS += -I. -I./include $(PLATFORM_CCFLAGS) $(OPT)
CXXFLAGS += -I. -I./include $(PLATFORM_CXXFLAGS) $(OPT)

LDFLAGS += $(PLATFORM_LDFLAGS)
LIBS += $(PLATFORM_LIBS)

STATIC_OUTDIR = out-static

STATIC_PROGRAMS := $(addprefix $(STATIC_OUTDIR)/, $(PROGNAMES))

STATIC_LIBOBJECTS := $(addprefix $(STATIC_OUTDIR)/, $(SOURCES:.cc=.o)) 

STATIC_TESTOBJS := $(addprefix $(STATIC_OUTDIR)/, $(addsuffix .o, $(TESTS)))
STATIC_UTILOBJS := $(addprefix $(STATIC_OUTDIR)/, $(addsuffix .o, $(UTILS)))

STATIC_ALLOBJS := $(STATIC_LIBOBJECTS) $(STATIC_TESTOBJS) $(STATIC_UTILOBJS)

default: all

all: $(STATIC_OUTDIR)/libskywalker.a $(STATIC_PROGRAMS)

check: $(STATIC_PROGRAMS)
	for t in $(notdir $(TESTS)); do echo "***** Running $$t"; $(STATIC_OUTDIR)/$$t || exit 1; done

clean:
	-rm -rf out-static
	-rm -f build_config.mk
	-rm -rf ./paxos/paxos.pb.*

$(STATIC_OUTDIR):
	mkdir $@

$(STATIC_OUTDIR)/machine: | $(STATIC_OUTDIR)
	mkdir $@

$(STATIC_OUTDIR)/network: | $(STATIC_OUTDIR)
	mkdir $@

$(STATIC_OUTDIR)/paxos: | $(STATIC_OUTDIR)
	mkdir $@

$(STATIC_OUTDIR)/storage: | $(STATIC_OUTDIR)
	mkdir $@

$(STATIC_OUTDIR)/util: | $(STATIC_OUTDIR)
	mkdir $@

.PHONY: STATIC_OBJDIRS
STATIC_OBJDIRS: \
  $(STATIC_OUTDIR)/machine \
  $(STATIC_OUTDIR)/network \
  $(STATIC_OUTDIR)/storage \
  $(STATIC_OUTDIR)/paxos \
  $(STATIC_OUTDIR)/util

$(STATIC_ALLOBJS): | STATIC_OBJDIRS

$(STATIC_OUTDIR)/libskywalker.a:$(STATIC_LIBOBJECTS)
	rm -f $@
	ar -rs $@ $(STATIC_LIBOBJECTS)

$(STATIC_OUTDIR)/paxos_test:paxos/paxos_test.cc $(STATIC_LIBOBJECTS)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) paxos/paxos_test.cc $(STATIC_LIBOBJECTS) -o $@ $(LIBS)

$(STATIC_OUTDIR)/%.o: %.cc 
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(STATIC_OUTDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
