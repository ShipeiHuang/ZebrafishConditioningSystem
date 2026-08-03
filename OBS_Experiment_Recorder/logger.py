from pathlib import Path
from datetime import datetime

Path("logs").mkdir(exist_ok=True)

filename = datetime.now().strftime(
    "logs/log_%Y%m%d_%H%M%S.txt"
)

file = open(filename, "w")


def log(text):

    now = datetime.now().strftime("%H:%M:%S")

    line = f"[{now}] {text}"

    print(line)

    file.write(line + "\n")

    file.flush()


def close():

    file.close()