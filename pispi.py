import spidev
import time
import RPi.GPIO as GPIO

#tested on pi 4
ledPin = 3

# Pin Setup:
GPIO.setmode(GPIO.BCM) # Broadcom pin-numbering scheme
GPIO.setup(ledPin, GPIO.OUT) # LED pin set as output
GPIO.output(ledPin, GPIO.HIGH)

spi_bus = 0
spi_device = 0

spi = spidev.SpiDev()

spi.open(spi_bus, spi_device)
#10MHz
spi.max_speed_hz = 10000000
#sending 2 * 2400 bytes :4800 bytes total
while True:
    time.sleep(2/1000)
    #print('***************************\r\n')
    pTime = time.time()
    GPIO.output(ledPin, GPIO.LOW)
    #for i in range(0,2):
    rcv_byte=[0]*4800
    rcv_byte[0:2400] = spi.xfer2([0]*2400)
    GPIO.output(ledPin, GPIO.HIGH)
    time.sleep(3/1000)
    GPIO.output(ledPin, GPIO.LOW)
    time.sleep(3/1000)
    rcv_byte[2400:4800] = spi.xfer2([0]*2400)
    GPIO.output(ledPin, GPIO.HIGH)
    #for n in range(1,2000):
    #   print(rcv_byte[n])

    cTime = time.time()
    #show elapsed time
    #on my test takes around 6 MS
    print("%f delay"%((cTime-pTime)*1000))
