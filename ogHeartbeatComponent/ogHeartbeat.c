#include "legato.h"
#include "interfaces.h"
//#include "le_limit_interface.h" cant get this working, switching to shell script
//#include "le_appCtrl_common.h"
//#include "le_limit_interface.h"
//#include "le_appInfo_common.h"

#define BLINK_TIMER_IN_MS (600)
#define APP_START_TIMER_IN_MS (1000)
#define SHUTDOWN_TIMER_IN_MS (6000)

static bool LedOn;
static bool appsRunning;
static bool shuttingDown;
static le_timer_Ref_t AppTimerRef;
static le_timer_Ref_t ShutdownTimerRef;
static le_timer_Ref_t BlinkTimerRef;


static void BlinkTimer
(
	le_timer_Ref_t BlinkTimerRef
)
{
		if(LedOn)
		{
			//mangoh_led_Deactivate();
			LedOn = false;
		}else
		{
			//mangoh_led_Activate();
			LedOn = true;
		}
}
//--------------------------------------------------------------------------------------------------
/**
 * AppTimer is invoked every PUSH_TIMER_IN_MS (1000 right now) and will increment secondCount by one each time
 * initiate shutdown sequence when the timer "ticks" three times, i.e. the pushbutton has been held down for 3xPUSH_TIMER_IN_MS
 *  - make LED blink once shutdown is initiated
 */
//--------------------------------------------------------------------------------------------------
static void AppTimer
(
    le_timer_Ref_t AppTimerRef
)
{
	LE_INFO("AppTimer Invoked");
	le_timer_Start(BlinkTimerRef);	//start blinking to notify user
	le_timer_Stop(ShutdownTimerRef);//stop this from being invoked until after apps start/stop
	le_timer_Stop(AppTimerRef); //stop from this coming in mutliple times
	if (!appsRunning) //start apps
	{
		int systemResult;
		time_t     now;
		struct tm  ts;
		char timestamp[80] = {0};
		char systemCommand[300] = {0};
		
		time(&now);
		
		ts = *localtime(&now);
		strftime(timestamp, sizeof(timestamp), "%Y-%m-%d-%H-%M-%S", &ts);
		
		sprintf(systemCommand, "echo %s > /mnt/userrw/sdcard/lastStartTime.txt", timestamp);
		systemResult = system(systemCommand);

		systemResult = system("/legato/systems/current/apps/ogHeartbeat/read-only/var/ogApps.sh start");
		
		//le_appCtrl_Start("gnssLogger");
		
		//Return value of -1 means that the fork() has failed (see man system).
		if (0 == WEXITSTATUS(systemResult))
		{
			LE_INFO("Starting Apps Success");
			appsRunning = true;
		}
		else
		{
			LE_ERROR("Error starting apps Failed: (%d)", systemResult);
			le_timer_Stop(BlinkTimerRef);	
			//mangoh_led_Deactivate();
			LedOn = false;
		}

	}
	else // stop apps
	{
		int systemResult;
		le_timer_Stop(BlinkTimerRef);
		

		systemResult = system("/legato/systems/current/apps/ogHeartbeat/read-only/var/ogApps.sh stop");
		// Return value of -1 means that the fork() has failed (see man system).
		if (0 == WEXITSTATUS(systemResult))
		{
			LE_INFO("Stopping Apps Success");
			appsRunning = false;
			//mangoh_led_Activate();
			LedOn = true;
		}
		else
		{
			LE_ERROR("Error stopping apps Failed: (%d)", systemResult);
			//mangoh_led_Deactivate();
			LedOn = false;
		}	
		
	}
	le_timer_Start(ShutdownTimerRef);
	
}

//--------------------------------------------------------------------------------------------------
/**
 * AppTimer is invoked every PUSH_TIMER_IN_MS (1000 right now) and will increment secondCount by one each time
 * initiate shutdown sequence when the timer "ticks" three times, i.e. the pushbutton has been held down for 3xPUSH_TIMER_IN_MS
 *  - make LED blink once shutdown is initiated
 */
//--------------------------------------------------------------------------------------------------
static void ShutdownTimer
(
    le_timer_Ref_t AppTimerRef
)
{
	LE_INFO("ShutdownTimer Invoked");
	shuttingDown = true;
	//use this to set boot trigger is desired in future
	//	LE_ASSERT_OK(le_ulpm_BootOnGpio(36, LE_ULPM_GPIO_HIGH));

	//  LE_ASSERT_OK(le_ulpm_ShutDown()); //devmode must be un-installed for this command to work!!

	system("/sbin/sys_shutdown");

	
}


//--------------------------------------------------------------------------------------------------
/**
 * Turn the LED on and disable the timer while the button is pressed. When the  button is released,
 * turn off the LED and start the timer. if it is released after 1 second it will start of stop the apps
 */
//--------------------------------------------------------------------------------------------------
static void PushButtonHandler
(
    bool state, ///< true if the button is pressed
    void *ctx   ///< context pointer - not used
)
{
	if (!shuttingDown){
		if (state) //button pushed
		{
			LE_INFO("Starting Push Button Timer");
			le_timer_Start(AppTimerRef);
			le_timer_Start(ShutdownTimerRef);
		}
		else //button released
		{
			LE_INFO("Stopping push button timer");
			le_timer_Stop(AppTimerRef);
			le_timer_Stop(ShutdownTimerRef);

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
   /* LE_FATAL_IF(
        mangoh_led_SetPushPullOutput(MANGOH_LED_ACTIVE_HIGH, true) != LE_OK,
        "Couldn't configure LED GPIO as a push pull output");
    LedOn = true;*/

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
    AppTimerRef = le_timer_Create("App Timer");
    le_timer_SetMsInterval(AppTimerRef, APP_START_TIMER_IN_MS);
    le_timer_SetRepeat(AppTimerRef, 0);
    le_timer_SetHandler(AppTimerRef, AppTimer);
    
    ShutdownTimerRef = le_timer_Create("Shutdown Timer");
    le_timer_SetMsInterval(ShutdownTimerRef, SHUTDOWN_TIMER_IN_MS);
    le_timer_SetRepeat(ShutdownTimerRef, 0);
    le_timer_SetHandler(ShutdownTimerRef, ShutdownTimer);
    
    BlinkTimerRef = le_timer_Create("Blink Timer");
    le_timer_SetMsInterval(BlinkTimerRef, BLINK_TIMER_IN_MS);
    le_timer_SetRepeat(BlinkTimerRef, 0);
    le_timer_SetHandler(BlinkTimerRef, BlinkTimer);
    
   
    ConfigureGpios();
    
   // mangoh_led_Activate(); //turn on LED to show user ogHeartBeat is on. 
	LedOn = true;
    


}
