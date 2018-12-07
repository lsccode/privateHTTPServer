ROOT_DIR := $(shell pwd)
SOURCE_DIR := \
config/cgi-bin \
thttpd-2.29 

COMPILE_DIR := $(addprefix $(ROOT_DIR)/,$(SOURCE_DIR))

.PHONY:$(COMPILE_DIR)
all:$(COMPILE_DIR) 

$(COMPILE_DIR):
	$(MAKE) --directory=$@ $(TARGET)

install:
	$(MAKE) TARGET=install
	
release:
	@echo "release"
clean:
	$(MAKE) TARGET=clean

test:
	@echo $(PWD)
	@echo $(COMPILE_DIR)