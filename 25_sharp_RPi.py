import time
import PySimpleGUI as sg
from serial import Serial, SerialException
import serial.tools.list_ports as sp
import os
import threading
import csv

FONT_SIZE = 45  # 폰트 크기 1.5배로 키움
SETTING_FONT_SIZE = 22  # 설정 폰트도 키움
SETTING_FILE = 'config.csv'

# 전역변수 먼저 정의
ir_min = 100
ir_max = 6000
hall_min = -10000
hall_max = 10000

sg.theme('DarkBlack1')

# 화면을 가득차게 레이아웃 수정 - IR과 HALL을 한 줄에 배치
main_layout = [
    # Floor 1: RESET button
    [sg.Button("RESET", key='RESET', font=('Helvetica', FONT_SIZE), expand_x=True, size=(10, 1), pad=((5, 5), (5, 15)))],
    
    # Floor 2: IR and HALL side by side (50:50 left and right)
    [sg.Column([
        [sg.Text("IR", key="IR", font=('Helvetica', FONT_SIZE), justification='center', expand_x=True)],
        [sg.Input(key='-IR-', font=('Helvetica', FONT_SIZE-5), justification='center', expand_x=True, pad=((5, 5), (5, 5)), size=(10, 1))]
      ], expand_x=True, pad=((5, 5), (5, 5)), element_justification='center'),
     sg.Column([
        [sg.Text("HALL", key="HALL", font=('Helvetica', FONT_SIZE), justification='center', expand_x=True)],
        [sg.Input(key='-HALL-', font=('Helvetica', FONT_SIZE-5), justification='center', expand_x=True, pad=((5, 5), (5, 5)), size=(10, 1))]
      ], expand_x=True, pad=((5, 5), (5, 5)), element_justification='center')
    ],
    
    # Floor 3: Settings button
    [sg.Button("Settings", key='SETTINGS', font=('Helvetica', FONT_SIZE-5), button_color=('white', 'darkblue'), expand_x=True, pad=((5, 5), (15, 5)))]
]

# Window size and margin minimized - enlarged by 1.5x
window = sg.Window('IR/HALL Sensor', main_layout, size=(780, 375), resizable=True, finalize=True, margins=(2, 2))
window.set_title(f'IR:{ir_min}~{ir_max},HALL:{hall_min}~{hall_max}')

# Full screen mode
# window.Maximize()

hall = []

# Settings window layout - changed to 3-floor structure
def create_settings_window():
    settings_layout = [
        # Floor 1: IR and HALL labels
        [sg.Column([
            [sg.Text('IR', font=('Helvetica', SETTING_FONT_SIZE+5), justification='center', expand_x=True, pad=((0, 0), (10, 10)))]
          ], expand_x=True, element_justification='center'),
         sg.Column([
            [sg.Text('HALL', font=('Helvetica', SETTING_FONT_SIZE+5), justification='center', expand_x=True, pad=((0, 0), (10, 10)))]
          ], expand_x=True, element_justification='center')
        ],
        
        # Floor 2: Minimum value settings
        [sg.Column([
            [sg.Text('Min:', font=('Helvetica', SETTING_FONT_SIZE), pad=((0, 10), (5, 5)))],
            [sg.InputText(str(ir_min), key='-IR_MIN-', font=('Helvetica', SETTING_FONT_SIZE-5), size=(10, 1), justification='center')]
          ], expand_x=True, element_justification='center'),
         sg.Column([
            [sg.Text('Min:', font=('Helvetica', SETTING_FONT_SIZE), pad=((0, 10), (5, 5)))],
            [sg.InputText(str(hall_min), key='-HALL_MIN-', font=('Helvetica', SETTING_FONT_SIZE-5), size=(10, 1), justification='center')]
          ], expand_x=True, element_justification='center')
        ],
        
        # Floor 3: Maximum value settings
        [sg.Column([
            [sg.Text('Max:', font=('Helvetica', SETTING_FONT_SIZE), pad=((0, 10), (15, 5)))],
            [sg.InputText(str(ir_max), key='-IR_MAX-', font=('Helvetica', SETTING_FONT_SIZE-5), size=(10, 1), justification='center')]
          ], expand_x=True, element_justification='center'),
         sg.Column([
            [sg.Text('Max:', font=('Helvetica', SETTING_FONT_SIZE), pad=((0, 10), (15, 5)))],
            [sg.InputText(str(hall_max), key='-HALL_MAX-', font=('Helvetica', SETTING_FONT_SIZE-5), size=(10, 1), justification='center')]
          ], expand_x=True, element_justification='center')
        ],
        
        # Floor 4: Buttons
        [sg.Column([
            [sg.Button('Save', key='-SAVE-', font=('Helvetica', SETTING_FONT_SIZE), size=(8, 1), button_color=('white', 'green'), pad=((5, 5), (20, 10)))],
          ], expand_x=True, element_justification='center'),
         sg.Column([
            [sg.Button('Cancel', key='-CANCEL-', font=('Helvetica', SETTING_FONT_SIZE), size=(8, 1), button_color=('white', 'darkred'), pad=((5, 5), (20, 10)))],
          ], expand_x=True, element_justification='center')
        ]
    ]
    settings_window = sg.Window('Settings', settings_layout, modal=True, finalize=True, size=(500, 400))
    return settings_window

