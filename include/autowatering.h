

#include "time.h"
#include "esp_sntp.h"

void autoWatering(void *param);
void initAutoWatering();
void waterIfNeeded(tm timeInfo, int tm_hour, int tm_min, int pin);