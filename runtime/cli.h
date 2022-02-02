/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2018 Intel Corporation
 */

#ifndef CLI
#define CLI

#include <stddef.h>
#include <unistd.h>
#include "Config.h"
#include "nf/nm.h"
#include "nf/traffic_generator.h"
void cli_process(char *in, char *out, size_t out_size, int fd_client);

int cli_script_process(const char *file_name,
					   size_t msg_in_len_max,
					   size_t msg_out_len_max);

#endif /* CLI */
