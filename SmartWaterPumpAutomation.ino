/*****************************************************************************************
 *
 *  Project Name   : Smart Water Pump Automation System
 *  Version        : V1.0 (Stable Release)
 *  Platform       : ESP32 DevKit V1
 *  Framework      : Arduino IDE
 *  Cloud          : Blynk IoT
 *
 *----------------------------------------------------------------------------------------
 *  Developer
 *----------------------------------------------------------------------------------------
 *
 *  Narala Heshma Sree
 *  B.Tech - Electrical & Electronics Engineering
 *
 *----------------------------------------------------------------------------------------
 *  Description
 *----------------------------------------------------------------------------------------
 *
 *  Smart IoT based Water Pump Automation System designed using ESP32
 *  and Blynk IoT Platform.
 *
 *  This controller automates the operation of:
 *
 *      • 2 HP Water Pump
 *      • 6 HP Water Pump
 *
 *  The system provides intelligent motor protection,
 *  automatic operation,
 *  remote monitoring,
 *  fault notifications
 *  and manual control through Blynk IoT.
 *
 *----------------------------------------------------------------------------------------
 *  Main Features
 *----------------------------------------------------------------------------------------
 *
 *  ✓ Manual Mode
 *  ✓ Automatic Mode
 *  ✓ 2 HP Motor Control
 *  ✓ 6 HP Motor Control
 *  ✓ Over Current Protection
 *  ✓ Dry Run Protection
 *  ✓ Tank Full Protection
 *  ✓ Sump Dry Protection
 *  ✓ Sump Full Protection
 *  ✓ Maximum Runtime Protection
 *  ✓ Startup Delay
 *  ✓ Restart Delay
 *  ✓ Live Current Monitoring
 *  ✓ Blynk Notifications
 *  ✓ Last Fault Display
 *  ✓ Protection Bypass
 *  ✓ Remote Monitoring
 *
 *----------------------------------------------------------------------------------------
 *  Hardware Used
 *----------------------------------------------------------------------------------------
 *
 *  • ESP32 DevKit V1
 *  • Relay Module
 *  • Current Sensors (SCT)
 *  • Float Switches
 *  • Motor Contactors
 *  • WiFi Router
 *  • Blynk IoT Platform
 *
 *----------------------------------------------------------------------------------------
 *  Software Used
 *----------------------------------------------------------------------------------------
 *
 *  • Arduino IDE
 *  • Embedded C++
 *  • ESP32 Libraries
 *  • Blynk IoT
 *  • EmonLib
 *
 *----------------------------------------------------------------------------------------
 *  Copyright
 *----------------------------------------------------------------------------------------
 *
 *  Copyright (c) 2026
 *
 *  This project is developed for educational, research
 *  and learning purposes.
 *
 *****************************************************************************************/


//=========================================================================================
//                                 BLYNK CONFIGURATION
//=========================================================================================

// Replace these values with your own Blynk credentials

#define BLYNK_TEMPLATE_ID      "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME    "Motor Automation"
#define BLYNK_AUTH_TOKEN       "YOUR_AUTH_TOKEN"


//=========================================================================================
//                                      LIBRARIES
//=========================================================================================

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <EmonLib.h>


//=========================================================================================
//                                WIFI CONFIGURATION
//=========================================================================================

// Replace with your WiFi credentials

char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";


//=========================================================================================
//                               SYSTEM CONFIGURATION
//=========================================================================================

//--------------------------------------------------
// TEST MODE
//--------------------------------------------------
//
// 1 = Runtime Protection = 50 Seconds
//
// 0 = Runtime Protection = 15 Minutes
//
//--------------------------------------------------

#define TEST_MODE 0


//--------------------------------------------------
// OPERATING MODE
//--------------------------------------------------

bool autoMode = false;

// false = Manual Mode
// true  = Automatic Mode


//=========================================================================================
//                          MOTOR RUNTIME CALCULATION
//=========================================================================================

unsigned long motor2RunTime = 0;
unsigned long motor6RunTime = 0;

unsigned long lastRunTimeUpdate = 0;


//=========================================================================================
//                                 PIN DEFINITIONS
//=========================================================================================


//--------------------------------------------------
// MOTOR OUTPUTS
//--------------------------------------------------

#define RELAY_2HP_PIN          17

#define RELAY_6HP_START        27
#define RELAY_6HP_STOP         13


//--------------------------------------------------
// OVERHEAD TANK FLOAT SWITCHES
//--------------------------------------------------

#define TANK_LOW_PIN           32
#define TANK_FULL_PIN          33


//--------------------------------------------------
// SUMP FLOAT SWITCHES
//--------------------------------------------------

#define SUMP_FULL_PIN          26
#define SUMP_DRY_PIN           25


//--------------------------------------------------
// CURRENT SENSOR INPUTS
//--------------------------------------------------

#define CURRENT_2HP_PIN        34
#define CURRENT_6HP_PIN        35


//=========================================================================================
//                           CURRENT SENSOR CALIBRATION
//=========================================================================================

#define SCT_2HP_CALIBRATION    40

#define SCT_6HP_CALIBRATION    22


