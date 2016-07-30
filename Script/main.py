#!/usr/bin/env python

import time
import logging
import sys
import os

logging.basicConfig(level='INFO')

script_dir = os.path.dirname(os.path.realpath(__file__))
updateSensor = os.path.join(script_dir,'UpdateSensor.py')
atomizerScript = os.path.join(script_dir,'atomizer.py')

if __name__ == '__main__':
    logging.info(script_dir)
    logging.info(updateSensor)
    logging.info(atomizerScript)
    try:
        os.popen('python '+atomizerScript)
        os.popen('python '+ updateSensor)
        
    except Exception,e:
        logging.warn(e)
    while True:
        pass