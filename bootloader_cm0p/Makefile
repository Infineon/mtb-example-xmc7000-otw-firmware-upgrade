################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Makefile for configurating the Cortex-M0P core project
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

# Include the user configuration make file
include ../user_config.mk

################################################################################
# Basic Configuration
################################################################################

# Name of application (used to derive name of final linked file).
# 
# If APPNAME is edited, ensure to update or regenerate launch 
# configurations for your IDE.
APPNAME=bootloader_cm0p

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

################################################################################
# MCUboot Specific Configuration
###############################################################################

.PHONY: gen_cy_si_key_source_file gen_key_rsa2k gen_key_ecc256 generate_flashmap_cm0p

# Command to generate `cy_si_key.c` file
gen_cy_si_key_source_file:
	@echo -e "\nGenerate cy_si_key.c file"
	$(CY_PYTHON_PATH) -m cysecuretools convert-key -k ../keys/$(SECURE_MODE_KEY_FILE).pub -o $(MCUBOOT_CY_PATH)/platforms/utils/$(FAMILY)/cy_si_key.c --fmt secure_boot --endian little
	
# Command to generate permanent and public keys for bootloader to boot securely
gen_key_rsa2k:
	@echo -e "\nGenerate RSA2048 keys"
	$(CY_PYTHON_PATH) -m cysecuretools -t $(PLATFORM) create-key --key-type $(SECURE_MODE_KEY_TYPE) -o ../keys/$(SECURE_MODE_KEY_FILE).pem ../keys/$(SECURE_MODE_KEY_FILE).pub --format PEM

# Python command to generate permanent and public keys for application to boot securely
gen_key_ecc256:
	@echo -e "\nGenerate ECC256 keys"
	$(CY_PYTHON_PATH) $(IMGTOOL_PATH)/imgtool.py keygen -k ../keys/$(SIGN_KEY_FILE).pem -t ecdsa-p256
	$(CY_PYTHON_PATH) $(IMGTOOL_PATH)/imgtool.py getpub -k ../keys/$(SIGN_KEY_FILE).pem > ../keys/$(SIGN_KEY_FILE).pub

# Python command to generate memorymap sources file from memorymap JSON file
ifneq ($(FLASH_MAP), )
generate_flashmap_cm0p:
	@echo -e "\n============================================================="
	@echo -e "= Generating memorymap.h, memorymap.c and memorymap.mk ="
	@echo -e "============================================================="
	$(CY_PYTHON_PATH) ../scripts/memorymap_xmc7000.py run -p ../flashmap/$(PLATFORM_CONFIG) -i ../flashmap/$(FLASH_MAP) -o ./source -n memorymap > ./memorymap.mk
	@echo -e "=============================================================\n"

-include ./memorymap.mk
DEFINES+=CY_FLASH_MAP_JSON
endif

# Include the common library make file
include ../common_libs.mk

# Can be set at `MCUBOOT_LOG_LEVEL_DEBUG` to enable the verbose output of MCUBootApp.
MCUBOOT_LOG_LEVEL?=MCUBOOT_LOG_LEVEL_DEBUG
USE_SHARED_SLOT?=0
FIH_PROFILE_LEVEL_LIST:=OFF LOW MEDIUM HIGH
FIH_PROFILE_LEVEL?=MEDIUM

# Check FIH profile param
ifneq ($(filter $(FIH_PROFILE_LEVEL), $(FIH_PROFILE_LEVEL_LIST)),)
ifneq ($(FIH_PROFILE_LEVEL), OFF)
DEFINES+=MCUBOOT_FIH_PROFILE_ON
DEFINES+=MCUBOOT_FIH_PROFILE_$(FIH_PROFILE_LEVEL)
endif
else
$(error Wrong FIH_PROFILE_LEVEL param)
endif

# use USE_OVERWRITE = 1 for overwrite only mode
# use USE_OVERWRITE = 0 for swap upgrade mode
ifeq ($(USE_OVERWRITE), )
USE_OVERWRITE?=$(PLATFORM_DEFAULT_USE_OVERWRITE)
endif

