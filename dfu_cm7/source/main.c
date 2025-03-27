/******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the source code for CM7 in the XMC7000 OTW Firmware
 *              Upgrade Application for ModusToolbox.
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * Copyright 2023-2025, Cypress Semiconductor Corporation (an Infineon company) or
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
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_dfu.h"
#include "cy_retarget_io.h"

#if !(SWAP_DISABLED) && defined(UPGRADE_IMAGE)
/* Header file which contains the function to Write Image OK flag to the slot trailer */
#include "set_img_ok.h"
#endif

/*******************************************************************************
 * Macros
 ********************************************************************************/
#if defined(UPGRADE_IMAGE)
#define DFU_APP_USER_LED             CYBSP_USER_LED2
#else
#define DFU_APP_USER_LED             CYBSP_USER_LED
#endif

/* Timeout for Cy_DFU_Continue(), in milliseconds */
#define DFU_SESSION_TIMEOUT_MS      (20u)

/* Application ID */
#define USER_APP_ID                 (1u)

/* DFU command timeout */
#define DFU_COMMAND_TIMEOUT_MS      (5000u)

/* Interval for LED toggle */
#define LED_TOGGLE_INTERVAL_MS      (1000u)

#define VERSION_MESSAGE_VER         "[DFU App] Version:"

#define IMAGE_TYPE_MESSAGE_VER      "IMAGE_TYPE:"

#define CORE_NAME_MESSAGE_VER       "CPU:"

/* User input to mark the upgrade image in primary slot permanent */
#define UPGRADE_IMG_PERMANENT_CAPITAL           ('Y')
#define UPGRADE_IMG_PERMANENT_SMALL             ('y')

/* UART function parameter value to wait forever */
#define UART_WAIT_FOR_EVER                      (0)

/*******************************************************************************
 * Function Prototypes
 ********************************************************************************/
static uint32_t get_counter_timeout(uint32_t seconds, uint32_t timeout);
static cy_en_dfu_status_t restart_dfu(uint32_t *state, cy_stc_dfu_params_t *dfu_params);
static void user_app_soft_reset(void);

/*******************************************************************************
 * Global Variables
 ********************************************************************************/
