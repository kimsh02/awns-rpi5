# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

if(EXISTS "/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde-stamp/concorde-gitclone-lastrun.txt" AND EXISTS "/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde-stamp/concorde-gitinfo.txt" AND
  "/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde-stamp/concorde-gitclone-lastrun.txt" IS_NEWER_THAN "/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde-stamp/concorde-gitinfo.txt")
  message(VERBOSE
    "Avoiding repeated git clone, stamp file is up to date: "
    "'/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde-stamp/concorde-gitclone-lastrun.txt'"
  )
  return()
endif()

# Even at VERBOSE level, we don't want to see the commands executed, but
# enabling them to be shown for DEBUG may be useful to help diagnose problems.
cmake_language(GET_MESSAGE_LOG_LEVEL active_log_level)
if(active_log_level MATCHES "DEBUG|TRACE")
  set(maybe_show_command COMMAND_ECHO STDOUT)
else()
  set(maybe_show_command "")
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde"
  RESULT_VARIABLE error_code
  ${maybe_show_command}
)
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/bin/git"
            clone --no-checkout --config "advice.detachedHead=false" "https://github.com/matthelb/concorde.git" "concorde"
    WORKING_DIRECTORY "/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src"
    RESULT_VARIABLE error_code
    ${maybe_show_command}
  )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(NOTICE "Had to git clone more than once: ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/matthelb/concorde.git'")
endif()

execute_process(
  COMMAND "/usr/bin/git"
          checkout "master" --
  WORKING_DIRECTORY "/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde"
  RESULT_VARIABLE error_code
  ${maybe_show_command}
)
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'master'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "/usr/bin/git" 
            submodule update --recursive --init 
    WORKING_DIRECTORY "/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde"
    RESULT_VARIABLE error_code
    ${maybe_show_command}
  )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy "/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde-stamp/concorde-gitinfo.txt" "/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde-stamp/concorde-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  ${maybe_show_command}
)
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/Users/kimsh/Downloads/jobs/last-semester/awns-rpi5/app/build/concorde/src/concorde-stamp/concorde-gitclone-lastrun.txt'")
endif()