def init_cfg():
    global ir_min, ir_max, hall_min, hall_max
    with open(SETTING_FILE, 'w') as f:
        wr = csv.writer(f)
        wr.writerow(['IR', ir_min, ir_max])
        wr.writerow(['HALL', hall_min, hall_max])


def read_cfg():
    global ir_min, ir_max, hall_min, hall_max
    if not os.path.isfile(SETTING_FILE):
        init_cfg()
        return

    with open(SETTING_FILE, 'r') as f:
        rd = csv.reader(f)

        for line in rd:
            if not line:
                continue
            if line[0] == 'IR':
                ir_min, ir_max = list(map(int, line[1:3]))
            if line[0] == 'HALL':
                hall_min, hall_max = list(map(int, line[1:3]))


def dtr_brige(ser):
    ser.dtr = True
    thread = threading.Timer(1.5, read_ir_hall, args=(ser,))
    thread.daemon = True
    thread.start()


def read_ir_hall(ser):
    ser.readline()
    input_display_color = 'white'
    count = 0
    while True:
        try:
            raw_data = str(ser.readline().decode('utf-8'))
            if not raw_data:
                ser.dtr = False
                # print("dtr false")
                thread = threading.Timer(0.5, dtr_brige, args=(ser,))
                thread.daemon = True
                thread.start()
                break
            if count > 20:
                count = 0
                if input_display_color == 'yellow':
                    input_display_color = 'lightskyblue'
                else:
                    input_display_color = 'yellow'
            count += 1
            window['IR'].update(text_color=input_display_color)
            window['HALL'].update(text_color=input_display_color)
            # print(raw_data)
            raw_data = raw_data.rstrip()
            raw_data = raw_data.split(",")
            ir_value = int(raw_data[0][3:])
            hall_value = int(raw_data[1][5:])
            ir_display(ir_value)
            hall_background_color(hall_value)
        except SerialException as e:
            print(e)
            # window.close()
            # os._exit(1)
        except Exception as e:
            print(e)


def ir_display(value):
    global ir_min, ir_max
    if ir_min < value < ir_max:
        result = f"OK ({value})"
        color = 'lightskyblue'
    elif value == 65535:
        result = f"OPEN ({value})"
        color = 'gray'
    else:
        result = f"NG ({value})"
        color = 'red'

    window['-IR-'].update(result, background_color=color)


def hall_background_color(value):
    global hall_min, hall_max
    hall.insert(0, value)
    if len(hall) > 10:
        hall.pop()

    if any(l for l in hall if l != -1):
        if hall_min < hall[0] < hall_max:
            window['-HALL-'].update(f"OK ({hall[0]})", background_color='lightskyblue')
        else:
            window['-HALL-'].update(f"NG ({hall[0]})", background_color='red')
    else:
        window['-HALL-'].update(f"OPEN ({value})", background_color='gray')


def make_serial(com):
    try:
        return Serial(com, 115200, timeout=0.3)
    except:
        return None


# 설정 파일 읽기
read_cfg()

count = 0
ser_list = []
for port in sp.comports():
    count += 1
    if s := make_serial(port.device):
        ser_list.append(s)
        thread = threading.Timer(1.5, read_ir_hall, args=(s,))
        thread.daemon = True
        thread.start()
        # threading.Thread(target=read_ir_hall, args=(s,), daemon=True).start()

if not count:
    os._exit(1)

# 윈도우 타이틀 업데이트
window.set_title(f'IR:{ir_min}~{ir_max},HALL:{hall_min}~{hall_max}')

while True:
    event, values = window.read()

    if event == sg.WINDOW_CLOSED:
        os._exit(1)
        break
    if event == 'RESET':
        for ser in ser_list:
            ser.dtr = False
        time.sleep(0.5)
        for ser in ser_list:
            ser.dtr = True
    if event == 'SETTINGS':
        settings_window = create_settings_window()
        while True:
            settings_event, settings_values = settings_window.read()
            if settings_event in (sg.WINDOW_CLOSED, '-CANCEL-'):
                settings_window.close()
                break
            if settings_event == '-SAVE-':
                try:
                    ir_min = int(settings_values['-IR_MIN-'])
                    ir_max = int(settings_values['-IR_MAX-'])
                    hall_min = int(settings_values['-HALL_MIN-'])
                    hall_max = int(settings_values['-HALL_MAX-'])
                    init_cfg()  # 설정 저장
                    settings_window.close()
                    window.set_title(f'IR:{ir_min}~{ir_max},HALL:{hall_min}~{hall_max}')
                    break
                except ValueError:
                    sg.popup_error('Please enter numbers only.') 