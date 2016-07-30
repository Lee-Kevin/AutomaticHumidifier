#!/usr/bin/env python

import time
import logging
import sys
import os

logging.basicConfig(level='INFO')
min_humi = 20
max_humi = 99
if __name__ == '__main__':

    while True:
        f = open("/tmp/Sensor/humi")
        humi = f.read()
        logging.info(humi)
        if float(humi) < min_humi:
            logging.info("Open the atomizer")
            fs = open("/tmp/Sensor/atomizer","wb")
            fs.write("1")
            fs.close()
        elif float(humi) > max_humi:
            logging.info("Close the atomizer")
            fs = open("/tmp/Sensor/atomizer","wb")
            fs.write("0")
            fs.close()            
        logging.info("the humi is %s" % str(humi))
        time.sleep(60)
        