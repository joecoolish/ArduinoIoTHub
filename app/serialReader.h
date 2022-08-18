#ifndef JODAY_SERIAL
#define JODAY_SERIAL

void initSerial();
bool readFromSerial(char *prompt, char *buf, int maxLen, int timeout);

#endif