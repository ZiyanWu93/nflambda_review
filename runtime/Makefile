# binary name
APP = nf_p

current_dir = $(shell pwd)
SRCS-y := $(shell find ${current_dir} -name "*.c")
SRCS-header := $(shell find ${current_dir} -name "*.h")

all: build/$(APP) build/$(APP)-debug

run: build/$(APP)
#	sudo build/nf_p 8093
	sudo build/nf_p

db: debug
#	sudo build/nf_p-debug 8093
	sudo build/nf_p-debug

debug: build/$(APP)-debug

only_build: build/$(APP)

print:
	echo $(current_dir)

PKGCONF ?= pkg-config

# PC_FILE := $(shell $(PKGCONF) --path libdpdk 2>/dev/null)
CFLAGS += -O3 $(shell $(PKGCONF) --cflags libdpdk) -D_GNU_SOURCE -Wfatal-errors
DEBUG_CFLAGS += -O0 $(shell $(PKGCONF) --cflags libdpdk) -D_GNU_SOURCE -Wfatal-errors
# Add flag to allow experimental API as l2fwd uses rte_ethdev_set_ptype API
CFLAGS += -DALLOW_EXPERIMENTAL_API
CFLAGS += -D_GNU_SOURCE
LDFLAGS_SHARED = $(shell $(PKGCONF) --libs libdpdk)

# LDFLAGS_STATIC = -Wl,-Bstatic $(shell $(PKGCONF) --static --libs libdpdk)
LDFLAGS += -lpqos
LDFLAGS += -lpci
LDFLAGS += -lpapi
LDFLAGS += -lyaml
LDFLAGS += -ldl

# library written by myself
LDFLAGS += -lset
LDFLAGS += -lzgraph
LDFLAGS += -lzqueue
LDFLAGS += -lnfvi

DEBUGFLOAGS = -g -D DEBUG $(DEBUG_CFLAGS)
INC := -I/${PAPI_DIR}/include

build/$(APP): $(SRCS-y) $(SRCS-header) Makefile | build
	$(CC) $(CFLAGS) $(SRCS-y) -o $@ $(LDFLAGS) $(LDFLAGS_SHARED)

build/$(APP)-debug:  $(SRCS-y) $(SRCS-header) Makefile | build
	$(CC) $(DEBUGFLOAGS) $(SRCS-y) -o $@ $(LDFLAGS) $(LDFLAGS_SHARED)

build:
	@mkdir -p $@

.PHONY: clean
clean:
	rm -rf build