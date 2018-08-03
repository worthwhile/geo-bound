#include "Adafruit_GPS.h"
#include "AssetTracker.h"
#include "deque"
using namespace std;

//SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

// Used to keep track of the last time we published data
long lastPublish = 0;

int led1 = D7;

typedef struct CONFIG {
    uint8_t antenna;
    uint8_t delay_minutes;
};

typedef struct GPS {
    float latitude;
    float longitude;
    uint32_t timestamp;
};

deque <GPS> gps_data={};

CONFIG config = {};

// Create an AssetTracker
AssetTracker tracker = AssetTracker();

// A FuelGauge for checking on the battery state
FuelGauge fuel;

CONFIG getConfig() {
    CONFIG config = {};
    EEPROM.get(0, config);

    // if config is not set, initialize all settings to defaults
    if (config.antenna == 0xFF) {
        config.antenna = 2;
        config.delay_minutes = 3;
        EEPROM.put(0, config);
    }
    return config;
}

void connectTimeout(){
   Particle.disconnect();
}

void connectToParticle(){
   if (!Particle.connected()){
       Serial.printlnf("Connecting to particle");
       Particle.connect();
       delay(5000);
       Serial.printlnf("Connected %d",(int)Particle.connected());
   }
}

// setup() and loop() are both required. setup() runs once when the device starts
// and is used for registering functions and variables and initializing things
void setup() {
    config = getConfig();

    // Sets up all the necessary AssetTracker bits
    tracker.begin();

    pinMode(led1, OUTPUT);

    // These three functions are useful for remote diagnostics. Read more below.
    Particle.function("batt", batteryStatus);
    Particle.function("gps", gpsPublish);
    Particle.function("antenna", chooseAntenna);
    Particle.function("config", showConfig);
    Particle.function("delay", setDelay);

    showConfig("");
    batteryStatus("");
    tracker.gpsOn();
    configAntenna();
    Serial.begin(9600);
}

// loop() runs continuously
void loop() {
    // capture the GPS output
    tracker.updateGPS();
    connectToParticle();
    sendAll();

    if (millis() - lastPublish > config.delay_minutes * 60 * 1000) {
        // GPS requires a "fix" on the satellites to give good data,
        // so we should only publish data if there's a fix, for real. gpsFix has a glitch so use getFixQuality to be sure.
        if (tracker.gpsFix() && tracker.getFixQuality()) {
            // Remember when we published
            lastPublish = millis();
            digitalWrite(led1, HIGH);
            gpsPublish("G");
            digitalWrite(led1, LOW);
            delay(200);

            // make sure we see the blink
            for (int i = 0; i < 5; i++) {
                digitalWrite(led1, HIGH);
                delay(200);
                digitalWrite(led1, LOW);
                delay(200);
            }
        }
    }
}

int gpsPublish(String command) {
    GPS gps;

    gps.latitude = tracker.readLatDeg();
    gps.longitude = tracker.readLonDeg();
    gps.timestamp = tracker.getGpsTimestamp();
    gps_data.push_front(gps);

    // only keep 2000 readings.  If cell is offline for a long time, then some data will be lost but it becomes less relevant.
    // i tested and it seems that the particle can keep about 7000
    if (gps_data.size()>2000){
        gps_data.pop_back();
    }
    Serial.printlnf("# in Q: %d",gps_data.size());
    sendAll();

    return int(tracker.getFixQuality());
}

int sendAll(){
    if (Cellular.ready()){
        if (Particle.connected()){
            int count = (int)gps_data.size();
            for (int i = 0; i < count && Particle.connected(); i++){
                GPS gps = gps_data.back();
                Serial.printlnf("Sending " + String(gps.latitude) + "," + String(gps.longitude) + "," + String(gps.timestamp));
                if (Particle.publish("G", String(gps.latitude) + "," + String(gps.longitude) + "," + String(gps.timestamp), 60, PRIVATE)){
                    gps_data.pop_back();
                }else{
                    break; //something didn't work, quit.
                }
            }
        }
    }
}

// Lets you remotely check the battery status by calling the function "batt"
// Triggers a publish with the info (so subscribe or watch the dashboard)
// and also returns a '1' if there's >10% battery left and a '0' if below
int batteryStatus(String command) {
    // Publish the battery voltage and percentage of battery remaining
    // if you want to be really efficient, just report one of these
    // the String::format("%f.2") part gives us a string to publish,
    // but with only 2 decimal points to save space
    Particle.publish("B",
        "v:" + String::format("%.2f", fuel.getVCell()) +
        ",c:" + String::format("%.2f", fuel.getSoC()) + command,
        60, PRIVATE
    );
    // make sure we see the blink
    digitalWrite(led1, HIGH);
    delay(250);
    digitalWrite(led1, LOW);

    // if there's more than 10% of the battery left, then return 1
    if (fuel.getSoC() > 10) {
        return 1;
    }
    // if you're running out of battery, return 0
    else {
        return 0;
    }
}

// remotely choose gps antenna
int chooseAntenna(String command) {
    // 1= internal, 2=external
    int new_value = atoi(command);
    if (new_value) {
        config.antenna = max(new_value, 2);
    }

    EEPROM.put(0, config);
    configAntenna();
    showConfig("");
    return config.antenna;
}

// remotely set publish delay
int setDelay(String command) {
    int new_delay = atoi(command);
    if (new_delay) {
        config.delay_minutes = new_delay;
        EEPROM.put(0, config);
        showConfig("");
    }
    return new_delay;
}

// choose the antenna according to config
int configAntenna() {

    switch (config.antenna) {
    case 1:
        tracker.antennaInternal();
        return 1;
    case 2:
        tracker.antennaExternal();
        return 2;

    default:
        tracker.antennaInternal();
        return 1;
    }
}

// publish the current config
int showConfig(String command) {

    Particle.publish("C", String::format("delayMinutes %d, antenna %d", config.delay_minutes, config.antenna));
    return 1;
}
