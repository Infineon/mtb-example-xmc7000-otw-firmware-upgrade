################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Makefile for configurating the Cortex-M7 core project
#
################################################################################
# \copyright
# Copyright 2023, Cypress Semiconductor Corporation (an Infineon company)
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

# Include the common make files
include ../common.mk

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

# Include the application user config make files 
include ../user_config.mk

# To enable the DFU transport components.
ifeq ($(SELECTED_TRANSPORT), I2C)
    COMPONENTS=DFU_I2C DFU_USER
    $(info Selected I2C transport for DFU.)
else ifeq ($(SELECTED_TRANSPORT), UART)
    COMPONENTS=DFU_UART DFU_USER
    DEFINES+=DFU_UART_TX=P0_1
    DEFINES+=DFU_UART_RX=P0_0
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
	$(PYTHON_PATH) ../scripts/memorymap_rework.py run -p ../flashmap/xmc7200_platform.json -i ../flashmap/$(FLASH_MAP) -o ./source -n memorymap > ./memorymap.mk
	@echo -e "=============================================================\n"

-include ./memorymap.mk
endif

# Default start address of application (boot)
PLATFORM_USER_APP_START=$(APPLICATION_1_BOOT_SLOT_ADDRESS)

# For XMC7000 platform PRIMARY_IMG_START start is the same as USER_APP_START
# This parameter can be different in cases when code is resided in
# flash mapped to one address range, but executed using different bus
# for access with another address range. For example, execution of code
# from external memory in XIP mode.
PLATFORM_DEFAULT_PRIMARY_IMG_START=$(APPLICATION_1_BOOT_SLOT_ADDRESS)

USER_APP_START=$(APPLICATION_1_BOOT_SLOT_ADDRESS)
USER_APP_SIZE=$(APPLICATION_1_BOOT_SLOT_SIZE)
PRIMARY_IMG_START=$(APPLICATION_1_BOOT_SLOT_ADDRESS)
SECONDARY_IMG_START=$(APPLICATION_1_UPGRADE_SLOT_ADDRESS)

# possible values are 0 and 0xff
# internal Flash by default
ERASED_VALUE=0xFF

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
-include ./libs/mtb.mk

MCUBOOT_PATH=$(SEARCH_mcuboot)
MCUBOOT_CY_PATH=$(MCUBOOT_PATH)/boot/cypress
MCUBOOTAPP_PATH=$(MCUBOOT_CY_PATH)/MCUBootApp

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS+=XMC7xDUAL_CM0P_SLEEP

# Set this application to run at the CM7
CORE=CM7
CORE_NAME=CM7_$(APP_CORE_ID)

# Include confirmation flag setting (img_ok) implementation
ifeq ($(IMG_TYPE), UPGRADE)
ifeq ($(USE_OVERWRITE), 0)
SOURCES+=$(MCUBOOT_CY_PATH)/platforms/img_confirm/$(FAMILY)/set_img_ok.c 
endif
endif

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES+=\
    ../bootloader_cm0p/keys\
    $(MCUBOOT_CY_PATH)/platforms/memory/flash_map_backend\
    $(MCUBOOTAPP_PATH)/config\
    $(MCUBOOTAPP_PATH)/config/mcuboot_config\
    $(MCUBOOTAPP_PATH)/os\

# The following defines used for firmware upgrade
DEFINES+=MCUBOOT_IMAGE_NUMBER=$(MCUBOOT_IMAGE_NUMBER) \
         USER_APP_RAM_START=$(USER_APP_RAM_START) \
         USER_APP_RAM_SIZE=$(USER_APP_RAM_SIZE) \
         USER_APP_START=$(APPLICATION_1_BOOT_SLOT_ADDRESS) \
         USER_APP_SIZE=$(APPLICATION_1_BOOT_SLOT_SIZE) \
         PRIMARY_IMG_START=$(APPLICATION_1_BOOT_SLOT_ADDRESS) \
         MEMORY_ALIGN=$(PLATFORM_MEMORY_ALIGN) \
         PLATFORM_MAX_TRAILER_PAGE_SIZE=$(PLATFORM_MAX_TRAILER_PAGE_SIZE) \
         APP_$(APP_CORE) \
         BOOT_$(APP_CORE) \
         APP_CORE_ID=$(APP_CORE_ID) \
         $(PLATFORM_DEFINES_APP) \
         $(FAMILY) \
         $(PLATFORM) \
         $(DEVICE_NAME)