#if defined COMPONENT_DFU_I2C
#define DFU_TRANSPORT_MESSAGE_VER       "I2C"
cy_en_dfu_transport_t selected_transport = CY_DFU_I2C;
#elif defined COMPONENT_DFU_UART
#define DFU_TRANSPORT_MESSAGE_VER       "UART"
cy_en_dfu_transport_t selected_transport = CY_DFU_UART;
#elif defined COMPONENT_DFU_SPI
#define DFU_TRANSPORT_MESSAGE_VER       "SPI"
cy_en_dfu_transport_t selected_transport = CY_DFU_SPI;
#elif defined COMPONENT_DFU_CANFD
#define DFU_TRANSPORT_MESSAGE_VER       "CANFD"
cy_en_dfu_transport_t selected_transport = CY_DFU_CANFD;
#endif

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*
* This is the DFU main function for CM7 CPU.
* 1. Initialize the BSP and retarget-io. 
* 2. Start DFU communication.
* 3. DFU wait for DFU host tool command.
* 3. If updated application has been received successfully.
* 4. Then soft reset the device.
* 5. If DFU_IDLE_TIMEOUT_MS seconds has passed and no new application has been received.
*    Restart the DFU and continue to wait for DFU host command.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void) {
    cy_rslt_t result = CY_RSLT_TYPE_ERROR;

    /* Used to count seconds */
    uint32_t count = 0;

    /* Status codes for DFU API */
    cy_en_dfu_status_t status = CY_DFU_ERROR_UNKNOWN;

    /*
     * DFU state, one of the:
     * - CY_DFU_STATE_NONE
     * - CY_DFU_STATE_UPDATING
     * - CY_DFU_STATE_FINISHED
     * - CY_DFU_STATE_FAILED
     */
    uint32_t state = CY_DFU_STATE_NONE;

    /* Buffer to store DFU commands */
    CY_ALIGN(4) static uint8_t buffer[CY_DFU_SIZEOF_DATA_BUFFER];

    /* Buffer for DFU data packets for transport API */
    CY_ALIGN(4) static uint8_t packet[CY_DFU_SIZEOF_CMD_BUFFER];

    /* Update watchdog timer to mark successful start up of application */
    /* Disabling the Watchdog timer started by the bootloader */
    cyhal_wdt_free(NULL);

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    
    /* Retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    printf("\n===========================\r\n");
    printf("%s %s %s %s %s %s\r\n", VERSION_MESSAGE_VER, IMG_VER_MSG,
    IMAGE_TYPE_MESSAGE_VER, IMG_TYPE_MSG, CORE_NAME_MESSAGE_VER,
    CORE_NAME_MSG);
    printf("\n===========================\r\n");

    /* Initialize the User LED */
    result = cyhal_gpio_init(DFU_APP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_ON);
    
    /* GPIO init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

/* After a successful swap-based upgrade*/
#if !(SWAP_DISABLED) && defined(UPGRADE_IMAGE)
    int img_ok_status = IMG_OK_SET_FAILED;
    uint8_t response;

    if (*((uint8_t*) IMG_OK_ADDR) != USER_SWAP_IMAGE_OK) {
        printf("[DFU App] Do you want to mark the upgrade image in primary slot permanent (Y/N) ?\r\n");
        cyhal_uart_getc(&cy_retarget_io_uart_obj, &response, UART_WAIT_FOR_EVER);
        printf("[DFU App] Received response: %c\r\n", response);

        if ((UPGRADE_IMG_PERMANENT_CAPITAL == response) || (UPGRADE_IMG_PERMANENT_SMALL == response)) {
            /* Write Image OK flag to the slot trailer, so MCUBoot-loader
             * will not revert new image
             */
            img_ok_status = set_img_ok(IMG_OK_ADDR, USER_SWAP_IMAGE_OK);

            if (IMG_OK_ALREADY_SET == img_ok_status) {
                printf("[DFU App] Img_ok is already set in trailer\r\n");
            } else if (IMG_OK_SET_SUCCESS == img_ok_status) {
                printf("[DFU App] SWAP Status : Image OK was set at 0x%08x.\r\n", IMG_OK_ADDR);
            } else {
                printf("[DFU App] SWAP Status : Failed to set Image OK.\r\n");
            }
        } else {
            printf("[DFU App] The upgrade image was not marked permanent. Revert swap will happen in the next boot.\r\n");
        }
    } else {
        printf("[DFU App] Image OK is already set in trailer\r\n");
    }

#endif /* !(SWAP_DISABLED) && defined(UPGRADE_IMAGE) */

    /* DFU params, used to configure DFU */
    cy_stc_dfu_params_t dfu_params;

    /* Initialize dfu_params structure */
    dfu_params.timeout = DFU_SESSION_TIMEOUT_MS;
    dfu_params.dataBuffer = &buffer[0];
    dfu_params.packetBuffer = &packet[0];

    /* Initialize DFU */
    status = Cy_DFU_Init(&state, &dfu_params);

    /* DFU init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize DFU communication */
    Cy_DFU_TransportStart(selected_transport);

    /* enable interrupts */
    __enable_irq();

    printf("[DFU App] %s DFU TRANSPORT STARTED !!!\r\n", DFU_TRANSPORT_MESSAGE_VER);

    for (;;) {
        status = Cy_DFU_Continue(&state, &dfu_params);

        ++count;

        if (state == CY_DFU_STATE_FINISHED) {
            /*
             * Finished loading the application image
             * Validate the DFU application. Stop transporting if the application is valid.
             * NOTE Cy_DFU_ValidateApp should be implemented on the application level
             */
            status = Cy_DFU_ValidateApp(USER_APP_ID, &dfu_params);
            if (status == CY_DFU_SUCCESS) {
                printf("[DFU App] Successfully downloaded the upgrade image and placed it into the secondary slot\r\n");
                printf("[DFU App] Reset the device to switch the control to the edge protect bootloader\r\n");
                cyhal_system_delay_ms(50);
                /* Flush the TX buffer, need to be fixed in retarget_io */
                while (cy_retarget_io_is_tx_active()) {
                }
                cy_retarget_io_deinit();
                Cy_DFU_TransportStop();
                user_app_soft_reset();
            } else if (status == CY_DFU_ERROR_VERIFY) {
                /*
                 * Restarts loading, the alternatives to Halt MCU are here
                 * or switch to the other app if it is valid.
                 * Error code can be handled here, which means print to debug UART.
                 */
                printf("[DFU App] Upgrade image verification failed\r\n");
                status = restart_dfu(&state, &dfu_params);
            }
        } else if (state == CY_DFU_STATE_FAILED) {
            /*
             * An error occurred during the loading process.
             * Handle it here. This code just restarts the loading process.
             */
            printf("[DFU App] An error occurred during the loading process\r\n");
            status = restart_dfu(&state, &dfu_params);
        } else if (state == CY_DFU_STATE_UPDATING) {
            /*
             * The DFU_SESSION_TIMEOUT_MS variable is equal to the time DFU waits for the
             * next Host command.
             */
            bool time_out = (count >= (DFU_COMMAND_TIMEOUT_MS / DFU_SESSION_TIMEOUT_MS));
            /*
             * if no command was received within few seconds after the loading
             * started, restart loading.
             */
            if (status == CY_DFU_SUCCESS) {
                count = 0u;
            } else if (status == CY_DFU_ERROR_TIMEOUT) {
                if (time_out != 0u) {
                    count = 0u;
                    restart_dfu(&state, &dfu_params);
                }
            } else {
#if defined COMPONENT_DFU_UART
                /*
                 * DFU updating status is unknown,
                 * Delay because Transport still may be sending an error or success response to the host.
                 * Reset the device, to give a control to edge protect bootloader.
                 * To validate the downloaded image
                 */
                printf("[DFU App] DFU updating status is unknown \r\n");
                printf("[DFU App] Reset the device to switch the control to the edge protect bootloader\r\n");
                printf("[DFU App] To vaild the upgrade image\r\n");
                cyhal_system_delay_ms(50);
                /* Flush the TX buffer, need to be fixed in retarget_io */
                while (cy_retarget_io_is_tx_active()) {
                }
                cy_retarget_io_deinit();
                Cy_DFU_TransportStop();
                user_app_soft_reset();
#else
                count = 0u;
                /* Delay because Transport still may be sending an error response to the host. */
                cyhal_system_delay_ms(50);
                restart_dfu(&state, &dfu_params);
#endif
            }
        }
        /* Blink once per seconds approximately */
        if ((count % get_counter_timeout(LED_TOGGLE_INTERVAL_MS, DFU_SESSION_TIMEOUT_MS)) == 0u) {
            /* Invert the USER LED state */
            cyhal_gpio_toggle(DFU_APP_USER_LED);
        }
    }
}

