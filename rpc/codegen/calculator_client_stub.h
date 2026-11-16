#ifndef CALCULATOR_CLIENT_STUB_H
#define CALCULATOR_CLIENT_STUB_H

#include "calculator.h"

struct mul_reply_t *multiply(int sock, struct mul_request_t *req);

#endif // CALCULATOR_CLIENT_STUB_H