//=========================================================================================
//                           MOTOR PROTECTION SETTINGS
//=========================================================================================

//--------------------------------------------------
// Current Limits
//--------------------------------------------------

const float CURRENT_LIMIT_2HP = 13.0;

const float CURRENT_LIMIT_6HP = 13.0;


//--------------------------------------------------
// Dry Run Threshold
//--------------------------------------------------

const float DRY_RUN_CURRENT = 1.0;


//--------------------------------------------------
// Protection Delays
//--------------------------------------------------

const unsigned long DRY_RUN_DELAY = 10000UL;

const unsigned long RESTART_DELAY = 5000UL;


//--------------------------------------------------
// Restart Timer
//--------------------------------------------------

unsigned long motor2HP_StopTime = 0;

unsigned long motor6HP_StopTime = 0;
//=========================================================================================
//                                RUNTIME CONFIGURATION
//=========================================================================================

#if TEST_MODE

// Test Mode : 50 Seconds

const unsigned long MAX_RUN_TIME = 50000UL;

#else

// Normal Mode : 15 Minutes

const unsigned long MAX_RUN_TIME = 15UL * 60UL * 1000UL;

#endif


//=========================================================================================
//                              SYSTEM TIMING SETTINGS
//=========================================================================================

// Ignore Protection Immediately After Motor Start

const unsigned long STARTUP_DELAY      = 10000UL;

// Over Current Trip Delay

const unsigned long OVERCURRENT_DELAY  = 3000UL;

// Dry Run Confirmation Delay

// Dry-run delay uses DRY_RUN_DELAY defined above.

// Relay Pulse Width

const unsigned long RELAY_PULSE_TIME   = 1000UL;


//=========================================================================================
//                              MOTOR PROTECTION TIMERS
//=========================================================================================

//---------------- Over Current ----------------

unsigned long overCurrent2Start = 0;
unsigned long overCurrent6Start = 0;


//---------------- Dry Run ----------------

unsigned long dryRun2Start = 0;
unsigned long dryRun6Start = 0;


//=========================================================================================
//                               PROTECTION FLAGS
//=========================================================================================

//---------------- Over Current ----------------

bool overCurrent2Detected = false;
bool overCurrent6Detected = false;


//---------------- Dry Run ----------------

bool dryRun2Detected = false;
bool dryRun6Detected = false;


//=========================================================================================
//                                 MOTOR STATUS
//=========================================================================================

// Current Running Status

bool motor2HP_State = false;
bool motor6HP_State = false;


// Startup Status

bool motor2Starting = false;
bool motor6Starting = false;


// Startup Lock

bool motor2StartupLock = false;
bool motor6StartupLock = false;


//=========================================================================================
//                               MOTOR TIMERS
//=========================================================================================

// Motor Start Time

unsigned long motor2HP_StartTime = 0;
unsigned long motor6HP_StartTime = 0;


// Motor Stop Time
// Declared in the protection settings section.


//=========================================================================================
//                              RELAY PULSE CONTROL
//=========================================================================================

unsigned long relayPulseStart = 0;

bool relayStartPulse = false;
bool relayStopPulse  = false;


//=========================================================================================
//                              CURRENT MEASUREMENTS
//=========================================================================================

// Instantaneous Current

float current2HP = 0.0;
float current6HP = 0.0;


// Filtered Current

float avgCurrent2HP = 0.0;
float avgCurrent6HP = 0.0;


// Last Update Time

static unsigned long lastCurrentUpdate = 0;


//=========================================================================================
//                           FLOAT SWITCH CONFIGURATION
//=========================================================================================

// Protection Bypass

bool bypassFloat       = false;
bool bypassOverCurrent = false;
bool bypassDryRun      = false;


// Current Monitoring

bool currentMonitoring = true;


//=========================================================================================
//                             FLOAT SWITCH STATUS
//=========================================================================================

//---------------- Overhead Tank ----------------

bool tankLow  = false;
bool tankFull = false;


//---------------- Sump ----------------

bool sumpDry  = false;
bool sumpFull = false;


//=========================================================================================
//                                FAULT STATUS
//=========================================================================================

bool motor2Fault = false;
bool motor6Fault = false;


//=========================================================================================
//                                LAST FAULT
//=========================================================================================

String lastFault = "NONE";


//=========================================================================================
//                                SYSTEM OBJECTS
//=========================================================================================

// Current Measurement

EnergyMonitor emon2HP;
EnergyMonitor emon6HP;


// Blynk Timer

BlynkTimer timer;


//=========================================================================================
//                             FUNCTION PROTOTYPES
//=========================================================================================

//---------------- Core Functions ----------------

void checkSensorsAndTimers();
void processRelayPulse();


//---------------- Motor Control ----------------

void start2HPMotor();
void stop2HPMotor(String reason);

void start6HPMotor();
void stop6HPMotor(String reason);


//---------------- Sensor Functions ----------------

//=========================================================================================
//                                      SETUP
//=========================================================================================

