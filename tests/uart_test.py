import os
import time
import sys
import tty

fd = os.open("/dev/pts/5", os.O_WRONLY | os.O_NOCTTY)
tty.setraw(fd)

# Test 1: One message split across three writes
os.write(fd, b"i.MX8MP ")
time.sleep(0.2)
os.write(fd, b"UART TEST: 0 :*\r")
time.sleep(0.2)
os.write(fd, b"\n")

time.sleep(0.5)

# Test 2: Two messages in one write
os.write(fd, b"i.MX8MP UART TEST: 1 :) \r\ni.MX8MP UART TEST: 2 :D \r\n")

os.close(fd)
