# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/Janis/esp/esp-idf/components/bootloader/subproject"
  "C:/dev/Arduino-Collection/turntable/ESP-IDF/zigbee_test/HA_on_off_light/build/bootloader"
  "C:/dev/Arduino-Collection/turntable/ESP-IDF/zigbee_test/HA_on_off_light/build/bootloader-prefix"
  "C:/dev/Arduino-Collection/turntable/ESP-IDF/zigbee_test/HA_on_off_light/build/bootloader-prefix/tmp"
  "C:/dev/Arduino-Collection/turntable/ESP-IDF/zigbee_test/HA_on_off_light/build/bootloader-prefix/src/bootloader-stamp"
  "C:/dev/Arduino-Collection/turntable/ESP-IDF/zigbee_test/HA_on_off_light/build/bootloader-prefix/src"
  "C:/dev/Arduino-Collection/turntable/ESP-IDF/zigbee_test/HA_on_off_light/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/dev/Arduino-Collection/turntable/ESP-IDF/zigbee_test/HA_on_off_light/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/dev/Arduino-Collection/turntable/ESP-IDF/zigbee_test/HA_on_off_light/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
