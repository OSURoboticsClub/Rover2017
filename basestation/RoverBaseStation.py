#!/usr/bin/env python

"""
    Main file used to launch the Rover Base Station
    No other files should be used for launching this application.
"""

__author__ = "Corwin Perren"
__credits__ = [""]
__license__ = "GPL (GNU General Public License) 3.0"
__version__ = "0.1"
__maintainer__ = "Corwin Perren"
__email__ = "caperren@caperren.com"
__status__ = "Development"

#####################################
# Imports
#####################################
# Python native imports
import sys
from PyQt5 import QtWidgets, QtCore, QtGui, uic
import signal
import logging

# Custom Imports
from Framework.SettingsCore import Settings
from Framework.LoggingCore import Logger
from Interface.InterfaceCore import Interface
from Framework.XBOXControllerCore import XBOXController
from Framework.FreeSkyControllerCore import FreeSkyController
from Framework.RoverControllerCore import RoverController

#####################################
# Global Variables
#####################################
UI_FILE_PATH = "Resources/UI/RoverBaseStation.ui"


#####################################
# Application Class Definition
#####################################
class ApplicationWindow(QtWidgets.QMainWindow):
    connect_all_signals_to_slots_signal = QtCore.pyqtSignal()
    start_all_threads = QtCore.pyqtSignal()
    kill_threads_signal = QtCore.pyqtSignal()

    def __init__(self, parent=None):
        # noinspection PyArgumentList
        super(ApplicationWindow, self).__init__(parent)
        uic.loadUi(UI_FILE_PATH, self)

        # ########## Class Variables ##########
        self.num_threads_running = 0
        self.threads = []  # type: [QtCore.QThread]

        # ########## Instantiation of program classes ##########
        # Settings class and version number set
        self.settings_class = Settings(self)
        self.settings = QtCore.QSettings()
        self.settings.setValue("miscellaneous/version", __version__)

        # Uncomment these lines to completely reset settings and quit, then re-comment and rerun program
        # self.settings.clear()
        # self.close()

        # Set up the global logger instance
        self.logger_class = Logger(console_output=True)
        self.logger = logging.getLogger("RoverBaseStation")

        # All interface elements
        self.xbox_controller_class = XBOXController(self)
        self.freesky_controller_class = FreeSkyController(self)
        self.interface_class = Interface(self)
        self.rover_controller_class = RoverController(self)

        # ########## Add threads to list for easy access on program close ##########
        self.threads.append(self.interface_class.live_logs_class)
        self.threads.append(self.xbox_controller_class)
        self.threads.append(self.freesky_controller_class)
        self.threads.append(self.rover_controller_class)

        # ########## Setup signal/slot connections ##########
        for thread in self.threads:
            self.connect_all_signals_to_slots_signal.connect(thread.connect_signals_to_slots__slot)

        self.connect_all_signals_to_slots_signal.emit()

        # ########## Start all child threads ##########
        for thread in self.threads:
            self.start_all_threads.connect(thread.start)

        self.start_all_threads.emit()

        # ########## Ensure all threads started properly ##########
        for thread in self.threads:
            if not thread.isRunning():
                self.close()

        self.logger.info("All threads started successfully!")



    def closeEvent(self, event):
        # Tell all threads to die
        self.kill_threads_signal.emit()

        # Wait for all the threads to end properly
        for thread in self.threads:
            thread.wait()

        # Print final log noting shutdown and shutdown the logger to flush to disk
        self.logger.debug("########## Application Stopping ##########")
        logging.shutdown()

        # Accept the close event to properly close the program
        event.accept()


#####################################
# Main Definition
#####################################
if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal.SIG_DFL)  # This allows the keyboard interrupt kill to work  properly
    application = QtWidgets.QApplication(sys.argv)  # Create the base qt gui application
    app_window = ApplicationWindow()  # Make a window in this application
    app_window.setWindowTitle("Rover Base Station")  # Sets the window title
    app_window.show()  # Show the window in the application
    application.exec_()  # Execute launching of the application
