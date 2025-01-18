# SmallBASIC for teensy mcu
# Copyright(C) 2025 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

set(COMMON_DIR ${CMAKE_SOURCE_DIR}/../../common)

set(SOURCES
  ${COMMON_DIR}/../lib/match.c
  ${COMMON_DIR}/../lib/str.c
  ${COMMON_DIR}/../lib/matrix.c
  ${COMMON_DIR}/bc.c
  ${COMMON_DIR}/blib.c
  ${COMMON_DIR}/blib_func.c
  ${COMMON_DIR}/blib_math.c
  ${COMMON_DIR}/brun.c
  ${COMMON_DIR}/ceval.c
  ${COMMON_DIR}/random.c
  ${COMMON_DIR}/eval.c
  ${COMMON_DIR}/plugins.c
  ${COMMON_DIR}/fmt.c
  ${COMMON_DIR}/kw.c
  ${COMMON_DIR}/proc.c
  ${COMMON_DIR}/sberr.c
  ${COMMON_DIR}/scan.c
  ${COMMON_DIR}/str.c
  ${COMMON_DIR}/tasks.c
  ${COMMON_DIR}/hashmap.c
  ${COMMON_DIR}/var_map.c
  ${COMMON_DIR}/var_eval.c
  ${COMMON_DIR}/keymap.c
  ${COMMON_DIR}/units.c
  ${COMMON_DIR}/var.c
)

add_library(common STATIC ${SOURCES})

target_include_directories(common PRIVATE ${COMMON_DIR})
target_include_directories(common PRIVATE ${CMAKE_SOURCE_DIR}/../../..)
target_include_directories(common PRIVATE ${CMAKE_SOURCE_DIR}/../..)