# 1. Add defines to enable image overwrite operation
# 2. To enable downgrade prevention
# 3. To enable bootstarp
ifeq ($(USE_OVERWRITE), 1)
DEFINES+=MCUBOOT_OVERWRITE_ONLY
ifeq ($(USE_SW_DOWNGRADE_PREV), 1)
DEFINES+=MCUBOOT_DOWNGRADE_PREVENTION
endif
else
ifeq ($(USE_BOOTSTRAP), 1)
DEFINES+=MCUBOOT_BOOTSTRAP
endif
endif

# Below flag is automatically set/unset by memorymap.mk.
ifeq ($(USE_HW_ROLLBACK_PROT), 1)
DEFINES+=MCUBOOT_HW_ROLLBACK_PROT
endif

# Below flag is automatically set/unset by memorymap.mk.
ifeq ($(USE_MEASURED_BOOT), 1)
DEFINES+=MCUBOOT_MEASURED_BOOT
DEFINES+=MAX_BOOT_RECORD_SZ=512
DEFINES+=MCUBOOT_SHARED_DATA_BASE=0x08000800
DEFINES+=MCUBOOT_SHARED_DATA_SIZE=0x200
endif

# Below flag is automatically set/unset by memorymap.mk.
ifeq ($(USE_DATA_SHARING), 1)
DEFINES+=MCUBOOT_DATA_SHARING
DEFINES+=MAX_BOOT_RECORD_SZ=512
DEFINES+=MCUBOOT_SHARED_DATA_BASE=0x08000800
DEFINES+=MCUBOOT_SHARED_DATA_SIZE=0x200
endif

################################################################################
# MBEDTLS Files
################################################################################

MBEDTLS_PATH=$(MCUBOOT_PATH)/ext/mbedtls