void setup()
{
    //--------------------------------------------------
    // Initialize Serial Monitor
    //--------------------------------------------------

    Serial.begin(115200);

    Serial.println();
    Serial.println("==========================================================");
    Serial.println("      SMART WATER PUMP AUTOMATION SYSTEM");
    Serial.println("              Version 1.0");
    Serial.println("==========================================================");

#if TEST_MODE

    Serial.println("Operating Mode : TEST MODE (50 Seconds)");

#else

    Serial.println("Operating Mode : NORMAL MODE (15 Minutes)");

#endif

    Serial.print("Control Mode   : ");

    if (autoMode)
        Serial.println("AUTOMATIC");
    else
        Serial.println("MANUAL");

    Serial.println();
    Serial.println("Initializing Hardware...");
    Serial.println();


    //==================================================
    // Configure Relay Outputs
    //==================================================

    pinMode(RELAY_2HP_PIN, OUTPUT);
    pinMode(RELAY_6HP_START, OUTPUT);
    pinMode(RELAY_6HP_STOP, OUTPUT);

    // Active LOW Relays

    digitalWrite(RELAY_2HP_PIN, HIGH);
    digitalWrite(RELAY_6HP_START, HIGH);
    digitalWrite(RELAY_6HP_STOP, HIGH);

    Serial.println("Relay Outputs Initialized");


    //==================================================
    // Configure Float Switch Inputs
    //==================================================

    pinMode(TANK_LOW_PIN, INPUT_PULLUP);
    pinMode(TANK_FULL_PIN, INPUT_PULLUP);

    pinMode(SUMP_DRY_PIN, INPUT_PULLUP);
    pinMode(SUMP_FULL_PIN, INPUT_PULLUP);

    Serial.println("Float Switches Initialized");


    //==================================================
    // Initialize Current Sensors
    //==================================================

    emon2HP.current(CURRENT_2HP_PIN, SCT_2HP_CALIBRATION);
    emon6HP.current(CURRENT_6HP_PIN, SCT_6HP_CALIBRATION);

    Serial.println("Current Sensors Initialized");


    //==================================================
    // Connect to WiFi & Blynk
    //==================================================

    Serial.print("Connecting to WiFi... ");

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    Serial.println("Connected");
    Serial.println("Blynk Cloud Connected");


    //==================================================
    // Start Software Timers
    //==================================================

    timer.setInterval(5000L, checkSensorsAndTimers);


    //==================================================
    // System Ready
    //==================================================

    Serial.println();
    Serial.println("==========================================================");
    Serial.println("System Initialization Completed");
    Serial.println("ESP32 Ready");
    Serial.println("Motor Protection Enabled");
    Serial.println("Waiting for Commands...");
    Serial.println("==========================================================");
}



//=========================================================================================
//                                      LOOP
//=========================================================================================

void loop()
{
    //--------------------------------------------------
    // Maintain Blynk Connection
    //--------------------------------------------------

    if (!Blynk.connected())
    {
        Serial.println("Blynk Disconnected...");
        Serial.println("Attempting Reconnection...");

        Blynk.connect();
    }


    //--------------------------------------------------
    // Execute Blynk Tasks
    //--------------------------------------------------

    Blynk.run();


    //--------------------------------------------------
    // Execute Software Timers
    //--------------------------------------------------

    timer.run();


    //--------------------------------------------------
    // Process Relay Pulses
    //--------------------------------------------------

    processRelayPulse();
}
//=========================================================================================
//                               BLYNK CONNECTED EVENT
//=========================================================================================

BLYNK_CONNECTED()
{
    Serial.println();
    Serial.println("==========================================================");
    Serial.println("           CONNECTED TO BLYNK CLOUD");
    Serial.println("==========================================================");

    //--------------------------------------------------
    // Synchronize Dashboard Status
    //--------------------------------------------------

    Blynk.virtualWrite(V5, motor2HP_State);
    Blynk.virtualWrite(V6, motor6HP_State);
    Blynk.virtualWrite(V15, lastFault);

    //--------------------------------------------------
    // Reset Momentary Push Buttons
    //--------------------------------------------------

    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V2, 0);
    Blynk.virtualWrite(V11, 0);
    Blynk.virtualWrite(V12, 0);

    //--------------------------------------------------
    // Synchronize Switch States
    //--------------------------------------------------

    Blynk.syncVirtual(V19);      // Auto / Manual
    Blynk.syncVirtual(V25);      // Current Monitoring
    Blynk.syncVirtual(V26);      // Float Protection Bypass
    Blynk.syncVirtual(V27);      // Over Current Protection Bypass
    Blynk.syncVirtual(V28);      // Dry Run Protection Bypass

    //--------------------------------------------------
    // System Ready
    //--------------------------------------------------

    Serial.println("Dashboard Synchronization Completed");
}



//=========================================================================================
//                           2 HP MOTOR START
//=========================================================================================

BLYNK_WRITE(V1)
{
    if (!param.asInt()) return;

    Serial.println("Command Received : START 2 HP");

    start2HPMotor();

    Blynk.virtualWrite(V1, 0);
}



//=========================================================================================
//                            2 HP MOTOR STOP
//=========================================================================================

