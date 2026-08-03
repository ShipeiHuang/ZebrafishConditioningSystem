"""
obs_controller.py

Communicate with OBS
"""

from obsws_python import ReqClient
from config import HOST, PORT, PASSWORD


class OBSController:

    def __init__(self):

        self.client = None

        self.connected = False

        self.recording = False

    # -----------------------------------

    def connect(self):

        try:

            self.client = ReqClient(
                host=HOST,
                port=PORT,
                password=PASSWORD
            )

            self.connected = True

            print("Connected to OBS.")

            return True

        except Exception as e:

            print("Cannot connect to OBS.")

            print(e)

            return False

    # -----------------------------------

    def start_recording(self, label=""):

        if not self.connected:

            print("OBS not connected.")

            return

        if self.recording:

            print("Already recording.")

            return

        try:

            self.client.start_record()

            self.recording = True

            if label != "":

                print(f"START RECORDING : {label}")

            else:

                print("START RECORDING")

        except Exception as e:

            print(e)

    # -----------------------------------

    def stop_recording(self, label=""):

        if not self.connected:

            print("OBS not connected.")

            return

        if not self.recording:

            print("Already stopped.")

            return

        try:

            self.client.stop_record()

            self.recording = False

            if label != "":

                print(f"STOP RECORDING : {label}")

            else:

                print("STOP RECORDING")

        except Exception as e:

            print(e)

    # -----------------------------------

    def is_connected(self):

        return self.connected

    # -----------------------------------

    def is_recording(self):

        return self.recording

    # -----------------------------------

    def disconnect(self):

        try:

            if self.client:

                self.client.disconnect()

        except:

            pass

        self.connected = False