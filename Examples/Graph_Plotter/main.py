import serial
import time
import threading
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# --- CONFIGURATION ---
SERIAL_PORT = '........'  #Enter the path to the serial port.
BAUD_RATE = 115200  # Match Arduino
WINDOW_SIZE = 500  # How many points to show
SMOOTHING_WINDOW = 5  # Number of data points used in a Moving Average Filter

# --- DATA BUFFERS ---
raw_ecg_buffer = deque([0] * WINDOW_SIZE, maxlen=WINDOW_SIZE)
filtered_ecg_buffer = deque([0] * WINDOW_SIZE, maxlen=WINDOW_SIZE)

# Variables for display
current_bpm = 0
current_rr_ms = 0


class RealTimePeakDetector:
    def __init__(self):
        self.last_peak_time = time.time()
        self.last_peak_height = 0
        self.rr_history = deque(maxlen=5)
        self.bpm_history = deque(maxlen=10)

    def detect(self, signal_window):
        if len(signal_window) < 100: return None, None

        data = list(signal_window)
        current_val = data[-1]

        # Calculate local stats for dynamic thresholding
        recent_segment = data[-50:]
        window_max = max(recent_segment)
        window_avg = sum(recent_segment) / len(recent_segment)

        current_time = time.time()
        time_diff = current_time - self.last_peak_time

        # Primary Threshold (must be significantly above noise)
        base_threshold = window_avg + (window_max - window_avg) * 0.7

        # Dynamic Logic
        is_peak = (current_val == window_max) and (current_val > base_threshold)

        if is_peak:
            # Standard Beat (enough time has passed)
            if time_diff > 0.42:
                return self.register_beat(current_time, current_val, time_diff)

            # Potential Premature Beat (quick but tall)
            # If it's fast (300-420ms), it MUST be at least 85% as tall as the last R-peak
            # This allows detection of real PVCs while ignoring T-waves
            elif time_diff > 0.30 and current_val > (self.last_peak_height * 0.85):
                return self.register_beat(current_time, current_val, time_diff)

        return None, None

    def register_beat(self, timestamp, height, rr_interval):
        self.last_peak_time = timestamp
        self.last_peak_height = height

        # Calculate BPM
        raw_bpm = 60.0 / rr_interval

        # Apply Median Filter to RR intervals
        self.rr_history.append(rr_interval)
        median_rr = sorted(list(self.rr_history))[len(self.rr_history) // 2]

        # Final BPM based on median RR
        final_bpm = int(60.0 / median_rr)

        if 40 < final_bpm < 200:
            return final_bpm, int(median_rr * 1000)
        return None, None


# Initialize Detector
detector = RealTimePeakDetector()


# --- SERIAL THREAD ---
def read_serial_data():
    global current_bpm, current_rr_ms
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)
        ser.reset_input_buffer()
        print(f"Connected to {SERIAL_PORT}. Filtering signal...")

        while True:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line.startswith("E:"):
                    parts = line.split(',')
                    val_str = parts[0].split(':')[1]
                    raw_val = int(val_str)

                    # Add to raw buffer
                    raw_ecg_buffer.append(raw_val)

                    # Apply Smoothing (moving average)
                    if len(raw_ecg_buffer) >= SMOOTHING_WINDOW:
                        # Take average of last N samples
                        recent_samples = list(raw_ecg_buffer)[-SMOOTHING_WINDOW:]
                        smooth_val = sum(recent_samples) / SMOOTHING_WINDOW
                        filtered_ecg_buffer.append(smooth_val)
                    else:
                        filtered_ecg_buffer.append(raw_val)

                    # Detect Heart Rate
                    # If hardware sends HR, use it. If not, calculate it.
                    hw_hr_found = False
                    if len(parts) > 1:
                        for p in parts:
                            if "H:" in p:
                                hr = int(p.split(':')[1])
                                if hr > 0:
                                    current_bpm = hr
                                    hw_hr_found = True

                    # If hardware HR is 0 or missing, Python detector is used
                    if not hw_hr_found:
                        new_bpm, new_rr = detector.detect(filtered_ecg_buffer)
                        if new_bpm is not None:
                            current_bpm = new_bpm
                            current_rr_ms = new_rr

            except Exception:
                pass
    except serial.SerialException:
        print("ERROR: Check Serial Port!")


# --- PLOTTING FUNCTION ---
def update_graph(i, line, text_element):
    if len(filtered_ecg_buffer) < 10: return line, text_element

    # Plot the FILTERED data
    line.set_ydata(filtered_ecg_buffer)

    # Update Text
    text_element.set_text(f"♥ BPM: {current_bpm}\nRR: {current_rr_ms} ms")

    # Auto-scale Y-Axis
    curr_min = min(filtered_ecg_buffer)
    curr_max = max(filtered_ecg_buffer)
    padding = (curr_max - curr_min) * 0.1
    plt.ylim(curr_min - padding, curr_max + padding)

    return line, text_element


# --- MAIN ---
if __name__ == "__main__":
    t = threading.Thread(target=read_serial_data)
    t.daemon = True
    t.start()

    fig, ax = plt.subplots(figsize=(10, 6))
    fig.canvas.manager.set_window_title('Filtered ECG Monitor')

    # Styling
    ax.set_facecolor('#111111')
    fig.patch.set_facecolor('#111111')
    ax.grid(True, color='#333333', linestyle='--', linewidth=0.5)
    ax.tick_params(colors='white')
    ax.spines['bottom'].set_color('white')
    ax.spines['left'].set_color('white')

    # Plot Line (Neon Green)
    line, = ax.plot(list(range(WINDOW_SIZE)), filtered_ecg_buffer, color='#00FF00', lw=1.5)

    hr_text = ax.text(0.02, 0.95, "Initializing...", transform=ax.transAxes,
                      fontsize=16, color='#00FF00', verticalalignment='top',
                      fontweight='bold',
                      bbox=dict(boxstyle='round', facecolor='#222222', edgecolor='#00FF00'))

    ani = animation.FuncAnimation(fig, update_graph, fargs=(line, hr_text),
                                  interval=30, blit=False, cache_frame_data=False)

    print("Starting... If graph is hidden, check PyCharm Settings!")
    plt.show(block=True)