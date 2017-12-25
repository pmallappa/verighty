CXX = g++
CFLAGS = -Wall
CFLAGS += -g -Iinclude

LD_DEFAULT_SCRIPT = link_default.lds
LDFLAGS = -Xlinker -T$(LD_DEFAULT_SCRIPT) -Tlink_tests.lds

HEADERS = $(wildcard *.h)
EXCLUDED_SRC_FILES = common_usb.c

SRC_FILES = $(filter-out $(EXCLUDED_SRC_FILES),$(wildcard *.c))
CXX_SRC_FILES += 
OBJECTS = $(SRC_FILES:.c=.o)
OBJECTS += $(CXX_SRC_FILES:.cpp=.o)

default: $(LD_DEFAULT_SCRIPT) main main-dyn

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) -c $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@

.PHONY: $(LIBUSB_DIR) libusb.so 

## Generating Default script from the toolchain itself
#  two parts to the following,
#  1. Query the toolchain to print out the linker script
#  2. Use 'sed' to extract relavant information.
#    format of the output is
#    <snip>  ; some compiler infomation
#    =======================================
#    SECTION: {
#		<snip>
#    } 
#    =======================================
#    <snip> ; some more information
$(LD_DEFAULT_SCRIPT):
	$(CC) -static /dev/null -Wl,--verbose 2>/dev/null | sed '1,/=====*/d;/=====*/,$$d' > $@
 
.ONESHELL:

$(LD_DEFAULT_SCRIPT):
	$(CC) -static /dev/null -Wl,--verbose 2>/dev/null | sed '1,/=====*/d;/=====*/,$$d' > $@

$(info $(OBJECTS))
.PHONY:$(LIB_AES_ECC)

test_crypto.o: EXTRA_CFLAGS:= -I../../components/aes -I../../components/crypto -I../../components/crypto/hashes

silk_test: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

main: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

main-dyn: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ -lusb $(LDFLAGS)

clean:
	rm -f $(OBJECTS) *~
	rm -f silk_test
	rm -f $(LD_DEFAULT_SCRIPT)


