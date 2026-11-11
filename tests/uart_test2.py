import os
import time
import tty

fd = os.open("/dev/pts/5", os.O_WRONLY | os.O_NOCTTY)
tty.setraw(fd)

# Test 3: Send an oversized message (300 bytes)
oversized_message = b'A' * 300 + b'\r\n'
os.write(fd, oversized_message)

time.sleep(0.5)

# Test 4: Send a valid message after the oversized message
os.write(fd, b'i.MX8MP UART TEST: AFTER OVERFLOW\r\n')

os.close(fd)
