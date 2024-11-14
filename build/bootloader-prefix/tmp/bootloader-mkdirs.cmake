# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "F:/esp32/v5.3.1/esp-idf/components/bootloader/subproject"
  "F:/esp32/v5.3.1/Projects/sample_project/build/bootloader"
  "F:/esp32/v5.3.1/Projects/sample_project/build/bootloader-prefix"
  "F:/esp32/v5.3.1/Projects/sample_project/build/bootloader-prefix/tmp"
  "F:/esp32/v5.3.1/Projects/sample_project/build/bootloader-prefix/src/bootloader-stamp"
  "F:/esp32/v5.3.1/Projects/sample_project/build/bootloader-prefix/src"
  "F:/esp32/v5.3.1/Projects/sample_project/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "F:/esp32/v5.3.1/Projects/sample_project/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "F:/esp32/v5.3.1/Projects/sample_project/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
