################################################################################
# \file user_config.mk
# \version 1.0
#
# \brief
# Holds configuration and error checking that are common to both the Bootloader
# and DFU applications. Ensure that this file is included in the application's 
# Makefile. 
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
# Shared Configuration
################################################################################

# Flashmap JSON file name
FLASH_MAP?=xmc7000_overwrite_single.json

# Device family name. Ex: PSOC6, XMC7000
FAMILY=XMC7000

# Device platform name. Ex: PLATFORM=XMC7200, PLATFORM=XMC7100
PLATFORM?=XMC7200

# Application core ID. 
# Ex: APP_CORE_ID=0 for user app run by CM7_0, APP_CORE_ID=1 for user app run by CM7_1
APP_CORE_ID?=0

# User app core
APP_CORE=CM7

# Base kit KIT_XMC72_EVK
DEVICE_NAME=XMC7200D_E272K8384

# Name of the key file, used in two places. 
# 1. #included in keys.c for embedding it in the Bootloader app, used for image
#    authentication. 
# 2. Passed as a parameter to the Python module "imgtool" for signing the image
#    in the DFU app Makefile. The path of this key file is set in the DFU
#    app Makefile.
SIGN_KEY_FILE=cypress-test-ec-p256

# RAM size for MCUBoot based Bootloader app run by CM0+
BOOTLOADER_APP_RAM_SIZE=0x00020000

# RAM size  for user app (DFU) run by CM7
USER_APP_RAM_SIZE=0x00060000

# MCUBoot header size
# Must be a multiple of 1024 because of the following reason. 
# CM4 image starts right after the header and the CM4 image begins with the
# interrupt vector table. The starting address of the table must be 1024-bytes
# aligned. See README.md for details.
# Number of bytes to be aligned to = Number of interrupt vectors x 4 bytes.
# (1024 = 256 x 4)
# 
# Header size is used in two places. 
# 1. The location of CM4 image is offset by the header size from the ORIGIN
# value specified in the linker script. 
# 2. Passed to the imgtool while signing the image. The imgtool fills the space
# of this size with zeroes and then adds the actual header from the beginning of
# the image.
BOOT_HEADER_SIZE=0x400

# Default memory single chunk size
# Default upgrade method
PLATFORM_DEFAULT_USE_OVERWRITE?=0
PLATFORM_CHUNK_SIZE=0x200

# Minimum erase size of underlying memory hardware
PLATFORM_MEMORY_ALIGN=0x200
PLATFORM_MAX_TRAILER_PAGE_SIZE=0x8000

################################################################################
# MCUBOOT App Configuration
################################################################################

# To prepare a secure MCUBoot image, set the variable to '1'
USE_SECURE_MODE_FOR_MCUBOOT?=1

# Secure boot key file name and type
SECURE_MODE_KEY_TYPE=RSA2048
SECURE_MODE_KEY_FILE=cypress-test-rsa2k

# Downgrade prevention, to avoid older firmware version for upgrade
USE_SW_DOWNGRADE_PREV?=1

# When set to `1` and Swap mode is enabled, the application in the secondary slot will overwrite the primary slot if the primary slot application is invalid.
USE_BOOTSTRAP?=1

################################################################################
# User App Configuration
################################################################################

# Default DFU transport set to I2C.
#
# Supported options are I2C, SPI and UART
#
# Select transport here, as required.
SELECTED_TRANSPORT?=I2C

# Image type to BOOT
IMG_TYPE?=BOOT

# Image ID
IMG_ID=1

# image type can be BOOT or UPGRADE
IMG_TYPES=BOOT UPGRADE