# Add additional defines to the build process (without a leading -D).
DEFINES+=CY_DFU_FLOW=CY_DFU_MCUBOOT_FLOW
DEFINES+=CY_DFU_PRODUCT=0x01020304

# Application-specific DEFINES
ifeq ($(IMG_TYPE), BOOT)
    DEFINES+=BOOT_IMAGE
    ENC_IMG=0
    APP_VERSION_MAJOR ?= 1
    APP_VERSION_MINOR ?= 0
    APP_VERSION_BUILD ?= 0
    PLATFORM_SIGN_ARGS:=--header-size 1024 --align 8
    HEADER_OFFSET=$(PRIMARY_IMG_START)
else
    DEFINES+=UPGRADE_IMAGE\
             SWAP_DISABLED=$(USE_OVERWRITE)
    APP_VERSION_MAJOR ?= 2
    APP_VERSION_MINOR ?= 0
    APP_VERSION_BUILD ?= 0
    PLATFORM_SIGN_ARGS:=--pad --header-size 1024 --align 8
    HEADER_OFFSET=$(SECONDARY_IMG_START)
endif

# add flag to imgtool if not using swap for upgrade
ifeq ($(USE_OVERWRITE), 1)
UPGRADE_TYPE:=--overwrite-only
DEFINES+=MCUBOOT_OVERWRITE_ONLY
endif

# Add version metadata to image
IMG_VER_ARG=$(APP_VERSION_MAJOR).$(APP_VERSION_MINOR).$(APP_VERSION_BUILD)
DEFINES+=IMG_VER_MSG='"$(IMG_VER_ARG)"' 
DEFINES+=IMG_TYPE_MSG='"$(IMG_TYPE)"'
DEFINES+=CORE_NAME_MSG='"$(CORE_NAME)"'
SIGN_ARGS=$(PLATFORM_SIGN_ARGS)

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

# Path to the linker script to use and other linker options.
LDFLAGS+=-Wl,--defsym,BOOT_HEADER_SIZE=$(BOOT_HEADER_SIZE)
LDFLAGS+=-Wl,--defsym,BOOTLOADER_APP_FLASH_SIZE=$(BOOTLOADER_SIZE)
LDFLAGS+=-Wl,--defsym,BOOTLOADER_APP_FLASH_ORIGIN=$(BOOTLOADER_ORIGIN)
LDFLAGS+=-Wl,--defsym,BOOTLOADER_APP_RAM_SIZE=$(BOOTLOADER_APP_RAM_SIZE)
LDFLAGS+=-Wl,--defsym,USER_APP_RAM_SIZE=$(USER_APP_RAM_SIZE)
LDFLAGS+=-Wl,--defsym,USER_APP_FLASH_START=$(USER_APP_START)
LDFLAGS+=-Wl,--defsym,USER_APP_FLASH_SIZE=$(USER_APP_SIZE)

# Additional / custom libraries to link in to the application.
LDLIBS=

# Path to the linker script to use (if empty, use the default linker script).
LINKER_SCRIPT=

# Custom pre-build commands to run.
PREBUILD=$(PREBUILD_VAR)

PREBUILD_VAR=+\
$(MAKE) generate_flashmap_cm7;\

# Custom post-build commands to run.
POSTBUILD=cysecuretools -t $(PLATFORM) sign-image $(SIGN_ARGS) -v $(IMG_VER_ARG) -d "($(IMG_ID), $(IMG_VER_ARG))" -S $(USER_APP_SIZE) -R $(ERASED_VALUE) $(UPGRADE_TYPE) --key-path ../keys/$(SIGN_KEY_FILE).pem --image build/APP_KIT_XMC72_EVK/$(CONFIG)/$(APPNAME).hex --output build/APP_KIT_XMC72_EVK/$(CONFIG)/$(APPNAME).hex --hex-addr=$(HEADER_OFFSET)

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
