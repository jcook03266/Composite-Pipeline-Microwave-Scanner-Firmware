# @Author: Justin Cook <jcook03266@gmail.com>

import Code.Script_Entry_Point.main as main
from Code.Script_Entry_Point.main import *
from Code.Data_Aggregation import *

# Global
px = main.px

# Graph Control
is_heat_map_continuous = True


def graph_heat_map(data: ScanDataBatchCollection):
    voltages = data.transform_batches_to_voltages()
    z_offsets = data.joined_transform_batches_to_z_offsets()

    if main.debug:
        print(z_offsets)
        print(Port_Phi)
        print(data.collection)
        print(voltages)

    fig = px.imshow(voltages,
                    labels=dict(x="Port / Angle Phi(Φ)",
                                y="Z - Horizontal Distance [cm]",
                                color="Voltage"),
                    x=Port_Phi,
                    y=z_offsets,
                    title="Voltage Difference from Reference"
                    )
    fig.update_xaxes(side="top")
    fig.show()
