import can
import cantools
import threading
import time
import random
import os

# --- SETTINGS ---
DBC_FILE = "temp/Chademo.dbc"
CAN_INTERFACE = "vcan0"

def get_non_overlapping_signals(message):
    """
    Filters the signals in a message to ensure we don't try to
    write to the same bit twice, which causes encode() to crash.
    """
    seen_bits = set()
    valid_signals = []

    for sig in message.signals:
        # Calculate bit range for this signal
        # This is a simplified check; cantools handles the complex packing
        start = sig.start
        end = sig.start + sig.length

        # Check if any bit in this range is already 'owned' by a signal
        if any(bit in seen_bits for bit in range(start, end)):
            continue # Skip overlapping signal

        for bit in range(start, end):
            seen_bits.add(bit)
        valid_signals.append(sig)

    return valid_signals

def sender_task(bus, message, signals_to_use):
    """Generates raw bits and sends them without any scaling math."""
    interval = (message.cycle_time or 100) / 1000.0

    while True:
        try:
            # 1. Generate Raw bits only.
            # getrandbits(8) guarantees a value between 0-255.
            # It is physically impossible for this to be 'too big'.
            raw_data = {sig.name: random.getrandbits(sig.length) for sig in signals_to_use}

            # 2. Encode with scaling=False.
            # This bypasses the float-to-int math that was crashing.
            data = message.encode(raw_data, scaling=False, strict=False)

            msg = can.Message(
                arbitration_id=message.frame_id,
                data=data,
                is_extended_id=message.is_extended_frame
            )
            bus.send(msg)

        except Exception as e:
            # If this hits, the DBC definition itself is likely illegal
            print(f"Skipping {message.name}: {e}")
            break # Stop this thread if it's fundamentally broken

        time.sleep(interval)

def main():
    print("--- CHAdeMO Simulator: RAW BIT MODE ---")

    if not os.path.exists(DBC_FILE):
        print(f"Error: {DBC_FILE} not found.")
        return

    # Load DBC - strict=False is required for your overlapping file
    try:
        db = cantools.database.load_file(DBC_FILE, strict=False)
        print(f"DBC Loaded. Found {len(db.messages)} messages.")
    except Exception as e:
        print(f"DBC Error: {e}")
        return

    # Initialize Bus
    try:
        bus = can.interface.Bus(channel=CAN_INTERFACE, bustype="socketcan")
        print(f"CAN Bus {CAN_INTERFACE} is online.")
    except Exception as e:
        print(f"CAN Error: {e}")
        return

    threads = []
    for msg in db.messages:
        if not msg.signals:
            continue

        # Critical Step: Remove overlaps before sending to the thread
        safe_signals = get_non_overlapping_signals(msg)

        t = threading.Thread(
            target=sender_task,
            args=(bus, msg, safe_signals),
            daemon=True
        )
        t.start()
        threads.append(t)

    print(f"Successfully started {len(threads)} message threads.")
    print("Use 'candump vcan0' in another terminal to see the data.")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nShutting down.")

if __name__ == "__main__":
    main()