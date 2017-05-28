"""
    This file contains the live logs page sub-class
"""

#####################################
# Imports
#####################################
# Python native imports
from PyQt5 import QtCore, QtWidgets
import logging
from Framework.MiniBoardIOCore import write_drive_motor_power, read_drive_motor_power, write_pause, \
    read_pan_tilt, write_pan_tilt, write_arm_motors

#####################################
# Global Variables
#####################################
DRIVE_MIN = -127
DRIVE_MAX = 127

ARMS_MIN = -127
ARMS_MAX = 127

DEAD_BAND_FRSKY = 20
DEAD_BAND_XBOX = 1500


#####################################
# Notes
#####################################
##### Xbox State Names
# "left_stick_x_axis"
# "left_stick_y_axis"
# "left_stick_center_pressed"
#
# "right_stick_x_axis"
# "right_stick_y_axis"
# "right_stick_center_pressed"
#
# "left_trigger_z_axis"
# "left_bumper_pressed"
#
# "right_trigger_z_axis"
# "right_bumper_pressed"
#
# "dpad_x"
# "dpad_y"
#
# "select_pressed"
# "start_pressed"
# "home_pressed"
#
# "a_pressed"
# "b_pressed"
# "x_pressed"
# "y_pressed"

##### Frsky State Names
# "left_stick_x_axis"
# "left_stick_y_axis"
#
# "right_stick_x_axis"
# "right_stick_y_axis"
#
# "sc_state"
# "sg_state"
# "se_state"
# "sa_state"
#
# "ls_axis"
# "rs_axis"
#
# "s1_axis"


