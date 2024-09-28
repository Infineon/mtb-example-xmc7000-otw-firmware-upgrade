################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Makefile for configurating the Cortex-M7 core project
#
################################################################################
# \copyright
# Copyright 2023-2024, Cypress Semiconductor Corporation (an Infineon company)
# SPDX-License-Identifier: Apache-2.0
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

################################################################################
# Basic Configuration
################################################################################

# Include the user configuration make file
include ../user_config.mk

# Name of application (used to derive name of final linked file).
# 
# If APPNAME is edited, ensure to update or regenerate launch 
# configurations for your IDE.
APPNAME=dfu_cm7

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

################################################################################
# DFU App Specific Configuration
###############################################################################

# To enable the DFU transport components.
ifeq ($(SELECTED_TRANSPORT), I2C)
    COMPONENTS=DFU_I2C DFU_USER
    $(info Selected I2C transport for DFU.)
else ifeq ($(SELECTED_TRANSPORT), UART)
    COMPONENTS=DFU_UART DFU_USER
    DEFINES+=DFU_UART_TX=P20_4
    DEFINES+=DFU_UART_RX=P20_3
    $(info Selected UART transport for DFU.)
else ifeq ($(SELECTED_TRANSPORT), SPI)
    COMPONENTS=DFU_SPI DFU_USER
    $(info Selected SPI transport for DFU.)  
else
    $(error selected DFU transport is not supported at the moment !.)
endif

################################################################################
# Memory (flash) map  Specific Configuration For Firmware Upgrade
###############################################################################

.PHONY: generate_flashmap_cm7

# Python command to generate flashmap header file from flashmap JSON file
ifneq ($(FLASH_MAP), )
generate_flashmap_cm7:
	@echo -e "\n============================================================="
	@echo -e "= Generating memorymap.h, memorymap.c and memorymap.mk ="
	@echo -e "============================================================="
	$(CY_PYTHON_PATH) ../scripts/memorymap_xmc7000.py run -p ../flashmap/$(PLATFORM_CONFIG) -i ../flashmap/$(FLASH_MAP) -o ./source -n memorymap -d $(IMG_ID) > ./memorymap.mk
	@echo -e "=============================================================\n"

-include ./memorymap.mk
DEFINES+=CY_FLASH_MAP_JSON
endif

# Include the common library make file
include ../common_libs.mk

# use USE_OVERWRITE = 1 for overwrite only mode
# use USE_OVERWRITE = 0 for swap upgrade mode
ifeq ($(USE_OVERWRITE), )
USE_OVERWRITE?=$(PLATFORM_DEFAULT_USE_OVERWRITE)
endif

# 1. Define the image type.
# 2. Ignore the build directory(if any) of other build mode to avoid linker error.
# 3. Disabling the encrpyt image support while signing the boot image
# 4. Add flag, if not using swap for upgrade
ifeq ($(IMG_TYPE), BOOT)
DEFINES+=BOOT_IMAGE
CY_IGNORE+=build/UPGRADE
ENC_IMG=0
else ifeq ($(IMG_TYPE), UPGRADE)
DEFINES+=UPGRADE_IMAGE
CY_IGNORE+=build/BOOT
DEFINES+=SWAP_DISABLED=$(USE_OVERWRITE)
endif

# Set the version of the app using the following three variables.
# This version information is passed to the Python module "imgtool" or "cysecuretools" while
# signing the image in the post build step. Default values are set as follows.
ifeq ($(IMG_TYPE), BOOT)
APP_VERSION_MAJOR?=1
APP_VERSION_MINOR?=0
APP_VERSION_BUILD?=0
else ifeq ($(IMG_TYPE), UPGRADE)
APP_VERSION_MAJOR?=2
APP_VERSION_MINOR?=0
APP_VERSION_BUILD?=0
endif

# Add flag to imgtool if not using swap for upgrade
ifeq ($(USE_OVERWRITE), 1)
UPGRADE_TYPE:=--overwrite-only
DEFINES+=MCUBOOT_OVERWRITE_ONLY
endif

# Add image type, version and core name metadata to image
IMG_VER_ARG=$(APP_VERSION_MAJOR).$(APP_VERSION_MINOR).$(APP_VERSION_BUILD)
DEFINES+=IMG_VER_MSG='"$(IMG_VER_ARG)"'
DEFINES+=IMG_TYPE_MSG='"$(IMG_TYPE)"'
DEFINES+=CORE_NAME_MSG='"$(CORE_NAME)"'

# Flash erase value
# Possible values are 0 and 0xff
# External flash is not supported for XMC7000 family at this moment
ERASED_VALUE=0xff

################################################################################
# Advanced Configuration
################################################################################

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
COMPONENTS+=

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=XMC7xDUAL_CM0P_SLEEP XMC7x_CM0P_SLEEP

# Set this application to run at the CM7
CORE=$(APP_CORE)
CORE_NAME=$(APP_CORE)_$(APP_CORE_ID)

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
# Include confirmation flag setting (img_ok) implementation
ifeq ($(IMG_TYPE), UPGRADE)
ifeq ($(USE_OVERWRITE), 0)
SOURCES+=$(MCUBOOT_CY_PATH)/platforms/img_confirm/$(FAMILY)/set_img_ok.c
INCLUDES+=$(MCUBOOT_CY_PATH)/platforms/img_confirm
endif
endif

