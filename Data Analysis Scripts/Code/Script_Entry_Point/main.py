#!/usr/bin/env python
# -*- coding: utf-8 -*-

# @Author: Justin Cook <jcook03266@gmail.com>

import plotly.express as px
import pandas as pd
import time as t
import serial
from serial import Serial
from typing import Union
from Code.Data_Models.RFScan import *
from Code.Data_Aggregation.Scan_Data_Aggregation import *

# Instance variables
# Script Env
debug = True

# Robot constants
antenna_count = 6
ports_used = [1, 2, 3, 6, 7, 8]
port_phi_offset = [0, 60, 120, 180, 240, 300]
Port_Phi = [
    f"Port {ports_used[0]} | {port_phi_offset[0]}°"
    , f"Port {ports_used[1]} | {port_phi_offset[1]}°"
    , f"Port {ports_used[2]} | {port_phi_offset[2]}°"
    , f"Port {ports_used[3]} | {port_phi_offset[3]}°"
    , f"Port {ports_used[4]} | {port_phi_offset[4]}°"
    , f"Port {ports_used[5]} | {port_phi_offset[5]}°"
]  # Headers for graph indices

# Communication Cycle tracking / control and debouncing (delaying)
total_transmissions = 0
debounceInterval = 0.5
awaitingData = True
windows_port = "COM3" # Used for running this script on a Windows machine
serial_timeout = 1
stop_bits = serial.STOPBITS_ONE
arduino_baudrate = 9600  # This must match the arduino's baud rate, 9600 is preferred for the Uno
arduino_serial_port: Union[Serial, Serial, Serial, Serial]


def main():
    global arduino_serial_port

    print("Start Serial Communication")
    arduino_serial_port = serial.Serial(port=windows_port,
                                        baudrate=arduino_baudrate,
                                        timeout=serial_timeout,
                                        stopbits=stop_bits)

    # Connection successful, Port is open and ready for communication
    print(f"Connection to Port {windows_port} Successful")

    # Jumpstart the serial port
    arduino_serial_port.flushInput()

    aggregate_scatterplot_data()
    aggregate_heatmap_data()


main()
