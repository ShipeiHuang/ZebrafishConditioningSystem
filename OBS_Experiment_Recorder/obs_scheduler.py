import csv
import time
from pathlib import Path
from datetime import datetime
from obsws_python import ReqClient

# ==========================
# OBS SETTINGS
# ==========================

HOST = "localhost"
PORT = 4455
PASSWORD = "zebrafish"

# ==========================
# CONNECT TO OBS
# ==========================

client = ReqClient(
    host=HOST,
    port=PORT,
    password=PASSWORD
)

print("===================================")
print("      OBS Recording Scheduler")
print("===================================")
print("Connected to OBS\n")

# ==========================
# LOAD CSV
# ==========================

schedule = []

with open("schedule.csv", newline="") as f:
    reader = csv.DictReader(f)

    for row in reader:

        def to_seconds(text):
            h, m, s = map(int, text.split(":"))
            return h * 3600 + m * 60 + s

        start = to_seconds(row["Start"])
        stop = to_seconds(row["Stop"])

        schedule.append(("start", start))
        schedule.append(("stop", stop))

schedule.sort(key=lambda x: x[1])

print(f"Loaded {len(schedule)//2} recording periods.\n")

# ==========================
# LOG FILE
# ==========================

Path("logs").mkdir(exist_ok=True)

log_name = datetime.now().strftime("logs/log_%Y%m%d_%H%M%S.txt")

log = open(log_name, "w")

def write_log(text):

    now = datetime.now().strftime("%H:%M:%S")

    line = f"{now}  {text}"

    print(line)

    log.write(line + "\n")

    log.flush()

# ==========================
# WAIT FOR EXPERIMENT
# ==========================

input("Press ENTER when experiment starts...")

experiment_start = time.time()

write_log("Experiment Started")

# ==========================
# MAIN LOOP
# ==========================

recording = False

for action, target in schedule:

    while True:

        elapsed = time.time() - experiment_start

        remain = target - elapsed

        if remain <= 0:
            break

        print(
            f"\rNext: {action.upper():5s}  "
            f"in {remain:8.1f} sec",
            end=""
        )

        time.sleep(0.5)

    print()

    try:

        if action == "start":

            if not recording:

                client.start_record()

                recording = True

                write_log("Start Recording")

            else:

                write_log("Already Recording")

        else:

            if recording:

                client.stop_record()

                recording = False

                write_log("Stop Recording")

            else:

                write_log("Already Stopped")

    except Exception as e:

        write_log(str(e))

# ==========================
# FINISH
# ==========================

write_log("Finished Schedule")

log.close()

print("\nDone.")