/*******************************************************************************
 * Function Name: get_counter_timeout
 ********************************************************************************
 * Returns number of counts that correspond to number of seconds passed as
 * a parameter.
 * E.g. comparing counter with 300 seconds is like this.
 * ---
 * uint32_t counter = 0u;
 * for (;;)
 * {
 *     Cy_SysLib_Delay(UART_TIMEOUT);
 *     ++count;
 *     if (count >= get_counter_timeout(seconds: 300u, timeout: UART_TIMEOUT))
 *     {
 *         count = 0u;
 *         DoSomething();
 *     }
 * }
 * ---
 *
 * Both parameters are required to be compile time constants,
 * so this function gets optimized out to single constant value.
 *
 * Parameters:
 *  seconds    Number of seconds to pass. Must be less that 4_294_967 seconds.
 *  timeout    Timeout for Cy_DFU_Continue() function, in milliseconds.
 *             Must be greater than zero.
 *             It is recommended to be a value that produces no reminder
 *             for this function to be precise.
 * Return:
 *  See description.
 *******************************************************************************/
static uint32_t get_counter_timeout(uint32_t seconds, uint32_t timeout) {
    uint32_t count = 1;

    if (timeout != 0) {
        count = ((seconds) / timeout);
    }

    return count;
}

/*******************************************************************************
 * Function Name: restart_dfu
 ********************************************************************************
 * This function re-initializes the DFU and resets the DFU transport.
 *
 * Parameters:
 *  dfu_params     input DFU parameters.
 *  state          input current state of the DFU
 *
 * Return:
 *  Status of operation.
 *******************************************************************************/
static cy_en_dfu_status_t restart_dfu(uint32_t *state, cy_stc_dfu_params_t *dfu_params) {
    cy_en_dfu_status_t status = CY_DFU_SUCCESS;

    if (!state || !dfu_params) {
        status = CY_DFU_ERROR_UNKNOWN;
    }

    /* Restart DFU process */
    if (status == CY_DFU_SUCCESS) {
        status = Cy_DFU_Init(state, dfu_params);
        if (status == CY_DFU_SUCCESS) {
        Cy_DFU_TransportReset();
        }
    }
    printf("[DFU App] Restarted the DFU\r\n");
    return status;
}

/*******************************************************************************
 * Function Name: user_app_soft_reset
 ********************************************************************************
 * soft reset to boot the edge protect bootloader, bootloader will validate 
 * the upgrade image and boot it
 *******************************************************************************/
static void user_app_soft_reset(void) {
    do {
        Cy_SysLib_ClearResetReason();
    } while (Cy_SysLib_GetResetReason() != 0);
    NVIC_SystemReset();
}

/* [] END OF FILE */