# The following defines used for firmware upgrade
DEFINES+=MCUBOOT_IMAGE_NUMBER=$(MCUBOOT_IMAGE_NUMBER)\
         USER_APP_START=$(PRIMARY_IMG_START)\
         USER_APP_SIZE=$(SLOT_SIZE)\
         PRIMARY_IMG_START=$(PRIMARY_IMG_START)\
         MEMORY_ALIGN=$(PLATFORM_MEMORY_ALIGN)\
         PLATFORM_MAX_TRAILER_PAGE_SIZE=$(PLATFORM_MAX_TRAILER_PAGE_SIZE)\
         APP_$(APP_CORE)\
         BOOT_$(APP_CORE)\
         APP_CORE_ID=$(APP_CORE_ID)\
         $(PLATFORM_DEFINES_APP)\
         $(FAMILY)\
         $(PLATFORM)

# Add additional defines to the build process (without a leading -D).
DEFINES+=CY_DFU_FLOW=CY_DFU_MCUBOOT_FLOW
DEFINES+=CY_DFU_PRODUCT=0x01020304

DEFINES+=APP_VERSION_MAJOR=$(APP_VERSION_MAJOR)\
         APP_VERSION_MINOR=$(APP_VERSION_MINOR)\
         APP_VERSION_BUILD=$(APP_VERSION_BUILD)

# Add additional defines to the build process (without a leading -D).
DEFINES+=CY_RETARGET_IO_CONVERT_LF_TO_CRLF

# Disabled D-cache for XMC7000 based BSPs
DEFINES+=CY_DISABLE_XMC7000_DATA_CACHE

# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT=

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CFLAGS=

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS=

# Additional / custom linker flags.
LDFLAGS+=-Wl,--defsym,MCUBOOT_HEADER_SIZE=$(MCUBOOT_HEADER_SIZE)
LDFLAGS+=-Wl,--defsym,CM0P_RAM_SIZE=$(BOOTLOADER_APP_RAM_SIZE)
LDFLAGS+=-Wl,--defsym,USER_APP_SIZE=$(SLOT_SIZE)
LDFLAGS+=-Wl,--defsym,USER_APP_START=$(PRIMARY_IMG_START)
LDFLAGS+=-Wl,--defsym,CM0P_FLASH_SIZE=$(shell expr $$(( $(PRIMARY_IMG_START) - 0x10000000 )) )
LDFLAGS+=-Wl,--defsym,USER_APP_RAM_SIZE=$(USER_APP_RAM_SIZE)

# Additional / custom libraries to link in to the application.
LDLIBS=

# Path to the custom linker script to use and other linker options.
LINKER_SCRIPT=

# Set build directory for BOOT and UPGRADE images
CY_BUILD_LOCATION=./build/$(IMG_TYPE)
BINARY_OUT_PATH=$(CY_BUILD_LOCATION)/$(TARGET)/$(CONFIG)/$(APPNAME)

XMC7000_PLATFORM_SIGN_ARGS=$(PLATFORM) sign-image --header-size 1024 --align 8 -v $(IMG_VER_ARG) -d "($(IMG_ID),$(IMG_VER_ARG))" -S $(SLOT_SIZE) -R $(ERASED_VALUE) $(UPGRADE_TYPE)\
               --key-path $(SIGN_KEY_FILE_PATH)/$(SIGN_KEY_FILE).pem

# Starting address of the CM4 app or the offset at which the header of an image
# will begin. Image = Header + App + TLV + Trailer. See MCUboot documenation for
# details.
# New relocated address = ORIGIN + HEADER_OFFSET
# Padding the image for UPGRADE image.
ifeq ($(IMG_TYPE), BOOT)
HEX_START_ADDR=$(PRIMARY_IMG_START)
else ifeq ($(IMG_TYPE), UPGRADE)
XMC7000_PLATFORM_SIGN_ARGS+= --pad
HEX_START_ADDR=$(SECONDARY_IMG_START)
endif

# Custom pre-build commands to run.
PREBUILD=$(PREBUILD_VAR)

# Custom pre-build commands to run.
PREBUILD_VAR=+\
rm -f ./source/memorymap.c;\
rm -f ./source/memorymap.h;\
rm -f ./memorymap.mk;\
$(MAKE) generate_flashmap_cm7;

POSTBUILD_VAR=+\
cp -f $(BINARY_OUT_PATH).hex $(BINARY_OUT_PATH)_unsigned.hex;\
rm -f $(BINARY_OUT_PATH).hex;\
$(CY_PYTHON_PATH) -m cysecuretools -t $(XMC7000_PLATFORM_SIGN_ARGS) --image $(BINARY_OUT_PATH)_unsigned.hex --output $(BINARY_OUT_PATH).hex --hex-addr=$(HEX_START_ADDR);

# Custom post-build commands to run.
POSTBUILD=$(POSTBUILD_VAR)

################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the shared repo location.
#
# All .mtb files have the format, <URI>#<COMMIT>#<LOCATION>. If the <LOCATION> field 
# begins with $$ASSET_REPO$$, then the repo is deposited in the path specified by 
# the CY_GETLIBS_SHARED_PATH variable. The default location is one directory level 
# above the current app directory.
# This is used with CY_GETLIBS_SHARED_NAME variable, which specifies the directory name.
CY_GETLIBS_SHARED_PATH=../..

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

# Absolute path to the compiler's "bin" directory. The variable name depends on the 
# toolchain used for the build. Refer to the ModusToolbox user guide to get the correct
# variable name for the toolchain used in your build.
# 
# The default depends on the selected TOOLCHAIN (GCC_ARM uses the ModusToolbox
# software provided compiler by default).
CY_COMPILER_GCC_ARM_DIR=

include $(CY_TOOLS_DIR)/make/start.mk
