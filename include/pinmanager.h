#define MINUTE 60 * 1000
#define WATER_TIME 9000   // 2 * MINUTE
#define WATER_TIME_8 5000 // 10 * MINUTE

extern bool on[10];

void turnOffAfterTime(void *pin);
void setPinState(int pin, bool state);
void buttonPressHandler(int pin, int button);
void runPins(void *);
void setWaterState(void *state);