BLYNK_WRITE(V11)
{
    if (!param.asInt()) return;

    Serial.println("Command Received : STOP 2 HP");

    stop2HPMotor("Manual Stop");

    Blynk.virtualWrite(V11, 0);
}



//=========================================================================================
//                           6 HP MOTOR START
//=========================================================================================

BLYNK_WRITE(V2)
{
    if (!param.asInt()) return;

    Serial.println("Command Received : START 6 HP");

    start6HPMotor();

    Blynk.virtualWrite(V2, 0);
}



//=========================================================================================
//                            6 HP MOTOR STOP
//=========================================================================================

BLYNK_WRITE(V12)
{
    if (!param.asInt()) return;

    Serial.println("Command Received : STOP 6 HP");

    stop6HPMotor("Manual Stop");

    Blynk.virtualWrite(V12, 0);
}



//=========================================================================================
//                             AUTO / MANUAL MODE
//=========================================================================================

BLYNK_WRITE(V19)
{
    autoMode = param.asInt();

    Blynk.virtualWrite(V20, autoMode);

    if (autoMode)
    {
        Blynk.virtualWrite(V21, "AUTO");
        Serial.println("AUTO MODE ENABLED");
    }
    else
    {
        Blynk.virtualWrite(V21, "MANUAL");
        Serial.println("MANUAL MODE ENABLED");
    }
}



//=========================================================================================
//                     FLOAT PROTECTION BYPASS
//=========================================================================================

BLYNK_WRITE(V26)
{
    bypassFloat = param.asInt();

    Serial.print("Float Protection : ");
    Serial.println(bypassFloat ? "BYPASSED" : "ENABLED");
}



//=========================================================================================
//                  OVER CURRENT PROTECTION BYPASS
//=========================================================================================

BLYNK_WRITE(V27)
{
    bypassOverCurrent = param.asInt();

    Serial.print("Over Current Protection : ");
    Serial.println(bypassOverCurrent ? "BYPASSED" : "ENABLED");
}



//=========================================================================================
//                     DRY RUN PROTECTION BYPASS
//=========================================================================================

BLYNK_WRITE(V28)
{
    bypassDryRun = param.asInt();

    Serial.print("Dry Run Protection : ");
    Serial.println(bypassDryRun ? "BYPASSED" : "ENABLED");
}



//=========================================================================================
//                        CURRENT MONITORING
//=========================================================================================

BLYNK_WRITE(V25)
{
    currentMonitoring = param.asInt();

    Serial.print("Current Monitoring : ");
    Serial.println(currentMonitoring ? "ENABLED" : "DISABLED");
}
//=========================================================================================
//                              START 2 HP MOTOR
//=========================================================================================

void start2HPMotor()
{
    unsigned long currentMillis = millis();

    Serial.println();
    Serial.println("==========================================================");
    Serial.println("Starting 2 HP Motor");
    Serial.println("==========================================================");

    //--------------------------------------------------
    // Check : Motor Already Running
    //--------------------------------------------------

    if (motor2HP_State)
    {
        Serial.println("2 HP Motor is already running.");
        return;
    }

    //--------------------------------------------------
    // Check : Restart Delay Protection
    //--------------------------------------------------

    if ((currentMillis - motor2HP_StopTime) < RESTART_DELAY)
    {
        Serial.println("Restart Delay Active.");
        return;
    }

    //--------------------------------------------------
    // Check : Sump Dry Protection
    //--------------------------------------------------

    if (!bypassFloat && sumpDry)
    {
        Serial.println("Start Aborted : Sump Dry.");

        lastFault = "2 HP : Sump Dry";

        Blynk.virtualWrite(V15, lastFault);
        Blynk.virtualWrite(V1, 0);

        Blynk.logEvent("motor_status", lastFault);

        return;
    }

    //--------------------------------------------------
    // Check : Tank Full Protection
    //--------------------------------------------------

    if (!bypassFloat && tankFull)
    {
        Serial.println("Start Aborted : Tank Full.");

        lastFault = "2 HP : Tank Full";

        Blynk.virtualWrite(V15, lastFault);
        Blynk.virtualWrite(V1, 0);

        Blynk.logEvent("motor_status", lastFault);

        return;
    }

    //--------------------------------------------------
    // Start Motor
    //--------------------------------------------------

    digitalWrite(RELAY_2HP_PIN, LOW);

    //--------------------------------------------------
    // Update Motor Status
    //--------------------------------------------------

    motor2HP_State     = true;
    motor2Fault        = false;

    motor2HP_StartTime = currentMillis;
    motor2RunTime      = 0;

    motor2Starting     = true;
    motor2StartupLock  = true;

    //--------------------------------------------------
    // Reset Protection Timers
    //--------------------------------------------------

    overCurrent2Start = 0;
    dryRun2Start      = 0;

    //--------------------------------------------------
    // Reset Protection Flags
    //--------------------------------------------------

    overCurrent2Detected = false;
    dryRun2Detected      = false;

    //--------------------------------------------------
    // Clear Previous Fault
    //--------------------------------------------------

    lastFault = "NONE";

    //--------------------------------------------------
    // Update Blynk Dashboard
    //--------------------------------------------------

    Blynk.virtualWrite(V5, 1);                  // Motor Running LED
    Blynk.virtualWrite(V15, "2 HP Running");

    // Reset Start Push Button

    Blynk.virtualWrite(V1, 0);

    //--------------------------------------------------
    // Send Notification
    //--------------------------------------------------

    Blynk.logEvent("motor_status", "2 HP Motor Started");

    //--------------------------------------------------
    // Serial Monitor
    //--------------------------------------------------

    Serial.print("Motor Start Time : ");
    Serial.println(motor2HP_StartTime);

    Serial.println("2 HP Motor Started Successfully.");
    Serial.println("==========================================================");
}
//=========================================================================================
//                               STOP 2 HP MOTOR
//=========================================================================================

