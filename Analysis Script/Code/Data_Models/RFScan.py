# @Author: Justin Cook <jcook03266@gmail.com>
# General data models for structuring imported serialized data

from datetime import datetime


# This class defines a structured object used for retaining data regarding a
# radio frequency reading converted by a power meter into a voltage output
class RFScan:
    port_number: int  # Also used to identify which antenna this scan corresponds to
    phase_phi: int  # Angular offset (phi) of the antenna
    z_offset: int  # The horizontal offset of the robot when the reading was taken
    voltage: int
    snapshot_time: datetime  # Time at which the reading was taken

    def __init__(self,
                 port_number: int = 1,
                 phase: int = 0,
                 z_offset: int = 0,
                 voltage: int = 0,
                 snapshot_time: datetime = datetime.now()):
        self.id = port_number
        self.port_number = port_number
        self.phase = phase
        self.z_offset = z_offset
        self.voltage = voltage
        self.snapshot_time = snapshot_time
