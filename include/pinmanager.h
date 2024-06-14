extern bool on[9];

void initPins();
void setPinState(int pin, bool state);
void buttonPressHandler(int pin, int button);
void handlePins();