void stop2HPMotor(String reason)
{
    unsigned long currentMillis = millis();

    //--------------------------------------------------
    // Ignore Manual Stop During Startup
    //--------------------------------------------------

    if (reason == "Manual Stop" &&
        (currentMillis - motor2HP_StartTime < 5000))
    {
        Serial.println("Manual Stop Ignored (Startup Delay)");
        return;
    }

    //--------------------------------------------------
    // Check : Motor Already Stopped
    //--------------------------------------------------

    if (!motor2HP_State)
    {
        Serial.println("2 HP Motor is already stopped.");
        return;
    }

    //--------------------------------------------------
    // Store Stop Time
    //--------------------------------------------------

    motor2HP_StopTime = currentMillis;

    //--------------------------------------------------
    // Stop Motor
    //--------------------------------------------------

    Serial.println();
    Serial.println("==========================================================");
    Serial.println("Stopping 2 HP Motor");
    Serial.println("==========================================================");

    // Active LOW Relay

    digitalWrite(RELAY_2HP_PIN, HIGH);

    //--------------------------------------------------
    // Update Motor Status
    //--------------------------------------------------

    motor2HP_State = false;
    motor2Starting = false;
    motor2StartupLock = false;

    //--------------------------------------------------
    // Update Fault Status
    //--------------------------------------------------

    motor2Fault = (reason != "Manual Stop");

    //--------------------------------------------------
    // Reset Protection Timers
    //--------------------------------------------------

    overCurrent2Start = 0;
    dryRun2Start      = 0;

    //--------------------------------------------------
    // Reset Protection Flags
    //--------------------------------------------------

    overCurrent2Detected = false;
    dryRun2Detected      = false;

    //--------------------------------------------------
    // Store Last Fault
    //--------------------------------------------------

    lastFault = "2 HP : " + reason;

    //--------------------------------------------------
    // Update Dashboard
    //--------------------------------------------------

    Blynk.virtualWrite(V5, 0);                  // Motor Running LED
    Blynk.virtualWrite(V15, lastFault);         // Last Fault
    Blynk.virtualWrite(V22, motor2RunTime);     // Runtime

    // Reset Push Buttons

    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V11, 0);

    //--------------------------------------------------
    // Send Notification
    //--------------------------------------------------

    Blynk.logEvent("motor_status", lastFault);

    //--------------------------------------------------
    // Serial Monitor
    //--------------------------------------------------

    Serial.print("Reason          : ");
    Serial.println(reason);

    Serial.print("Motor Current   : ");
    Serial.print(avgCurrent2HP, 2);
    Serial.println(" A");

    Serial.print("Motor Runtime   : ");
    Serial.print(motor2RunTime);
    Serial.println(" Minutes");

    Serial.println("Motor Status    : STOPPED");

    Serial.println("==========================================================");
}
//=========================================================================================
//                              START 6 HP MOTOR
//=========================================================================================

