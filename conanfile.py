from conans import ConanFile, AutoToolsBuildEnvironment
from conans.tools import Git, SystemPackageTool
import os
import shutil


class Cornhole(ConanFile):
    name = "Cornhole"
    version = "0.1"

    settings = "compiler", "arch"

    avrcore = Git(folder="ArduinoCore-avr")

    def system_requirements(self):
        installer = SystemPackageTool()
        installer.install("arduino")

    def requirements(self):
        self.requires("aurdino-builder/1.44@arduino-builder/conanify")

    def source(self):
        self.avrcore.clone("https://github.com/arduino/ArduinoCore-avr.git")

    def build(self):
        self.run("arduino-builder -compile -logger=machine "
            "-hardware /usr/share/arduino/hardware " +
            "-tools /usr/share/arduino/tools " +
            "-built-in-libraries /usr/share/arduino/libraries " +
            "-libraries /home/jander10/Arduino/libraries " +
            "-fqbn=arduino:avr:nano:cpu=atmega328old " +
            "-ide-version=10809 " +
            "-build-path " + self.build_folder +
            " -warnings=none " +
            "-prefs=build.warn_data_percentage=75 " +
            "-prefs=runtime.tools.avrdude.path=/home/jander10/bin/arduino/hardware/tools/avr " +
            "-prefs=runtime.tools.avrdude-6.3.0-arduino14.path=/home/jander10/bin/arduino/hardware/tools/avr " +
            "-prefs=runtime.tools.avr-gcc.path=/home/jander10/bin/arduino/hardware/tools/avr " +
            "-prefs=runtime.tools.avr-gcc-5.4.0-atmel3.6.1-arduino2.path=/home/jander10/bin/arduino/hardware/tools/avr " +
            "-prefs=runtime.tools.arduinoOTA.path=/home/jander10/bin/arduino/hardware/tools/avr " +
            "-prefs=runtime.tools.arduinoOTA-1.2.1.path=/home/jander10/bin/arduino/hardware/tools/avr " +
            "-verbose src/cornhole/cornhole.ino")