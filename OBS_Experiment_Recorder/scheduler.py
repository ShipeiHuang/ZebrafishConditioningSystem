"""
scheduler.py

"""

import csv
import time

from utils import time_to_seconds, seconds_to_hms
from logger import log


class ExperimentScheduler:


    def __init__(self, obs_controller, csv_file):

        self.obs = obs_controller

        self.csv_file = csv_file

        self.events = []


    # -------------------------------------------------

    def load_schedule(self):

        """
        read schedule.csv
        """

        self.events = []


        with open(
            self.csv_file,
            newline="",
            encoding="utf-8"
        ) as file:


            reader = csv.DictReader(file)


            for row in reader:


                label = row["Label"]

                start = time_to_seconds(
                    row["Start"]
                )

                stop = time_to_seconds(
                    row["Stop"]
                )


                self.events.append(
                    {
                        "time": start,
                        "action": "start",
                        "label": label
                    }
                )


                self.events.append(
                    {
                        "time": stop,
                        "action": "stop",
                        "label": label
                    }
                )


        

        self.events.sort(
            key=lambda x:x["time"]
        )


        print(
            f"Loaded {len(self.events)//2} recording periods."
        )


    # -------------------------------------------------

    def show_next_event(self, elapsed):

        """
        next event
        """

        for event in self.events:


            if event["time"] > elapsed:


                remaining = (
                    event["time"]
                    -
                    elapsed
                )


                print(
                    "\rNext:",
                    event["action"].upper(),
                    event["label"],
                    "in",
                    seconds_to_hms(remaining),
                    end=""
                )

                return



    # -------------------------------------------------

    def wait_until(self, target_time, start_time):

        """
        wait until
        """

        while True:


            elapsed = (
                time.time()
                -
                start_time
            )


            remaining = (
                target_time
                -
                elapsed
            )


            if remaining <= 0:

                print()

                break


            self.show_next_event(elapsed)


            time.sleep(0.5)



    # -------------------------------------------------

    def run(self):

        


        if len(self.events) == 0:

            print(
                "No events loaded."
            )

            return



        print()

        input(
            "Press ENTER when experiment starts..."
        )


        experiment_start = time.time()


        log(
            "Experiment started"
        )


        print()


        for event in self.events:


            self.wait_until(
                event["time"],
                experiment_start
            )


            label = event["label"]


            if event["action"] == "start":


                self.obs.start_recording(
                    label
                )


                log(
                    f"START RECORDING : {label}"
                )



            elif event["action"] == "stop":


                self.obs.stop_recording(
                    label
                )


                log(
                    f"STOP RECORDING : {label}"
                )



        log(
            "Experiment finished"
        )


        print()

        print(
            "Experiment finished."
        )