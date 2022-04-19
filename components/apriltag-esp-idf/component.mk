COMPONENT_ADD_INCLUDEDIRS := . common
COMPONENT_SRCDIRS := . common

CFLAGS += -std=gnu99 -fPIC -Wall -Wno-unused-parameter -Wno-unused-function
CFLAGS += -I. -O3 -fno-strict-overflow
CFLAGS += -Wno-char-subscripts -Wno-sizeof-pointer-memaccess -Wno-maybe-uninitialized -Wno-implicit-function-declaration