#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

int main(void)
{
    int fd;
    struct termios tty;
    unsigned char buffer[256];

    fd = open("/dev/ttyUSB0", O_RDONLY | O_NOCTTY);

    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return EXIT_FAILURE;
    }

    cfmakeraw(&tty);

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8 | CLOCAL | CREAD;

    tty.c_cflag &= ~CRTSCTS;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("UART receiver initialized. Waiting for data...\n");

    while (1) {
        ssize_t received = read(fd, buffer, sizeof(buffer));

        if (received < 0) {
            if (errno == EINTR)
                continue;

            perror("read");
            break;
        }

        if (received == 0)
            continue;

        printf("%.*s", (int)received, buffer);
        fflush(stdout);
    }

    close(fd);
    return EXIT_SUCCESS;
}

