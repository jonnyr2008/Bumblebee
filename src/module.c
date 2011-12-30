/*
 * Copyright (C) 2011 Bumblebee Project
 * Author: Joaquín Ignacio Aramendía <samsagax@gmail.com>
 * Author: Jaron Viëtor AKA "Thulinma" <jaron@vietors.com>
 *
 * This file is part of Bumblebee.
 *
 * Bumblebee is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bumblebee is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bumblebee. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "module.h"
#include "bblogger.h"
#include "bbrun.h"

/**
 * Checks in /proc/modules whether a kernel module is loaded
 *
 * @param module_name The name of the kernel module
 * @return 1 if the module is loaded, 0 otherwise
 */
int module_is_loaded(char *module_name) {
  // use the same buffer length as lsmod
  char buffer[4096];
  FILE * bbs = fopen("/proc/modules", "r");
  int ret = 0;
  /* assume mod_len <= sizeof(buffer) */
  int mod_len = strlen(module_name);

  if (bbs == 0) {//error opening, return -1
    bb_log(LOG_DEBUG, "Couldn't open /proc/modules");
    return -1;
  }
  while (fgets(buffer, sizeof(buffer), bbs)) {
    /* match "module" with "module " and not "module-blah" */
    if (!strncmp(buffer, module_name, mod_len) && isspace(buffer[mod_len])) {
      /* module is found */
      ret = 1;
      break;
    }
  }
  fclose(bbs);
  return ret;
}


/**
 * Attempts to unload a module if loaded, for ten seconds before
 * giving up
 *
 * @param module_name The name of the kernel module to be unloaded
 * @return 1 if the driver is succesfully unloaded, 0 otherwise
 */
int module_unload(char *module_name) {
  if (module_is_loaded(module_name) == 1) {
    bb_log(LOG_INFO, "Unloading %s module\n", module_name);
    char *mod_argv[] = {
      "rmmod",
      "--wait",
      module_name,
      NULL
    };
    bb_run_fork_wait(mod_argv, 10);
    if (module_is_loaded(module_name) == 1) {
      bb_log(LOG_ERR, "Unloading %s module timed out.\n", module_name);
      return 0;
    }
  }
  return 1;
}