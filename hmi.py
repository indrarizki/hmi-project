import PySimpleGUI as sg
import matplotlib.pyplot as plt
from io import BytesIO
import matplotlib.dates as mdates
from datetime import datetime, timedelta
import pandas as pd
import serial
import numpy as np

ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)

def create_data_logger():
    data = {
        'Time': [],
        'SENSOR TDS': [],
        'SENSOR TSS': [],
        'SENSOR PH': [],
        'SENSOR CO2': [],
        'SENSOR CH4': []
    }
    return pd.DataFrame(data)

# Fungsi untuk menyimpan plot ke gambar
def save_plot_to_image(plot_type='ALL', width=800, height=600, data=None):
    plt.style.use('dark_background')
    plt.figure(figsize=(width / 80, height / 80))
    
    current_time = datetime.now()
    time_data = [current_time - timedelta(minutes=(9-i) * 10) for i in range(10)]
    
    if data is None:
        data = [np.random.rand(10) for _ in range(5)]
    
    if plot_type == 'ALL':
        titles = ["SENSOR TDS", "SENSOR TURBIDITY", "SENSOR PH", "SENSOR CO2", "SENSOR CH4"]
        y_labels = ["ppm", "NTU", "pH", "ppm", "ppm"] 
        x_labels = ["Time", "Time", "Time", "Time", "Time"]
        
        for i, (title, y_label, x_label) in enumerate(zip(titles, y_labels, x_labels), start=1):
            ax = plt.subplot(3, 2, i)
            ax.plot(time_data, data[title])
            ax.set_title(title, fontsize=14)
            ax.set_xlabel(x_label, fontsize=12)
            ax.set_ylabel(y_label, fontsize=12)
            ax.xaxis.set_major_formatter(mdates.DateFormatter('%m-%d %H:%M'))
            ax.xaxis.set_major_locator(mdates.AutoDateLocator())
            plt.setp(ax.get_xticklabels(), rotation=30, ha="right")
            
            plt.subplots_adjust(hspace=0.7, wspace=0.4, top=0.9)
    else:
        title = plot_type
        y = data[title]
        y_label = "ppm" if "SENSOR" in title else "Units"
        
        ax = plt.gca()
        ax.plot(time_data, y)
        ax.set_title(title, fontsize=14)
        ax.set_xlabel("Time", fontsize=12)
        ax.set_ylabel(y_label, fontsize=12)
        ax.xaxis.set_major_formatter(mdates.DateFormatter('%m-%d %H:%M'))
        ax.xaxis.set_major_locator(mdates.AutoDateLocator())
        plt.setp(ax.get_xticklabels(), rotation=30, ha="right")

    buf = BytesIO()
    plt.savefig(buf, format='png')
    buf.seek(0)
    plt.close()
    
    return buf.getvalue()

# Fungsi untuk ekspor data ke Excel
def export_to_excel(df, selected_sensor='ALL'):
    current_date = datetime.now().strftime('%Y-%m-%d')
    if selected_sensor == 'ALL':
        for sensor in ['SENSOR TDS', 'SENSOR TSS', 'SENSOR PH', 'SENSOR CO2', 'SENSOR CH4']:
            file_name = f"{current_date}_{sensor.replace(' ', '_')}.xlsx"
            df[['Time', sensor]].to_excel(file_name, index=False)
    else:
        if selected_sensor in df.columns:
            file_name = f"{current_date}_{selected_sensor.replace(' ', '_')}.xlsx"
            df[['Time', selected_sensor]].to_excel(file_name, index=False)

# Membuat jendela GUI
def make_window(theme=None):
    sg.theme(theme)
    
    sidebar_layout = [
                        [sg.Text('MONITORING', font='_ 14')],
                        [sg.OptionMenu(['ALL', "SENSOR TDS", "SENSOR TURBIDITY", "SENSOR PH", "SENSOR CO2", "SENSOR CH4"], default_value='ALL', s=(20, 2), key='-OPTIONMENU-')],
                        [sg.Button('Ok'),  sg.Button('Exit')],
                    ]
    
    main_layout = [
                    [sg.Image(key='-IMAGE-', visible=True, expand_x=True, expand_y=True)]  # Expand image to fill the space
                ]
    
    layout = [
        [sg.Column(sidebar_layout, vertical_alignment='top', element_justification='left', size=(200, None)),  # Sidebar on the left
         sg.Column(main_layout, expand_x=True, expand_y=True)]  # Main content on the right
    ]

    window = sg.Window('MONITORING ALAT ....', layout, finalize=True, right_click_menu=sg.MENU_RIGHT_CLICK_EDITME_VER_EXIT, keep_on_top=True, resizable=True)
    
    window.Maximize()
    
    return window

window = make_window()
data = create_data_logger()

# Memperbarui plot pada GUI
def update_plot(plot_type, window, data):
    window_size = window.size
    img_data = save_plot_to_image(plot_type=plot_type, width=window_size[0]-220, height=window_size[1], data=data)
    window['-IMAGE-'].update(data=img_data, visible=True)
    
def update_data(df):
    if ser.in_waiting > 0:
        raw_data = ser.readline().decode('utf-8').strip()
        sensor_values = raw_data.split('|')

        if len(sensor_values) == 5:
            current_time = datetime.now()
            new_data = {
                'Time': current_time,
                'SENSOR TDS': float(sensor_values[0]),
                'SENSOR TURBIDITY': float(sensor_values[1]),
                'SENSOR PH': float(sensor_values[2]),
                'SENSOR CO2': float(sensor_values[3]),
                'SENSOR CH4': float(sensor_values[4])
            }
            df = df.append(new_data, ignore_index=True)
            
            export_to_excel(df, selected_sensor='ALL')

            if len(df) > 10:
                df = df.iloc[1:]

    return df

# Inisialisasi jendela dan data
default_option = 'ALL'
update_plot(default_option, window, data)
df = create_data_logger()

# Event loop
while True:
    event, values = window.read(timeout=1000)
    if event == sg.WIN_CLOSED or event == 'Exit':
        break
    
    if event == 'Ok':
        plot_type = values['-OPTIONMENU-']
        default_option = plot_type
        update_plot(plot_type, window, data)
    
    #if event == 'Export to Excel':
     #   export_to_excel(data, selected_sensor=default_option, file_name=f'{default_option}_data.xlsx')
      #  sg.popup(f'Data {default_option} telah diekspor ke Excel', keep_on_top=True)
      
    df = update_data(df)
    update_plot(default_option, window, df)

window.close()