void start6HPMotor()
{
    unsigned long currentMillis = millis();

    Serial.println();
    Serial.println("==========================================================");
    Serial.println("Starting 6 HP Motor");
    Serial.println("==========================================================");

    //--------------------------------------------------
    // Check : Restart Delay Protection
    //--------------------------------------------------

    if ((currentMillis - motor6HP_StopTime) < RESTART_DELAY)
    {
        Serial.println("Restart Delay Active.");
        return;
    }

    //--------------------------------------------------
    // Check : Motor Already Running
    //--------------------------------------------------

    if (motor6HP_State)
    {
        Serial.println("6 HP Motor is already running.");
        return;
    }

    //--------------------------------------------------
    // Check : START Pulse Already Active
    //--------------------------------------------------

    if (relayStartPulse)
    {
        Serial.println("Start Pulse Already Active.");
        return;
    }

    //--------------------------------------------------
    // Check : Sump Full Protection
    //--------------------------------------------------

    if (!bypassFloat && sumpFull)
    {
        Serial.println("Start Aborted : Sump Already Full.");

        lastFault = "6 HP : Sump Already Full";

        Blynk.virtualWrite(V15, lastFault);
        Blynk.virtualWrite(V2, 0);

        Blynk.logEvent("motor_status", lastFault);

        return;
    }

    //--------------------------------------------------
    // Generate START Pulse
    //--------------------------------------------------

    digitalWrite(RELAY_6HP_START, LOW);

    relayStartPulse = true;
    relayPulseStart = currentMillis;

    //--------------------------------------------------
    // Update Motor Status
    //--------------------------------------------------

    motor6HP_State = true;
    motor6Fault = false;

    motor6HP_StartTime = currentMillis;
    motor6RunTime = 0;

    motor6Starting = true;
    motor6StartupLock = true;

    //--------------------------------------------------
    // Reset Protection Timers
    //--------------------------------------------------

    overCurrent6Start = 0;
    dryRun6Start = 0;

    //--------------------------------------------------
    // Reset Protection Flags
    //--------------------------------------------------

    overCurrent6Detected = false;
    dryRun6Detected = false;

    //--------------------------------------------------
    // Clear Previous Fault
    //--------------------------------------------------

    lastFault = "NONE";

    //--------------------------------------------------
    // Update Dashboard
    //--------------------------------------------------

    Blynk.virtualWrite(V6, 1);                  // Running LED
    Blynk.virtualWrite(V15, "6 HP Running");

    // Reset Start Push Button

    Blynk.virtualWrite(V2, 0);

    //--------------------------------------------------
    // Send Notification
    //--------------------------------------------------

    Blynk.logEvent("motor_status", "6 HP Motor Started");

    //--------------------------------------------------
    // Serial Monitor
    //--------------------------------------------------

    Serial.print("Motor Start Time : ");
    Serial.println(motor6HP_StartTime);

    Serial.println("6 HP Motor Started Successfully.");
    Serial.println("==========================================================");
}
//=========================================================================================
//                               STOP 6 HP MOTOR
//=========================================================================================

void stop6HPMotor(String reason)
{
    unsigned long currentMillis = millis();

    //--------------------------------------------------
    // Ignore Manual Stop During Startup
    //--------------------------------------------------

    if (reason == "Manual Stop" &&
        (currentMillis - motor6HP_StartTime < 5000))
    {
        Serial.println("Manual Stop Ignored (Startup Delay)");
        return;
    }

    //--------------------------------------------------
    // Check : Motor Already Stopped
    //--------------------------------------------------

    if (!motor6HP_State)
    {
        Serial.println("6 HP Motor is already stopped.");
        return;
    }

    //--------------------------------------------------
    // Store Stop Time
    //--------------------------------------------------

    motor6HP_StopTime = currentMillis;

    //--------------------------------------------------
    // Stop Motor
    //--------------------------------------------------

    Serial.println();
    Serial.println("==========================================================");
    Serial.println("Stopping 6 HP Motor");
    Serial.println("==========================================================");

    // Generate STOP Pulse (Active LOW)

    digitalWrite(RELAY_6HP_STOP, LOW);

    relayStopPulse = true;
    relayPulseStart = currentMillis;

    //--------------------------------------------------
    // Update Motor Status
    //--------------------------------------------------

    motor6HP_State = false;
    motor6Starting = false;
    motor6StartupLock = false;

    //--------------------------------------------------
    // Update Fault Status
    //--------------------------------------------------

    motor6Fault = (reason != "Manual Stop");

    //--------------------------------------------------
    // Reset Protection Timers
    //--------------------------------------------------

    overCurrent6Start = 0;
    dryRun6Start = 0;

    //--------------------------------------------------
    // Reset Protection Flags
    //--------------------------------------------------

    overCurrent6Detected = false;
    dryRun6Detected = false;

    //--------------------------------------------------
    // Store Last Fault
    //--------------------------------------------------

    lastFault = "6 HP : " + reason;

    //--------------------------------------------------
    // Update Blynk Dashboard
    //--------------------------------------------------

    Blynk.virtualWrite(V6, 0);                  // Running LED
    Blynk.virtualWrite(V15, lastFault);         // Last Fault
    Blynk.virtualWrite(V23, motor6RunTime);     // Runtime

    // Reset Push Buttons

    Blynk.virtualWrite(V2, 0);
    Blynk.virtualWrite(V12, 0);

    //--------------------------------------------------
    // Send Notification
    //--------------------------------------------------

    Blynk.logEvent("motor_status", lastFault);

    //--------------------------------------------------
    // Serial Monitor
    //--------------------------------------------------

    Serial.print("Reason          : ");
    Serial.println(reason);

    Serial.print("Motor Current   : ");
    Serial.print(avgCurrent6HP, 2);
    Serial.println(" A");

    Serial.print("Motor Runtime   : ");
    Serial.print(motor6RunTime);
    Serial.println(" Minutes");

    Serial.println("Motor Status    : STOPPED");

    if (autoMode)
        Serial.println("Operating Mode  : AUTO");
    else
        Serial.println("Operating Mode  : MANUAL");

    Serial.println("==========================================================");
}
//=========================================================================================
//                           PROCESS RELAY PULSES
//=========================================================================================