SOURCES+=$(wildcard $(MBEDTLS_PATH)/library/*.c)

INCLUDES+=\
     $(MBEDTLS_PATH)/include\
     $(MBEDTLS_PATH)/include/mbedtls\
     $(MBEDTLS_PATH)/include/psa\
     $(MBEDTLS_PATH)/library

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
COMPONENTS=

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=XMC7xDUAL_CM0P_SLEEP XMC7x_CM0P_SLEEP

# Set this application to run at the CM0+
CORE=CM0P
CORE_NAME=CM0P_0

SOURCES+=\
    $(wildcard $(MCUBOOT_PATH)/boot/bootutil/src/*.c)\
    $(MCUBOOTAPP_PATH)/keys.c

INCLUDES+=\
    $(MCUBOOT_PATH)/boot/bootutil/include/bootutil/crypto\
    $(MCUBOOTAPP_PATH)/os

# To set the protection state as a secure and to prepare a secure MCUBoot based bootloader image
ifeq ($(USE_SECURE_MODE_FOR_MCUBOOT), 1)
SOURCES+=\
     $(MCUBOOT_CY_PATH)/platforms/utils/$(FAMILY)/cy_si_config.c\
     $(MCUBOOT_CY_PATH)/platforms/utils/$(FAMILY)/cy_si_key.c
endif

# The following defines describe the flash map used by MCUBoot
DEFINES+=CY_BOOT_BOOTLOADER_SIZE=$(BOOTLOADER_SIZE)\
         CY_BOOT_SCRATCH_SIZE=$(MCUBOOT_SCRATCH_SIZE)\
         MCUBOOT_LOG_LEVEL=$(MCUBOOT_LOG_LEVEL)

# The following defines used by MCUBoot
DEFINES+=MBEDTLS_CONFIG_FILE='"mcuboot_crypto_config.h"'\
         ECC256_KEY_FILE='"$(SIGN_KEY_FILE).pub"'\
         MCUBOOT_IMAGE_NUMBER=$(MCUBOOT_IMAGE_NUMBER)\
         USE_SHARED_SLOT=$(USE_SHARED_SLOT)\
         MCUBOOT_PLATFORM_CHUNK_SIZE=$(PLATFORM_CHUNK_SIZE)\
         MEMORY_ALIGN=$(PLATFORM_MEMORY_ALIGN)\
         PLATFORM_MAX_TRAILER_PAGE_SIZE=$(PLATFORM_MAX_TRAILER_PAGE_SIZE)\
         MCUBOOT_MAX_IMG_SECTORS=$(MAX_IMG_SECTORS)\
         BOOT_$(CORE)\
         APP_$(APP_CORE)\
         APP_CORE_ID=$(APP_CORE_ID)\
         $(PLATFORM)\
         $(FAMILY)

# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT=hardfp

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

# Pass variables to linker script and overwrite path to it, if custom is required
LDFLAGS+=-Wl,--defsym,CM0P_FLASH_SIZE=$(BOOTLOADER_SIZE)
LDFLAGS+=-Wl,--defsym,CM0P_RAM_SIZE=$(BOOTLOADER_APP_RAM_SIZE)
LDFLAGS+=-Wl,--defsym,USER_APP_FLASH_SIZE=$(APPLICATION_1_BOOT_SLOT_SIZE)

# Additional / custom libraries to link in to the application.
LDLIBS=

# Path to the linker script to use (if empty, use the default linker script).
LINKER_SCRIPT= 

# Custom pre-build commands to run.
# MCUboot uses submodules and "make getlibs" currently does not support
# fetching submodules.
# The PREBUILD command fetches the submodules only if the content of the
# "mcuboot/ext/mbedtls" directory is empty.
PREBUILD_VAR=+\
@if [ -n "$$(ls -A $(MBEDTLS_PATH) 2>/dev/null)" ]; then\
    echo "Git submodules for MCUboot exist. Skipping this step...";\
    else\
    echo "Initializing Git Submodules for MCUboot";\
    cd libs/mcuboot;git submodule update --init --recursive;\
fi;\
rm -f ./source/memorymap.c;\
rm -f ./source/memorymap.h;\
rm -f ./memorymap.mk;\
$(MAKE) generate_flashmap_cm0p;\
$(MAKE) gen_cy_si_key_source_file;
#$(MAKE) gen_key_ecc256; 
#$(MAKE) gen_key_rsa2k;

# Custom pre-build commands to run.
PREBUILD=$(PREBUILD_VAR)

# Custom post-build commands to run.
ifeq ($(USE_SECURE_MODE_FOR_MCUBOOT), 1)
	POSTBUILD=$(CY_PYTHON_PATH) -m cysecuretools -t $(PLATFORM) sign-cysaf -i build/$(TARGET)/$(CONFIG)/$(APPNAME).elf --key-path ../keys/$(SECURE_MODE_KEY_FILE).pem --output build/$(TARGET)/$(CONFIG)/$(APPNAME)_secure.elf;\
	$(CY_TOOLS_DIR)/gcc/bin/arm-none-eabi-objcopy -O ihex build/$(TARGET)/$(CONFIG)/$(APPNAME)_secure.elf build/$(TARGET)/$(CONFIG)/$(APPNAME)_secure.hex;\
	$(CY_TOOLS_DIR)/gcc/bin/arm-none-eabi-objcopy build/$(TARGET)/$(CONFIG)/$(APPNAME)_secure.elf -S -O binary build/$(TARGET)/$(CONFIG)/$(APPNAME)_secure.bin --remove-section .cy_sflash_user_data --remove-section .cy_toc_part2;\
	mv -f build/$(TARGET)/$(CONFIG)/$(APPNAME)_secure.elf build/$(TARGET)/$(CONFIG)/$(APPNAME).elf;\
	mv -f build/$(TARGET)/$(CONFIG)/$(APPNAME)_secure.hex build/$(TARGET)/$(CONFIG)/$(APPNAME).hex;\
	mv -f build/$(TARGET)/$(CONFIG)/$(APPNAME)_secure.bin build/$(TARGET)/$(CONFIG)/$(APPNAME).bin
endif

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
