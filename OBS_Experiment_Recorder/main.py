"""
main.py

OBS Experiment Recorder Main Program
"""

from obs_controller import OBSController
from scheduler import ExperimentScheduler
from logger import log, close


def main():


    print("=" * 45)

    print(" OBS Experiment Recorder ")

    print("=" * 45)

    print()



    # -----------------------------
    # Connect OBS
    # -----------------------------

    obs = OBSController()


    if not obs.connect():

        print(
            "Failed to connect OBS."
        )

        return



    log(
        "Connected to OBS"
    )



    # -----------------------------
    # Load Scheduler
    # -----------------------------

    scheduler = ExperimentScheduler(

        obs_controller=obs,

        csv_file="schedule.csv"

    )


    scheduler.load_schedule()



    # -----------------------------
    # Start Experiment
    # -----------------------------

    try:


        scheduler.run()



    except KeyboardInterrupt:


        print()

        print(
            "Experiment interrupted by user."
        )


        log(
            "Experiment interrupted"
        )


        if obs.is_recording():

            obs.stop_recording(
                "Emergency Stop"
            )



    except Exception as e:


        print()

        print(
            "Unexpected error:"
        )

        print(e)


        log(
            f"ERROR: {e}"
        )



    finally:


        close()


        obs.disconnect()



        print()

        print(
            "Program closed."
        )



# ---------------------------------

if __name__ == "__main__":

    main()