# Specify the name of the output binary.
BIN=piano.bin

ifdef BREAK
ZOS_CFLAGS += -DBREAK
endif

ifndef ZGDK_PATH
    $(error "Failure: ZGDK_PATH variable not found. It must point to ZGDK path.")
endif

ENABLE_SOUND=1
TILED_OUTPUT=assets/piano.ztm

include $(ZGDK_PATH)/base_sdcc.mk

## Add your own rules here