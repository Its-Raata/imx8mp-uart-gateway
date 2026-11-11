#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

#define UART_DEVICE     "/dev/ttyUSB0"
#define UART_BAUDRATE   B115200

#define RX_CHUNK_SIZE   128
#define LINE_BUFFER_SIZE 256

int main(int argc, char *argv[])
{
    int fd;
    struct termios tty;

    unsigned char rx_chunk[RX_CHUNK_SIZE];
    unsigned char line_buffer[LINE_BUFFER_SIZE];

    size_t line_length = 0;
    int discarding = 0;

    /* Open UART device */
    const char *device = (argc > 1) ? argv[1] : UART_DEVICE;

    fd = open(device, O_RDONLY | O_NOCTTY);

    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    /* Get current UART configuration */
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return EXIT_FAILURE;
    }

    /* Configure raw mode */
    cfmakeraw(&tty);

    /* Configure baud rate */
    cfsetispeed(&tty, UART_BAUDRATE);
    cfsetospeed(&tty, UART_BAUDRATE);

    /* Configure 8N1 */
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8 | CLOCAL | CREAD;

    /* Disable hardware and software flow control */
    tty.c_cflag &= ~CRTSCTS;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* Blocking read: wait for at least one byte */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    /* Apply configuration */
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("UART receiver initialized. Waiting for data...\n");

    while (1) {

        ssize_t received = read(fd, rx_chunk, sizeof(rx_chunk));

        if (received < 0) {
            if (errno == EINTR)
                continue;

            perror("read");
            break;
        }

        if (received == 0)
            continue;

        /* Process every received byte */
        for (ssize_t i = 0; i < received; i++) {

            unsigned char byte = rx_chunk[i];

            /* Discard oversized messages until the next LF */
            if (discarding) {
                if (byte == '\n')
                    discarding = 0;

                continue;
            }

            /* Check buffer capacity */
            if (line_length >= sizeof(line_buffer)) {

                fprintf(stderr,
                        "Warning: message too long. Discarding it.\n");

                line_length = 0;
                discarding = 1;

                if (byte == '\n')
                    discarding = 0;

                continue;
            }

            /* Store received byte */
            line_buffer[line_length++] = byte;

            /* LF indicates the end of a line */
            if (byte == '\n') {

                size_t message_length = line_length - 1;

                /* Remove CR if the line uses CRLF */
                if (message_length > 0 &&
                    line_buffer[message_length - 1] == '\r') {
                    message_length--;
                }

                printf("RX: ");
                fwrite(line_buffer, 1, message_length, stdout);
                putchar('\n');
                fflush(stdout);

                /* Prepare for the next message */
                line_length = 0;
            }
        }
    }

    close(fd);

    return EXIT_SUCCESS;
}

