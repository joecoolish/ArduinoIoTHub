#ifndef JODAY_MESSAGE
#define JODAY_MESSAGE

#include "viewState.h"

void initSensor();
void parseTwinMessage(char *message, Light *state);
bool readMessage(int messageId, float temperature, float power, char *payload);

#endif