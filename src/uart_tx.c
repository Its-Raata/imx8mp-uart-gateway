#define _DEFAULT_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#define UART_DEVICE "/dev/ttymxc1"
#define BAUDRATE    B115200


static int write_all(int fd, const void *buffer, size_t length)
{
    const unsigned char *ptr = buffer;
    size_t total_written = 0;

    while (total_written < length) {
        ssize_t written = write(fd,
                                ptr + total_written,
                                length - total_written);

        if (written < 0) {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (written == 0) {
            errno = EIO;
            return -1;
        }

        total_written += (size_t)written;
    }

    return 0;
}

int main(void)
{
    int fd;
    struct termios tty;

    fd = open(UART_DEVICE, O_RDWR | O_NOCTTY);

    if (fd < 0) {
        perror("UART open");
        return EXIT_FAILURE;
    }

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return EXIT_FAILURE;
    }

    /* Configure raw mode */
    cfmakeraw(&tty);

    /* Configure baud rate */
    cfsetispeed(&tty, BAUDRATE);
    cfsetospeed(&tty, BAUDRATE);

    /* Configure 8N1 */
    tty.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
    tty.c_cflag |= CS8 | CLOCAL | CREAD;

    /* Disable hardware and software flow control */
    tty.c_cflag &= ~CRTSCTS;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return EXIT_FAILURE;
    }

    tcflush(fd, TCIOFLUSH);

    printf("UART initialized successfully\n");

    unsigned int counter = 0;

    while (1) {
        char buffer[64];

        int len = snprintf(buffer, sizeof(buffer),
                           "i.MX8MP UART TEST: %u\r\n",
                           counter++);

	if (write_all(fd, buffer, (size_t)len) == -1) {
    	    perror("write");
	    break;
	}

        printf("TX: %s", buffer);

        sleep(1);
    }

    close(fd);

    return EXIT_SUCCESS;
}

