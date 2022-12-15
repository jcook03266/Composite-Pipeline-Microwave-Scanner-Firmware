# @Author: Justin Cook <jcook03266@gmail.com>

import Code.Script_Entry_Point.main as main
from Code.Script_Entry_Point.main import *
from Code.Data_Aggregation import *

# Global
pd = main.pd
px = main.px
dot_size = 10


# Creates a DataFrame and graphs it in a scatter plot format
def graph_scatter_plot(data: [int]):
    df = pd.DataFrame(data, columns=['Port Number', 'Voltage [mV]'])

    # Margin of error definition for the incoming data
    margin_of_error = 0.01  # 1% margin of error
    df["e"] = df["Voltage [mV]"] * margin_of_error

    table = pd.DataFrame(data)

    if main.debug:
        print(df)
        print(table)

    fig = px.scatter(df,
                     x="Port Number",
                     y="Voltage [mV]",
                     color='Voltage [mV]',
                     size=dot_size,
                     trendline="ols",
                     error_y="e")
    fig.show()
