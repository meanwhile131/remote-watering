extern bool on[10];

void initPins();
void setPinState(int pin, bool state);
void buttonPressHandler(int pin, int button);
void handlePins();