/******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the source code for CM0P in the XMC7000 OTW Firmware
 *              Upgrade Application for ModusToolbox.
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * Copyright 2023, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 *******************************************************************************/
#include <inttypes.h>
#include <stdbool.h>
#include "cy_pdl.h"
#include "cyhal_wdt.h"
#include "cy_retarget_io.h"
#include "cybsp.h"
#include "cyw_platform_utils.h"
#include "cycfg_pins.h"
#include "cy_result.h"
#include "sysflash/sysflash.h"
#include "flash_map_backend/flash_map_backend.h"
#include "bootutil/image.h"
#include "bootutil/bootutil.h"
#include "bootutil/sign_key.h"
#include "bootutil/bootutil_log.h"
#include "bootutil/fault_injection_hardening.h"

/*******************************************************************************
* Macros
********************************************************************************/
#define CY_RSLT_MODULE_MCUBOOTAPP       0x500U

#define CY_RSLT_MODULE_MCUBOOTAPP_MAIN  0x51U

/** General module error */
#define MCUBOOTAPP_RSLT_ERR             \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MCUBOOTAPP, CY_RSLT_MODULE_MCUBOOTAPP_MAIN, 0))

/* WDT time out for reset mode, in milliseconds. */
#define WDT_TIME_OUT_MS                 4000

#define BOOT_MSG_FINISH                 "Edge Protect Bootloader finished.\r\n" \
                                        "Deinitializing hardware..."

/*******************************************************************************
* Function Prototypes
********************************************************************************/
static inline __attribute__((always_inline))
fih_uint calc_app_addr(uintptr_t flash_base, const struct boot_rsp *rsp);
static bool do_boot(struct boot_rsp *rsp);
static void hw_deinit(void);

/******************************************************************************
 * Function Name: main
 ******************************************************************************
 * Summary:
 *  System entrance point. This function initializes peripherals, initializes
 *  retarget IO, and performs a boot by calling the MCUboot functions. 
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int
 *
 ******************************************************************************/
int main(void)
{
    struct boot_rsp rsp = {};
    cy_rslt_t result = MCUBOOTAPP_RSLT_ERR;
    bool boot_succeeded = false;
    fih_int fih_status = FIH_FAILURE;

    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT((bool)0);
        /* Loop forever... */
        while (true) {
            __WFI();
        }
    }

    /* enable interrupts */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT((bool)0);
        /* Loop forever... */
        while (true) {
            __WFI();
        }
    }

    BOOT_LOG_INF("Edge Protect Bootloader Started");

    FIH_CALL(boot_go, fih_status, &rsp);

    if (FIH_TRUE == fih_eq(fih_status, FIH_SUCCESS)) {
            BOOT_LOG_INF("User Application validated successfully");
            /* initialize watchdog timer. it should be updated from user app
            * to mark successful start up of this app. if the watchdog is not updated,
            * reset will be initiated by watchdog timer and swap revert operation started
            * to roll back to operable image.
            */
            cyhal_wdt_t *wdt = NULL;

            result = cyhal_wdt_init(wdt, WDT_TIME_OUT_MS);
            if (CY_RSLT_SUCCESS == result) {
                boot_succeeded = do_boot(&rsp);
                if (!boot_succeeded) {
                    BOOT_LOG_ERR("Boot of next app failed");
                }
            } else {
                BOOT_LOG_ERR("Failed to init WDT");
            }
    } else {
            BOOT_LOG_ERR("Edge Protect Bootloader found none of bootable images");
    }

    while (true) {
        if (boot_succeeded) {
            (void)Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
        } else {
            __WFI();
        }
    }
}

/******************************************************************************
 * Function Name: calc_app_addr
 ******************************************************************************
 * Summary:
 *  This function extracts the calculate the application address
 *
 * Parameters:
 *  flash_base - internal flash base address
 *  rsp - Pointer to a structure holding the address to boot from. 
 *
 ******************************************************************************/
static inline __attribute__((always_inline))
fih_uint calc_app_addr(uintptr_t flash_base, const struct boot_rsp *rsp)
{
    return fih_uint_encode(flash_base +
                           rsp->br_image_off +
                           rsp->br_hdr->ih_hdr_size);
}

/******************************************************************************
 * Function Name: do_boot
 ******************************************************************************
 * Summary:
 *  This function extracts the primary image address and enables CM7 to let it boot
 *  from that address.
 *
 * Parameters:
 *  rsp - Pointer to a structure holding the address to boot from. 
 *
 ******************************************************************************/
static bool do_boot(struct boot_rsp *rsp)
{
    uintptr_t flash_base = 0;

    if (rsp != NULL) {
        int status = flash_device_base(rsp->br_flash_dev_id, &flash_base);

        if (0 == status) {
            fih_uint app_addr = calc_app_addr(flash_base, rsp);

            BOOT_LOG_INF("Starting User Application (wait)...");
            if (IS_ENCRYPTED(rsp->br_hdr)) {
                BOOT_LOG_DBG(" * User application is encrypted");
            }
            BOOT_LOG_INF("Start slot Address: 0x%08" PRIx32, (uint32_t)fih_uint_decode(app_addr));

            status = flash_device_base(rsp->br_flash_dev_id, &flash_base);
            if (status != 0 || fih_uint_eq(calc_app_addr(flash_base, rsp), app_addr) != FIH_TRUE) {
                return false;
            }

#if defined APP_CM7
            /* This function does not return */
            BOOT_LOG_INF("Launching app on CM7 core");
            BOOT_LOG_INF(BOOT_MSG_FINISH);
            hw_deinit();
            xmc7000_launch_cm7_app(app_addr);
            return true;
#else
#error "Application should run on Cortex-M7"
#endif /* APP_CM7 */

        } else {
            BOOT_LOG_ERR("Flash device ID not found");
            return false;
        }
    }
    return false;
}

/******************************************************************************
 * Function Name: hw_deinit
 ******************************************************************************
 * Summary:
 *  This function performs the necessary hardware de-initialization.
 *
 ******************************************************************************/
static void hw_deinit(void)
{
  /* Flush the TX buffer, need to be fixed in retarget_io */
    while(cy_retarget_io_is_tx_active()){}
    cy_retarget_io_deinit();
}

/* [] END OF FILE */
