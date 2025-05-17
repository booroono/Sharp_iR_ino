import time
import tkinter as tk
from tkinter import ttk, messagebox
from serial import Serial, SerialException
import serial.tools.list_ports as sp
import os
import threading
import csv

FONT_SIZE = 45
SETTING_FONT_SIZE = 22
SETTING_FILE = 'config.csv'

# Global variables
ir_min = 100
ir_max = 6000
hall_min = -10000
hall_max = 10000

class IRHALLApp:
    def __init__(self, root):
        self.root = root
        self.root.title("IR/HALL Sensor")
        self.root.geometry("780x375")
        self.root.configure(bg='black')
        
        self.hall = []
        self.ser_list = []
        
        # Read configuration
        self.read_cfg()
        
        # Create main layout
        self.create_main_layout()
        
        # Initialize serial connections
        self.init_serial()
        
        # Update window title
        self.update_title()
    
    def create_main_layout(self):
        # Reset button
        self.reset_btn = tk.Button(
            self.root,
            text="RESET",
            font=('Helvetica', FONT_SIZE),
            command=self.reset_serial,
            bg='black',
            fg='white'
        )
        self.reset_btn.pack(fill='x', padx=5, pady=(5, 15))
        
        # IR and HALL frame
        sensor_frame = tk.Frame(self.root, bg='black')
        sensor_frame.pack(fill='x', padx=5, pady=5)
        
        # IR section
        ir_frame = tk.Frame(sensor_frame, bg='black')
        ir_frame.pack(side='left', expand=True, fill='both')
        
        self.ir_label = tk.Label(
            ir_frame,
            text="IR",
            font=('Helvetica', FONT_SIZE),
            bg='black',
            fg='white'
        )
        self.ir_label.pack()
        
        self.ir_value = tk.Entry(
            ir_frame,
            font=('Helvetica', FONT_SIZE-5),
            justify='center',
            bg='black',
            fg='white'
        )
        self.ir_value.pack(fill='x', padx=5, pady=5)
        
        # HALL section
        hall_frame = tk.Frame(sensor_frame, bg='black')
        hall_frame.pack(side='right', expand=True, fill='both')
        
        self.hall_label = tk.Label(
            hall_frame,
            text="HALL",
            font=('Helvetica', FONT_SIZE),
            bg='black',
            fg='white'
        )
        self.hall_label.pack()
        
        self.hall_value = tk.Entry(
            hall_frame,
            font=('Helvetica', FONT_SIZE-5),
            justify='center',
            bg='black',
            fg='white'
        )
        self.hall_value.pack(fill='x', padx=5, pady=5)
        
        # Settings button
        self.settings_btn = tk.Button(
            self.root,
            text="Settings",
            font=('Helvetica', FONT_SIZE-5),
            command=self.show_settings,
            bg='darkblue',
            fg='white'
        )
        self.settings_btn.pack(fill='x', padx=5, pady=(15, 5))
    
    def create_settings_window(self):
        settings = tk.Toplevel(self.root)
        settings.title("Settings")
        settings.geometry("500x400")
        settings.configure(bg='black')
        
        # IR and HALL labels
        labels_frame = tk.Frame(settings, bg='black')
        labels_frame.pack(fill='x', pady=(10, 10))
        
        tk.Label(
            labels_frame,
            text="IR",
            font=('Helvetica', SETTING_FONT_SIZE+5),
            bg='black',
            fg='white'
        ).pack(side='left', expand=True)
        
        tk.Label(
            labels_frame,
            text="HALL",
            font=('Helvetica', SETTING_FONT_SIZE+5),
            bg='black',
            fg='white'
        ).pack(side='right', expand=True)
        
        # Min values
        min_frame = tk.Frame(settings, bg='black')
        min_frame.pack(fill='x', pady=5)
        
        tk.Label(
            min_frame,
            text="Min:",
            font=('Helvetica', SETTING_FONT_SIZE),
            bg='black',
            fg='white'
        ).pack(side='left', padx=(0, 10))
        
        self.ir_min_entry = tk.Entry(
            min_frame,
            font=('Helvetica', SETTING_FONT_SIZE-5),
            width=10
        )
        self.ir_min_entry.pack(side='left')
        self.ir_min_entry.insert(0, str(ir_min))
        
        tk.Label(
            min_frame,
            text="Min:",
            font=('Helvetica', SETTING_FONT_SIZE),
            bg='black',
            fg='white'
        ).pack(side='right', padx=(0, 10))
        
        self.hall_min_entry = tk.Entry(
            min_frame,
            font=('Helvetica', SETTING_FONT_SIZE-5),
            width=10
        )
        self.hall_min_entry.pack(side='right')
        self.hall_min_entry.insert(0, str(hall_min))
        
        # Max values
        max_frame = tk.Frame(settings, bg='black')
        max_frame.pack(fill='x', pady=5)
        
        tk.Label(
            max_frame,
            text="Max:",
            font=('Helvetica', SETTING_FONT_SIZE),
            bg='black',
            fg='white'
        ).pack(side='left', padx=(0, 10))
        
        self.ir_max_entry = tk.Entry(
            max_frame,
            font=('Helvetica', SETTING_FONT_SIZE-5),
            width=10
        )
        self.ir_max_entry.pack(side='left')
        self.ir_max_entry.insert(0, str(ir_max))
        
        tk.Label(
            max_frame,
            text="Max:",
            font=('Helvetica', SETTING_FONT_SIZE),
            bg='black',
            fg='white'
        ).pack(side='right', padx=(0, 10))
        
        self.hall_max_entry = tk.Entry(
            max_frame,
            font=('Helvetica', SETTING_FONT_SIZE-5),
            width=10
        )
        self.hall_max_entry.pack(side='right')
        self.hall_max_entry.insert(0, str(hall_max))
        
        # Buttons
        btn_frame = tk.Frame(settings, bg='black')
        btn_frame.pack(fill='x', pady=(20, 10))
        
        tk.Button(
            btn_frame,
            text="Save",
            font=('Helvetica', SETTING_FONT_SIZE),
            command=lambda: self.save_settings(settings),
            bg='green',
            fg='white'
        ).pack(side='left', expand=True)
        
        tk.Button(
            btn_frame,
            text="Cancel",
            font=('Helvetica', SETTING_FONT_SIZE),
            command=settings.destroy,
            bg='darkred',
            fg='white'
        ).pack(side='right', expand=True)
    
    def show_settings(self):
        self.create_settings_window()
    
    def save_settings(self, settings_window):
        try:
            global ir_min, ir_max, hall_min, hall_max
            ir_min = int(self.ir_min_entry.get())
            ir_max = int(self.ir_max_entry.get())
            hall_min = int(self.hall_min_entry.get())
            hall_max = int(self.hall_max_entry.get())
            self.init_cfg()
            self.update_title()
            settings_window.destroy()
        except ValueError:
            messagebox.showerror("Error", "Please enter numbers only.")
    
    def init_cfg(self):
        with open(SETTING_FILE, 'w') as f:
            wr = csv.writer(f)
            wr.writerow(['IR', ir_min, ir_max])
            wr.writerow(['HALL', hall_min, hall_max])
    
    def read_cfg(self):
        global ir_min, ir_max, hall_min, hall_max
        if not os.path.isfile(SETTING_FILE):
            self.init_cfg()
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
    
    def update_title(self):
        self.root.title(f'IR:{ir_min}~{ir_max}, HALL:{hall_min}~{hall_max}')
    
    def init_serial(self):
        count = 0
        for port in sp.comports():
            count += 1
            if s := self.make_serial(port.device):
                self.ser_list.append(s)
                thread = threading.Timer(1.5, self.read_ir_hall, args=(s,))
                thread.daemon = True
                thread.start()
        
        if not count:
            os._exit(1)
    
    def make_serial(self, com):
        try:
            return Serial(com, 115200, timeout=0.3)
        except:
            return None
    
    def reset_serial(self):
        for ser in self.ser_list:
            ser.dtr = False
        time.sleep(0.5)
        for ser in self.ser_list:
            ser.dtr = True
    
    def dtr_brige(self, ser):
        ser.dtr = True
        thread = threading.Timer(1.5, self.read_ir_hall, args=(ser,))
        thread.daemon = True
        thread.start()
    
    def read_ir_hall(self, ser):
        ser.readline()
        input_display_color = 'white'
        count = 0
        while True:
            try:
                raw_data = str(ser.readline().decode('utf-8'))
                if not raw_data:
                    ser.dtr = False
                    thread = threading.Timer(0.5, self.dtr_brige, args=(ser,))
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
                self.ir_label.config(fg=input_display_color)
                self.hall_label.config(fg=input_display_color)
                raw_data = raw_data.rstrip()
                raw_data = raw_data.split(",")
                try:
                    ir_value = int(raw_data[0][3:])
                    hall_value = int(raw_data[1][5:])
                    self.ir_display(ir_value)
                    self.hall_background_color(hall_value)
                except (IndexError, ValueError) as e:
                    print(f"Data parsing error: {e}, raw_data: {raw_data}")
            except SerialException as e:
                print(e)
            except Exception as e:
                print(e)
    
    def ir_display(self, value):
        if ir_min < value < ir_max:
            result = f"OK ({value})"
            color = 'lightskyblue'
        elif value == 65535:
            result = f"OPEN ({value})"
            color = 'gray'
        else:
            result = f"NG ({value})"
            color = 'red'
        
        self.ir_value.delete(0, tk.END)
        self.ir_value.insert(0, result)
        self.ir_value.config(bg=color)
    
    def hall_background_color(self, value):
        self.hall.insert(0, value)
        if len(self.hall) > 10:
            self.hall.pop()
        
        if any(l for l in self.hall if l != -1):
            if hall_min < self.hall[0] < hall_max:
                result = f"OK ({self.hall[0]})"
                color = 'lightskyblue'
            else:
                result = f"NG ({self.hall[0]})"
                color = 'red'
        else:
            result = f"OPEN ({value})"
            color = 'gray'
        
        self.hall_value.delete(0, tk.END)
        self.hall_value.insert(0, result)
        self.hall_value.config(bg=color)

if __name__ == "__main__":
    root = tk.Tk()
    app = IRHALLApp(root)
    root.mainloop() 