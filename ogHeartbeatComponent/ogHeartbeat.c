/**
 * @file
 *
 * Blinks the user controlled LED at 1Hz. If the push-button is pressed, the LED
 * will remain on until the push-button is released.
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless, Inc. Use of this work is subject to license.
 */

#include "legato.h"
#include "interfaces.h"
//#include "le_limit_interface.h" cant get this working, switching to shell script
//#include "le_appCtrl_common.h"

#define PUSH_TIMER_IN_MS (3000)
//#define LED_BLINK_TIMER_IN_MS (250)

static bool LedOn;
static bool appsRunning;
static bool shuttingDown;
static le_timer_Ref_t PushTimerRef;
//static le_timer_Ref_t LEDBlinkTimerRef;

//--------------------------------------------------------------------------------------------------
/**
 * initiate shutdown sequence when the timer expires, i.e. the pushbutton has been held down for PUSH_TIMER_IN_MS
 *  - make LED blink once shutdown is initiated
 */
//--------------------------------------------------------------------------------------------------
static void PushTimer
(
    le_timer_Ref_t PushTimerRef
)
{
	le_timer_Stop(PushTimerRef);
	mangoh_led_Activate();
	LedOn = true;	    
	usleep(250000);
	mangoh_led_Deactivate();
	LedOn = false;
	usleep(250000);
	mangoh_led_Activate();
	LedOn = true;
	usleep(250000);
	mangoh_led_Deactivate();
	LedOn = false;
	usleep(250000);
	mangoh_led_Activate();
	LedOn = true;
	usleep(250000);
	mangoh_led_Deactivate();
	LedOn = false;
	usleep(250000);
	mangoh_led_Activate();
	LedOn = true;
	//use this to set boot trigger is desired in future
	LE_ASSERT_OK(le_ulpm_BootOnGpio(36, LE_ULPM_GPIO_HIGH));

    LE_ASSERT_OK(le_ulpm_ShutDown()); //devmode must be un-installed for this command to work!!
    shuttingDown = true;
	//le_timer_Start(LEDBlinkTimerRef);
}

//--------------------------------------------------------------------------------------------------
/**
 * initiate shutdown sequence when the timer expires, i.e. the pushbutton has been held down for PUSH_TIMER_IN_MS
 *  - make LED blink once shutdown is initiated
 */
//--------------------------------------------------------------------------------------------------
/*
static void LEDBlinkTimer
(
    le_timer_Ref_t LEDTimerRef
)
{

    if (LedOn)
    {
        mangoh_led_Deactivate();
        LedOn = false;
    }
    else
    {
        mangoh_led_Activate();
        LedOn = true;
    }
}*/

//--------------------------------------------------------------------------------------------------
/**
 * Turn the LED on and disable the timer while the button is pressed. When the  button is released,
 * turn off the LED and start the timer.
 */
//--------------------------------------------------------------------------------------------------
static void PushButtonHandler
(
    bool state, ///< true if the button is pressed
    void *ctx   ///< context pointer - not used
)
{
	if (!shuttingDown){
		if (state)
		{
			LE_DEBUG("Starting Push Button Timer");
			le_timer_Start(PushTimerRef);
		}
		else
		{
			LE_DEBUG("Stopping push button timer");
			le_timer_Stop(PushTimerRef);

				if (!appsRunning)
				{
					//start apps
					int systemResult;

					systemResult = system("/legato/systems/current/apps/ogHeartbeat/read-only/var/ogApps.sh start");
					// Return value of -1 means that the fork() has failed (see man system).
					if (0 == WEXITSTATUS(systemResult))
					{
						LE_INFO("Starting Apps Success");
						mangoh_led_Activate();
						LedOn = true;
						appsRunning = true;
					}
					else
					{
						LE_ERROR("Error starting apps Failed: (%d)", systemResult);
					}

				}
				else
				{
					//stop apps
					int systemResult;

					systemResult = system("/legato/systems/current/apps/ogHeartbeat/read-only/var/ogApps.sh stop");
					// Return value of -1 means that the fork() has failed (see man system).
					if (0 == WEXITSTATUS(systemResult))
					{
						LE_INFO("Stopping Apps Success");
						mangoh_led_Deactivate();
						LedOn = false;
						appsRunning = false;
					}
					else
					{
						LE_ERROR("Error stopping apps Failed: (%d)", systemResult);
					}			

				}
		}
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Sets default configuration LED D750 as on
 */
//--------------------------------------------------------------------------------------------------
static void ConfigureGpios(void)
{
    // Set LED GPIO to output and initially turn the LED ON
    LE_FATAL_IF(
        mangoh_led_SetPushPullOutput(MANGOH_LED_ACTIVE_HIGH, true) != LE_OK,
        "Couldn't configure LED GPIO as a push pull output");
    LedOn = true;

    // Set the push-button GPIO as input
    LE_FATAL_IF(
        mangoh_button_SetInput(MANGOH_BUTTON_ACTIVE_LOW) != LE_OK,
        "Couldn't configure push button as input");
    mangoh_button_AddChangeEventHandler(MANGOH_BUTTON_EDGE_BOTH, PushButtonHandler, NULL, 0);
}
//remember to make sure that AT (AT+WIOCFG) command has been set for gpio and that gpio expander service is installed and running.
COMPONENT_INIT
{
	appsRunning = false;
	shuttingDown = false;
    PushTimerRef = le_timer_Create("Push Button Timer");
    le_timer_SetMsInterval(PushTimerRef, PUSH_TIMER_IN_MS);
    le_timer_SetRepeat(PushTimerRef, 0);
    le_timer_SetHandler(PushTimerRef, PushTimer);

    ConfigureGpios();
    
	mangoh_led_Activate();
	LedOn = true;	    
	usleep(500000);
	mangoh_led_Deactivate();
	LedOn = false;
	usleep(500000);
	mangoh_led_Activate();
	LedOn = true;
	usleep(500000);
	mangoh_led_Deactivate();
	LedOn = false;
	usleep(500000);
	mangoh_led_Activate();
	LedOn = true;
	usleep(500000);
	mangoh_led_Deactivate();
	LedOn = false;


}
