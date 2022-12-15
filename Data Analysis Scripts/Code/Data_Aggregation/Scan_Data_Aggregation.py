# @Author: Justin Cook <jcook03266@gmail.com>

import Code.Script_Entry_Point.main as main
from Code.Script_Entry_Point.main import *
from Code.Graphing_Scripts.HeatMap import *
from Code.Graphing_Scripts.ScatterPlot import *
from Code.Collections.Scan_Data_Batch import *
from Code.Collections.Scan_Data_Batch_Collection import *

# Global
arduino_serial_port = main.arduino_serial_port
total_transmissions = main.total_transmissions

# Heat map graphing Data Stores
heatMapData: ScanDataBatchCollection = ScanDataBatchCollection()  # Stores an unbounded amount of heat map data batches
heatMapDataBatch: ScanDataBatch = ScanDataBatch()  # Stores up to 6 scans | 6 antennas ~ 6 scans

# Scatter Plot Data Stores
scatterPlotScanData = []


def aggregate_scatterplot_data():
    global total_transmissions

    while awaitingData:
        incoming_data = arduino_serial_port.readline().decode('utf-8')

        if incoming_data:
            total_transmissions += 1
            print("Transmission " + str(total_transmissions))

            if incoming_data.__contains__("@Start"):
                parsedString = incoming_data.split(",")

                port = parsedString[0].replace('@Start', '')
                phi = parsedString[1]
                voltage = parsedString[2]
                z = parsedString[3]

                if main.debug:
                    print(parsedString)
                    print(parsedString)
                    print(port)
                    print(phi)
                    print(voltage)
                    print(z)

                combination = [int(port), int(voltage)]
                scatterPlotScanData.append(combination)

            elif incoming_data.__contains__("@End"):
                graph_scatter_plot(scatterPlotScanData)

                scatterPlotScanData.clear()
            else:
                print("Arduino Debug Data \n")
                print(incoming_data)

        # Pause between parsing cycles
        t.sleep(debounceInterval)


def aggregate_heatmap_data():
    global total_transmissions

    while awaitingData:
        incoming_data = arduino_serial_port.readline().decode('utf-8')

        if incoming_data:
            total_transmissions += 1
            print("Transmission " + str(total_transmissions))

            if incoming_data.__contains__("@Start"):
                parsedString = incoming_data.split(",")

                port = parsedString[0].replace('@Start', '')
                phi = parsedString[1]
                voltage = parsedString[2]
                z = parsedString[3]

                if main.debug:
                    print(parsedString)
                    print(parsedString)
                    print(port)
                    print(phi)
                    print(voltage)
                    print(z)

                heatMapDataBatch.add(voltage)

                if heatMapDataBatch.is_full():
                    heatMapData.add(heatMapDataBatch.get_collection_copy())
                    heatMapDataBatch.clear()

            elif incoming_data.__contains__("@End"):
                graph_heat_map(heatMapData)

                # If the heat map is not intended to be continuously updated then graph a new version every time
                if not is_heat_map_continuous:
                    heatMapData.clear()

                heatMapDataBatch.clear()
            else:
                print("Arduino Debug Data \n")
                print(incoming_data)

        # Pause between parsing cycles
        t.sleep(debounceInterval)