void processRelayPulse()
{
    unsigned long currentMillis = millis();

    //--------------------------------------------------
    // Process START Relay Pulse
    //--------------------------------------------------

    if (relayStartPulse)
    {
        if ((currentMillis - relayPulseStart) >= RELAY_PULSE_TIME)
        {
            // Release START Relay (Active LOW)

            digitalWrite(RELAY_6HP_START, HIGH);

            relayStartPulse = false;

            Serial.println("6 HP START Pulse Completed");
        }
    }

    //--------------------------------------------------
    // Process STOP Relay Pulse
    //--------------------------------------------------

    if (relayStopPulse)
    {
        if ((currentMillis - relayPulseStart) >= RELAY_PULSE_TIME)
        {
            // Release STOP Relay (Active LOW)

            digitalWrite(RELAY_6HP_STOP, HIGH);

            relayStopPulse = false;

            Serial.println("6 HP STOP Pulse Completed");
        }
    }
}
//=========================================================================================
//                         CHECK SENSORS & PROTECTION LOGIC
//=========================================================================================

void checkSensorsAndTimers()
{
    const unsigned long currentMillis = millis();
    static unsigned long lastPrint = 0;

    //=============================================================================
    // Read Float Switch Status
    //=============================================================================

    sumpDry  = (digitalRead(SUMP_DRY_PIN)  == LOW);
    sumpFull = (digitalRead(SUMP_FULL_PIN) == LOW);

    tankLow  = (digitalRead(TANK_LOW_PIN)  == LOW);
    tankFull = (digitalRead(TANK_FULL_PIN) == LOW);

    //=============================================================================
    // Automatic Operation
    //=============================================================================

    if (autoMode)
    {
        // Tank Low -> Start 2 HP
        if (tankLow && !tankFull && !motor2HP_State)
        {
            start2HPMotor();
        }

        // Tank Full -> Stop 2 HP
        if (!bypassFloat && tankFull && motor2HP_State)
        {
            stop2HPMotor("Tank Full");
        }

        // Sump Dry -> Start 6 HP
        if (sumpDry && !sumpFull && !motor6HP_State)
        {
            start6HPMotor();
        }

        // Sump Full -> Stop 6 HP
        if (!bypassFloat && sumpFull && motor6HP_State)
        {
            stop6HPMotor("Sump Full");
        }
    }

    //=============================================================================
    // Startup Protection Release
    //=============================================================================
    // Over Current and Dry Run protection are allowed after the startup delay.

    if (motor2Starting &&
        ((currentMillis - motor2HP_StartTime) >= STARTUP_DELAY))
    {
        motor2Starting = false;
        motor2StartupLock = false;

        Serial.println("2 HP Startup Completed.");
    }

    if (motor6Starting &&
        ((currentMillis - motor6HP_StartTime) >= STARTUP_DELAY))
    {
        motor6Starting = false;
        motor6StartupLock = false;

        Serial.println("6 HP Startup Completed.");
    }

    //=============================================================================
    // 2 HP Motor Protection
    //=============================================================================

    if (motor2HP_State)
    {
        //--------------------------------------------------
        // Maximum Runtime Protection
        //--------------------------------------------------

        if ((currentMillis - motor2HP_StartTime) >= MAX_RUN_TIME)
        {
            stop2HPMotor("Run Time Exceeded");
        }

        //--------------------------------------------------
        // Protection After Startup Delay
        //--------------------------------------------------

        if ((currentMillis - motor2HP_StartTime) >= STARTUP_DELAY)
        {
            //--------------------------------------------------
            // Over Current Protection
            //--------------------------------------------------

            if (!bypassOverCurrent &&
                (avgCurrent2HP > CURRENT_LIMIT_2HP))
            {
                if (!overCurrent2Detected)
                {
                    overCurrent2Detected = true;
                    overCurrent2Start = currentMillis;
                }

                if (((currentMillis - overCurrent2Start) >= OVERCURRENT_DELAY) &&
                    !motor2Starting)
                {
                    stop2HPMotor("Over Current");
                }
            }
            else
            {
                overCurrent2Detected = false;
                overCurrent2Start = 0;
            }

            //--------------------------------------------------
            // Dry Run Protection (Current Based)
            //--------------------------------------------------

            if (!bypassDryRun &&
                (avgCurrent2HP < DRY_RUN_CURRENT))
            {
                if (!dryRun2Detected)
                {
                    dryRun2Detected = true;
                    dryRun2Start = currentMillis;
                }

                if ((currentMillis - dryRun2Start) >= DRY_RUN_DELAY)
                {
                    stop2HPMotor("Dry Run");
                }
            }
            else
            {
                dryRun2Detected = false;
                dryRun2Start = 0;
            }
        }

        //--------------------------------------------------
        // Tank Full Protection
        //--------------------------------------------------

        if (!bypassFloat && tankFull)
        {
            stop2HPMotor("Tank Full");
        }

        //--------------------------------------------------
        // Sump Dry Protection
        //--------------------------------------------------

        if (!bypassFloat && sumpDry)
        {
            Serial.println("Sump Dry Detected");
            stop2HPMotor("Sump Dry");
        }

#if TEST_MODE
        Serial.print("2 HP Runtime : ");
        Serial.print((currentMillis - motor2HP_StartTime) / 1000UL);
        Serial.println(" Seconds");
#endif
    }

    //=============================================================================
    // 6 HP Motor Protection
    //=============================================================================

    if (motor6HP_State)
    {
        //--------------------------------------------------
        // Maximum Runtime Protection
        //--------------------------------------------------

        if ((currentMillis - motor6HP_StartTime) >= MAX_RUN_TIME)
        {
            stop6HPMotor("Run Time Exceeded");
        }

        //--------------------------------------------------
        // Protection After Startup Delay
        //--------------------------------------------------

        if ((currentMillis - motor6HP_StartTime) >= STARTUP_DELAY)
        {
            //--------------------------------------------------
            // Over Current Protection
            //--------------------------------------------------

            if (!bypassOverCurrent &&
                (avgCurrent6HP > CURRENT_LIMIT_6HP))
            {
                if (!overCurrent6Detected)
                {
                    overCurrent6Detected = true;
                    overCurrent6Start = currentMillis;
                }

                if (((currentMillis - overCurrent6Start) >= OVERCURRENT_DELAY) &&
                    !motor6Starting)
                {
                    stop6HPMotor("Over Current");
                }
            }
            else
            {
                overCurrent6Detected = false;
                overCurrent6Start = 0;
            }

            //--------------------------------------------------
            // Dry Run Protection (Current Based)
            //--------------------------------------------------

            if (!bypassDryRun &&
                (avgCurrent6HP < DRY_RUN_CURRENT))
            {
                if (!dryRun6Detected)
                {
                    dryRun6Detected = true;
                    dryRun6Start = currentMillis;
                }

                if ((currentMillis - dryRun6Start) >= DRY_RUN_DELAY)
                {
                    stop6HPMotor("Dry Run");
                }
            }
            else
            {
                dryRun6Detected = false;
                dryRun6Start = 0;
            }
        }

        //--------------------------------------------------
        // Sump Full Protection
        //--------------------------------------------------

        if (!bypassFloat && sumpFull)
        {
            stop6HPMotor("Sump Full");
        }

#if TEST_MODE
        Serial.print("6 HP Runtime : ");
        Serial.print((currentMillis - motor6HP_StartTime) / 1000UL);
        Serial.println(" Seconds");
#endif
    }

    //=============================================================================
    // Current Sensor Monitoring
    //=============================================================================

    if (currentMonitoring)
    {
        current2HP = emon2HP.calcIrms(1480);
        current6HP = emon6HP.calcIrms(1480);

        // Remove low-level sensor noise.
        if (current2HP < 0.20)
            current2HP = 0.0;

        if (current6HP < 0.20)
            current6HP = 0.0;

        // Low-pass filtering.
        avgCurrent2HP = (avgCurrent2HP * 0.8) + (current2HP * 0.2);
        avgCurrent6HP = (avgCurrent6HP * 0.8) + (current6HP * 0.2);

        //--------------------------------------------------
        // Update Current Values in Blynk Every 5 Seconds
        //--------------------------------------------------

        if ((currentMillis - lastCurrentUpdate) >= 5000UL)
        {
            lastCurrentUpdate = currentMillis;

            Blynk.virtualWrite(V3, avgCurrent2HP);
            Blynk.virtualWrite(V4, avgCurrent6HP);
        }
    }

    //=============================================================================
    // Water Level Indicators
    //=============================================================================

    Blynk.virtualWrite(V7, tankFull);
    Blynk.virtualWrite(V8, tankLow);
    Blynk.virtualWrite(V9, sumpFull);
    Blynk.virtualWrite(V10, sumpDry);

    //=============================================================================
    // Runtime Calculation
    //=============================================================================

    if ((currentMillis - lastRunTimeUpdate) >= 60000UL)
    {
        lastRunTimeUpdate = currentMillis;

        if (motor2HP_State)
            motor2RunTime++;

        if (motor6HP_State)
            motor6RunTime++;
    }

    //=============================================================================
    // Periodic Diagnostic Output
    //=============================================================================

    if ((currentMillis - lastPrint) >= 5000UL)
    {
        lastPrint = currentMillis;

        Serial.println();
        Serial.println("==========================================================");
        Serial.println("SYSTEM STATUS");
        Serial.println("==========================================================");

        Serial.print("Tank Low       : ");
        Serial.println(tankLow ? "ON" : "OFF");

        Serial.print("Tank Full      : ");
        Serial.println(tankFull ? "ON" : "OFF");

        Serial.print("Sump Dry       : ");
        Serial.println(sumpDry ? "ON" : "OFF");

        Serial.print("Sump Full      : ");
        Serial.println(sumpFull ? "ON" : "OFF");

        Serial.print("2 HP Current   : ");
        Serial.print(avgCurrent2HP, 2);
        Serial.println(" A");

        Serial.print("6 HP Current   : ");
        Serial.print(avgCurrent6HP, 2);
        Serial.println(" A");

        Serial.print("2 HP Motor     : ");
        Serial.println(motor2HP_State ? "RUNNING" : "STOPPED");

        Serial.print("6 HP Motor     : ");
        Serial.println(motor6HP_State ? "RUNNING" : "STOPPED");

        Serial.print("Mode           : ");
        Serial.println(autoMode ? "AUTO" : "MANUAL");

        Serial.println("==========================================================");
    }
}
