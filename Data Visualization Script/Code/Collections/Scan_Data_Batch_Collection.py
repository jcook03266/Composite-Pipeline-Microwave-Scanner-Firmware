# @Author: Justin Cook <jcook03266@gmail.com>

from Code.Data_Models.RFScan import *
from Code.Collections.Scan_Data_Batch import *
from Code.Collections.Protocols.Collection_Protocol import *


# A collection of Scan Data Batches
class ScanDataBatchCollection(GenericCollection):

    def __init__(self,
                 identifier: int = 0,
                 collection: [ScanDataBatch] = []):
        self.id = identifier
        self.update_collection(collection)

    def get_batch_at(self, index: int) -> ScanDataBatch:
        return self.collection.__getitem__(index)

    # Returns an array of arrays containing all individual voltages for each scan in each batch
    def transform_batches_to_voltages(self) -> [[int]]:
        batches: [ScanDataBatch] = self.collection
        joined_voltages: [[int]] = []

        for batch in batches:
            batch: ScanDataBatch = batch

            voltages = batch.get_voltages()
            joined_voltages.append(voltages)

        return joined_voltages

    # Returns an array of arrays containing all individual z offsets for each scan in each batch
    def transform_batches_to_z_offsets(self) -> [[int]]:
        batches: [ScanDataBatch] = self.collection
        joined_z_offsets: [[int]] = []

        for batch in batches:
            batch: ScanDataBatch = batch

            offsets = batch.get_z_offsets()
            joined_z_offsets.append(offsets)

        return joined_z_offsets

    # Joins all sub-array items into a single array
    def joined_transform_batches_to_z_offsets(self) -> [int]:
        offsets: [[int]] = self.transform_batches_to_z_offsets()
        joined_elements = []

        for offset_group in offsets:
            joined_elements += offset_group

        return joined_elements
