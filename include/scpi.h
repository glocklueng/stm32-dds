#ifndef _SCPI_H
#define _SCPI_H

#define SCPI_INPUT_BUFFER_LENGTH 256
#define SCPI_ERROR_QUEUE_SIZE 17

void scpi_init(void);

int scpi_process(char* data, int len);

#endif /* _SCPI_H */
