#ifndef CALCULATOR_SERVER_STUB_H
#define CALCULATOR_SERVER_STUB_H

#include "calculator.h"

struct mul_reply_t *multiply_handler(struct mul_request_t *req);

int calculator_dispatch(int sock);

#endif // CALCULATOR_SERVER_STUB_H
