extern bool on[10];

void setPinState(int pin, bool state);
void buttonPressHandler(int pin, int button);
void runPins(void *);
void setWaterState(void *state);