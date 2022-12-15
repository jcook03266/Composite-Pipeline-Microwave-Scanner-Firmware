# @Author: Justin Cook <jcook03266@gmail.com>

from Code.Data_Models.RFScan import *
from Code.Collections.Protocols.Collection_Protocol import *
from Code.Collections.Scan_Data_Batch_Collection import *


# A collection of RFScan objects with convenience methods for
# mutating data, and accessing specific criteria from within the same domain
class ScanDataBatch(GenericCollection):

    def __init__(self,
                 identifier: int = 0,
                 max_count: int = 6,
                 collection: [RFScan] = []):
        self.id = identifier
        self.max_count = max_count
        self.update_collection(collection)

    def get_scan_at(self, index: int) -> RFScan:
        return self.collection.__getitem__(index)

    def get_voltages(self) -> [int]:
        batch: [RFScan] = self.collection
        voltages = [int]

        for scan in batch:
            scan: RFScan = scan
            voltages.append(scan.voltage)

        return voltages

    def get_z_offsets(self) -> [int]:
        batch: [RFScan] = self.collection
        offsets = [int]

        for scan in batch:
            scan: RFScan = scan
            offsets.append(scan.z_offset)

        return offsets
