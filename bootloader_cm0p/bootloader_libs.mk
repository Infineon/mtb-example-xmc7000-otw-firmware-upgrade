################################################################################
# \file bootloader_libs.mk
# \version 1.0
#
# \brief
# Configuration file for adding/removing source files. Modify this file
# to suit the application needs.
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

-include ./libs/mtb.mk

MCUBOOT_PATH=$(SEARCH_mcuboot)
MCUBOOT_CY_PATH=$(MCUBOOT_PATH)/boot/cypress
MCUBOOTAPP_PATH=$(MCUBOOT_CY_PATH)/MCUBootApp
CY_LIBS_PATH=$(MCUBOOT_CY_PATH)/libs
# Need this for MCUBoot based bootloader app signing
IMGTOOL_PATH=$(SEARCH_mcuboot)/scripts

################################################################################
# MBEDTLS Files
################################################################################

MBEDTLS_PATH=$(MCUBOOT_PATH)/ext/mbedtls

SOURCES+=$(wildcard $(MBEDTLS_PATH)/library/*.c)
     
INCLUDES+=\
     $(MBEDTLS_PATH)/include\
     $(MBEDTLS_PATH)/library

# Define mbedtls configuration file
MBEDTLS_CONFIG_FILE = "\"$(MCUBOOT_PATH)/boot/cypress/MCUBootApp/config/mcuboot_crypto_config.h\""

################################################################################
# MCUboot Files
################################################################################

SOURCES+=\
    $(MCUBOOT_PATH)/boot/bootutil/src/boot_record.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/bootutil_misc.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/bootutil_public.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/caps.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/crc32c.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/encrypted.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/fault_injection_hardening.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/fault_injection_hardening_delay_rng_mbedtls.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/loader.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/swap_misc.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/swap_move.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/swap_scratch.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/swap_status_misc.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/swap_status_part.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/swap_status.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/tlv.c\
    $(MCUBOOT_CY_PATH)/platforms/memory/cy_flash_map.c\
    $(MCUBOOT_CY_PATH)/libs/retarget-io/cy_retarget_io.c\
    $(MCUBOOT_CY_PATH)/platforms/security_counter/cy_security_cnt.c\
    $(wildcard $(MCUBOOT_CY_PATH)/platforms/memory/$(FAMILY)/*.c)\
    $(MCUBOOT_CY_PATH)/platforms/utils/$(FAMILY)/cyw_platform_utils.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/image_validate.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/image_ec.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/image_rsa.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/image_ed25519.c\
    $(MCUBOOTAPP_PATH)/keys.c\
    $(MCUBOOT_PATH)/boot/bootutil/src/image_ec256.c

# To set the protection state as a secure and to prepare a secure MCUBoot based bootloader image
ifeq ($(USE_SECURE_MODE_FOR_MCUBOOT), 1)
SOURCES+=\
     $(MCUBOOT_CY_PATH)/platforms/utils/$(FAMILY)/cy_si_config.c\
     $(MCUBOOT_CY_PATH)/platforms/utils/$(FAMILY)/cy_si_key.c
endif
    
INCLUDES+=\
    ../keys\
    $(MCUBOOT_PATH)/boot/bootutil/include\
    $(MCUBOOT_PATH)/boot/bootutil/include/bootutil\
    $(MCUBOOT_PATH)/boot/bootutil/src\
    $(MCUBOOT_CY_PATH)/libs/retarget-io\
    $(MCUBOOT_CY_PATH)/platforms/memory\
    $(MCUBOOT_CY_PATH)/platforms/memory/sysflash\
    $(MCUBOOT_CY_PATH)/platforms/memory/flash_map_backend\
    $(MCUBOOT_CY_PATH)/platforms/utils/$(FAMILY)\
    $(MCUBOOT_CY_PATH)/platforms/memory/$(FAMILY)\
    $(MCUBOOTAPP_PATH)\
    $(MCUBOOTAPP_PATH)/config\
    $(MCUBOOTAPP_PATH)/config/mcuboot_config\
    $(MCUBOOTAPP_PATH)/os\
