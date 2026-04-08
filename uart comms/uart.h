#ifndef UART_H_
#define UART_H_

void initUART(void);
void uartSendChar(char c);
void uartSendString(const char *str);

#endif