#####################################
# Controller Class Definition
#####################################
class MotionProcessor(QtCore.QThread):
    send_miniboard_control_packet = QtCore.pyqtSignal(list)

    def __init__(self, main_window):
        super(MotionProcessor, self).__init__()

        # ########## Reference to top level window ##########
        self.main_window = main_window  # type: QtWidgets.QMainWindow

        # ########## Get the settings instance ##########
        self.settings = QtCore.QSettings()

        # ########## Get the Pick And Plate instance of the logger ##########
        self.logger = logging.getLogger("RoverBaseStation")

        # ########## Thread Flags ##########
        self.run_thread_flag = True
        self.wait_for_drive_response = False
        self.wait_for_primary_pan_tilt_response = False
        self.wait_for_arm_response = False

        # ########## Class Variables ##########
        self.xbox_states = {}
        self.frsky_states = {}

        self.last_pause_state = 0
        self.last_drive_state = 0

        self.last_left_drive_value = 0
        self.last_right_drive_value = 0

        self.pan_position = -1
        self.tilt_position = -1

    def run(self):
        self.logger.debug("Motion Processor Thread Starting...")

        # TODO: Switching out of drive state stops motion
        read_pan_tilt(self.send_miniboard_control_packet)

        # while self.pan_position == -1 or self.tilt_position == -1 or self.run_thread_flag:
        #     self.msleep(1)

        while self.run_thread_flag:
            if self.xbox_states and self.frsky_states:
                # self.logger.debug(self.frsky_states["se_state"])
                self.__set_pause_on_state_change()
                self.__all_stop_on_arm_control()
                if not self.frsky_states["sa_state"]:  # 0 is drive mode
                    if not self.frsky_states["se_state"]:  # 0 is drive mode
                        self.__drive_manual()
                        # self.__pan_tilt_manual()

                    else:  # 1 is arm mode
                        # self.__arm_control_manual()
                        pass
                else:  # 1 is auto mode
                    self.__drive_auto()

        self.logger.debug("Motion Processor Thread Stopping...")

    # noinspection PyUnresolvedReferences
    def connect_signals_to_slots__slot(self):
        self.main_window.xbox_controller_class.controller_update_ready_signal.connect(self.on_xbox_states_updated__slot)
        self.main_window.frsky_controller_class.controller_update_ready_signal.connect(
            self.on_frsky_states_updated__slot)

        self.send_miniboard_control_packet.connect(self.main_window.miniboard_class.append)

        self.main_window.miniboard_class.data_drive_motor_power.connect(
            self.on_drive_motor_power_response_received__slot)
        self.main_window.miniboard_class.ack_drive_motor_power.connect(self.on_drive_response_received__slot)

        self.main_window.miniboard_class.data_pan_tilt.connect(self.on_pan_tilt_primary_position_response__slot)
        # self.main_window.miniboard_class.data_pan_tilt_secondary.connect(
        #     self.on_pan_tilt_secondary_position_response__slot)

        self.main_window.miniboard_class.ack_pan_tilt.connect(self.on_primary_pan_tilt_write_acknowledged__slot)

        self.main_window.miniboard_class.ack_arm_motors.connect(self.on_arm_motors_write_acknowledged__slot)

        self.main_window.kill_threads_signal.connect(self.on_kill_threads__slot)

    def __set_pause_on_state_change(self):
        current_state = self.frsky_states['sf_state']

        if current_state != self.last_pause_state:
            write_pause(self.send_miniboard_control_packet, current_state)
            self.last_pause_state = current_state

    def __all_stop_on_arm_control(self):

        current_state = self.frsky_states["sa_state"]

        if (current_state == 1) and (self.last_drive_state == 0):
            self.wait_for_drive_response = True
            write_drive_motor_power(self.send_miniboard_control_packet, 0, 0, 0, 0,
                                    0, 0)

            while self.wait_for_drive_response:
                self.msleep(1)

            while self.frsky_states["left_stick_y_axis"] > DEAD_BAND_FRSKY and self.frsky_states[
                "right_stick_y_axis"] > DEAD_BAND_FRSKY:
                self.msleep(1)

            self.last_drive_state = current_state
        elif (current_state == 0) and (self.last_drive_state == 1):
            self.last_drive_state = 0

    def __arm_control_manual(self):
        OFFSET = 127

        max_speed_scale_raw = self.frsky_states["s1_axis"] + OFFSET
        base_speed_raw = self.frysky_states["left_stick_x_axis"]
        bicep_speed_raw = self.frysky_states["left_stick_y_axis"]
        forearm_speed_raw = self.frysky_states["right_stick_y_axis"]
        pitch_speed_raw = self.frysky_states["rs_axis"]
        wrist_rotation_speed_raw = self.frysky_states["right_stick_x_axis"]
        arm_gripper_speed_raw = self.frysky_states["ls_axis"]

        scale_percentage = max_speed_scale_raw / 255

        base_speed_scaled = base_speed_raw * max_speed_scale_raw
        bicep_speed_scaled = bicep_speed_raw * max_speed_scale_raw
        forearm_speed_scaled = forearm_speed_raw * max_speed_scale_raw
        pitch_speed_scaled = pitch_speed_raw * max_speed_scale_raw
        wrist_rotation_speed_scaled = wrist_rotation_speed_raw * max_speed_scale_raw
        arm_gripper_speed_scaled = arm_gripper_speed_raw * max_speed_scale_raw

        base_speed_scaled = int(self.clamp(base_speed_scaled, ARMS_MIN, ARMS_MAX))
        bicep_speed_scaled = int(self.clamp(bicep_speed_scaled, ARMS_MIN, ARMS_MAX))
        forearm_speed_scaled = int(self.clamp(forearm_speed_scaled, ARMS_MIN, ARMS_MAX))
        pitch_speed_scaled = int(self.clamp(pitch_speed_scaled, ARMS_MIN, ARMS_MAX))
        wrist_rotation_speed_scaled = int(self.clamp(wrist_rotation_speed_scaled, ARMS_MIN, ARMS_MAX))
        arm_gripper_speed_scaled = int(self.clamp(arm_gripper_speed_scaled, ARMS_MIN, ARMS_MAX))

        self.wait_for_arm_response = True

        write_arm_motors(self.send_miniboard_control_packet,
                         base_speed_scaled,
                         bicep_speed_scaled,
                         forearm_speed_scaled,
                         pitch_speed_scaled,
                         wrist_rotation_speed_scaled,
                         arm_gripper_speed_scaled)

        while self.wait_for_arm_response:
            self.msleep(1)

        # write_arm_motors(self.send_miniboard_control_packet, )

    def __drive_manual(self):
        OFFSET = 127

        max_speed_scale_raw = self.frsky_states["s1_axis"] + OFFSET  # Range (min to max) 0 to 255
        left_stick_y_axis_raw = self.frsky_states["left_stick_y_axis"]  # Range (min to max) -127 to 127
        right_stick_y_axis_raw = self.frsky_states["right_stick_y_axis"]  # Range (min to max) -127 to 127

        scale_percentage = max_speed_scale_raw / 255

        left_scaled = left_stick_y_axis_raw * scale_percentage
        right_scaled = right_stick_y_axis_raw * scale_percentage

        left_scaled = int(self.clamp(left_scaled, DRIVE_MIN, DRIVE_MAX))
        right_scaled = int(self.clamp(right_scaled, DRIVE_MIN, DRIVE_MAX))

        if left_scaled != self.last_left_drive_value or right_scaled != self.last_right_drive_value:
            self.wait_for_drive_response = True
            write_drive_motor_power(self.send_miniboard_control_packet, left_scaled, left_scaled, left_scaled, right_scaled,
                                    right_scaled, right_scaled)

            while self.wait_for_drive_response:
                self.logger.debug("Waitng for response...")
                self.msleep(1)

            self.last_left_drive_value = left_scaled
            self.last_right_drive_value = right_scaled
            self.logger.debug(left_scaled)
            self.logger.debug(right_scaled)

            # read_drive_motor_power(self.send_miniboard_control_packet)

    def __drive_auto(self):
        pass



    def __pan_tilt_manual(self):
        # X axis (min to max) -32768 to 32768
        # Y axis (min to max) 32768 to 32768

        # TODO: move to own function. Should work when in any mode but pause

        controller_pan_raw = self.xbox_states["right_stick_x_axis"]
        controller_tilt_raw = self.xbox_states["right_stick_y_axis"]

        if abs(controller_pan_raw) < DEAD_BAND_XBOX:
            controller_pan_raw = 0

        if abs(controller_tilt_raw) < DEAD_BAND_XBOX:
            controller_tilt_raw = 0

        controller_pan = -controller_pan_raw/2560
        controller_tilt = -controller_tilt_raw/2560

        new_pan = self.clamp(int(self.pan_position+controller_pan), 0, 65535)
        new_tilt = self.clamp(int(self.tilt_position+controller_tilt), 0, 65535)

        self.wait_for_primary_pan_tilt_response = True
        write_pan_tilt(self.send_miniboard_control_packet, new_pan, new_tilt)

        while self.wait_for_primary_pan_tilt_response:
            self.msleep(1)

        self.pan_position = new_pan
        self.tilt_position = new_tilt

    def on_xbox_states_updated__slot(self, states):
        self.xbox_states = states

    def on_frsky_states_updated__slot(self, states):
        self.frsky_states = states

    def on_primary_pan_tilt_write_acknowledged__slot(self):
        self.wait_for_primary_pan_tilt_response = False

    def on_secondary_pan_tilt_write_acknowledged__slot(self):
        self.wait_for_primary_pan_tilt_response  = False #TODO: make secondary

    def on_pan_tilt_primary_position_response__slot(self, sdict):
        try:
            self.pan_position = sdict["pan"]
            self.tilt_position = sdict["tilt"]
        except:
            pass

    def on_pan_tilt_secondary_position_response__slot(self, sdict):
        pass
        #self.logger.debug(sdict)

    def on_drive_motor_power_response_received__slot(self, sdict):
        pass
        #self.logger.debug(sdict)

    def on_drive_response_received__slot(self):
        self.wait_for_drive_response = False

    def on_arm_motors_write_acknowledged__slot(self):
        self.wait_for_arm_response = False

    def on_kill_threads__slot(self):
        self.run_thread_flag = False

    @staticmethod


    @staticmethod
    def clamp(n, minn, maxn):
        return max(min(maxn, n), minn)
