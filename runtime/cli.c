/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2018 Intel Corporation
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_ethdev.h>
#include <papi.h>

#include "cli.h"

#include "parser.h"
#include "state/state.h"

#ifndef CMD_MAX_TOKENS
#define CMD_MAX_TOKENS 256
#endif


#define MSG_CMD_UNKNOWN "Unknown command \"%s\".\n"

#define MSG_ARG_TOO_MANY "Too many arguments for command \"%s\".\n"


extern struct Config *c;

static int
is_comment(char *in) {
    if ((strlen(in) && index("!#%;", in[0])) ||
        (strncmp(in, "//", 2) == 0) ||
        (strncmp(in, "--", 2) == 0))
        return 1;

    return 0;
}

void cli_process(char *in, char *out, size_t out_size, int fd_clientf) {
    char *tokens[CMD_MAX_TOKENS];
    uint32_t n_tokens = RTE_DIM(tokens);
    int status;
    uint64_t current_time;

    if (is_comment(in))
        return;

    status = parse_tokenize_string(in, tokens, &n_tokens);
    if (status) {
        snprintf(out, out_size, MSG_ARG_TOO_MANY, "");
        return;
    }

    if (n_tokens == 0)
        return;

    if (strcmp(tokens[0], "config") == 0) {
        if (strcmp(tokens[1], "parameter") == 0) {
            c->initStateParameter.rand_range = atoi(tokens[2]);
            c->initStateParameter.data_complexity = atoi(tokens[3]);
            c->initStateParameter.control_complexity = atoi(tokens[4]);
            c->initStateParameter.control_frequency = atoi(tokens[5]);
            c->initStateParameter.execution_id = atoi(tokens[6]);
            c->initStateParameter.state_size = atoi(tokens[7]);
            c->initStateParameter.data_state_size = atoi(tokens[8]);
            c->initStateParameter.control_state_size = atoi(tokens[9]);

            DEBUG_PRINT("config parameter done\n");
            snprintf(out, out_size, "config parameter done!\n");
            return;
        }
        if (strcmp(tokens[1], "rss_reta_config") == 0) {
            int starting_bucket = atoi(tokens[2]);
            int num_bucket = atoi(tokens[3]);
            int worker_id = atoi(tokens[4]);
            int action_id = atoi(tokens[5]);

            uint32_t i;
            uint32_t reta_id;
            uint32_t reta_pos;
            for (i = starting_bucket; i < starting_bucket + num_bucket; i++) {
                reta_id = i / RTE_RETA_GROUP_SIZE;
                reta_pos = i % RTE_RETA_GROUP_SIZE;
                c->reta_conf[reta_id].reta[reta_pos] = c->worker_list[worker_id].queue_id;
            }

            for (i = starting_bucket; i < starting_bucket + num_bucket; i++) {
                c->bucket_to_action_id[i] = action_id;
            }
            DEBUG_PRINT("config rss done %d,%d,%d,%d\n", starting_bucket, num_bucket, worker_id, action_id);
            snprintf(out, out_size, "config rss done!\n");
            return;
        }

        if (strcmp(tokens[1], "rss") == 0) {
            /* RETA update */
            status = rte_eth_dev_rss_reta_update(0,
                                                 c->reta_conf,
                                                 RSS_I_TABLE_SIZE);
            if (status != 0) {
                rte_exit(0, "exit\n");
            }
            DEBUG_PRINT("config rss done\n");
            snprintf(out, out_size, "config rss done!\n");
            return;
        }
    }


    if (strcmp(tokens[0], "state") == 0) {
        if (strcmp(tokens[1], "create") == 0) {
            int index = atoi(tokens[3]);
            state_update(c->stateTable.stateTableEntry[index].state, &(c->initStateParameter));
            DEBUG_PRINT("state create done %d\n", index);
            snprintf(out, out_size, "coordinator send done\n");
            return;
        }
        if (strcmp(tokens[1], "install_path") == 0) {
            int action_index = atoi(tokens[2]);
            int path_index = atoi(tokens[3]);
            int path = atoi(tokens[4]);
            for (int i = 0; i < MAXIMUM_PATH_INSTANCE; i++) {
                if (c->actionTable.actionTableEntry[action_index].path[path_index][i] == -1) {
                    c->actionTable.actionTableEntry[action_index].path[path_index][i] = path;
                    break;
                }
            }
            c->actionTable.actionTableEntry[action_index].path_instance_number[path_index]++;
            DEBUG_PRINT("action install_path done\n");
            snprintf(out, out_size, "action install_path done\n");
            return;
        }
        if (strcmp(tokens[1], "install_simple_path") == 0) {
            int state_id = atoi(tokens[2]);
            int action_id = atoi(tokens[3]);
            int local_path = atoi(tokens[4]);
//            DEBUG_PRINT("action install_path here %d,%d,%d\n", state_id, action_id, local_path);
            c->stateTable.stateTableEntry[state_id].state->simple_path[local_path] = action_id;
            DEBUG_PRINT("action install_path done %d,%d,%d\n", state_id, action_id, local_path);
            snprintf(out, out_size, "action install_path done\n");
            return;
        }

        if (strcmp(tokens[1], "control_frequency") == 0) {
            int state_index = atoi(tokens[2]);
            int control_frequency = atoi(tokens[3]);
            c->stateTable.stateTableEntry[state_index].state->control_frequency = control_frequency;
            DEBUG_PRINT("state control_frequency done\n");
            snprintf(out, out_size, "action control_frequency done\n");
            return;
        }

        if (strcmp(tokens[1], "message_num") == 0) {
            int state_index = atoi(tokens[2]);
            DEBUG_PRINT("state message_num done\n");
            snprintf(out, out_size, "%d\n", c->stateTable.stateTableEntry[state_index].state->n_message);
            return;
        }
        if (strcmp(tokens[1], "data_complexity") == 0) {
            int state_index = atoi(tokens[2]);
            int val = atoi(tokens[3]);
            c->stateTable.stateTableEntry[state_index].state->data_complexity = val;
            DEBUG_PRINT("state data_complexity done\n");
            snprintf(out, out_size, "action data_complexity done\n");
            return;
        }
        if (strcmp(tokens[1], "data_state") == 0) {
            int state_index = atoi(tokens[2]);
            int val = atoi(tokens[3]);
            c->stateTable.stateTableEntry[state_index].state->rand_range = val;
            DEBUG_PRINT("state data_state done\n");
            snprintf(out, out_size, "action data_state done\n");
            return;
        }
    }


    if (strcmp(tokens[0], "action") == 0) {
        if (strcmp(tokens[1], "create") == 0) {
            int registration_id = atoi(tokens[2]);
            int action_index = atoi(tokens[3]);
            int state_index = atoi(tokens[4]);
            int core_id = atoi(tokens[5]);

            c->actionTable.actionTableEntry[action_index].action = c->actionRegistration[registration_id].action;
            c->actionTable.actionTableEntry[action_index].state = c->stateTable.stateTableEntry[state_index].state;
            c->actionTable.actionTableEntry[action_index].core_id = core_id;

            for (int i = 0; i < MAXIMUM_PATH; i++) {
                c->actionTable.actionTableEntry[action_index].path_instance_number[i] = 0;
                for (int j = 0; j < MAXIMUM_PATH_INSTANCE; j++) {
                    c->actionTable.actionTableEntry[action_index].path[i][j] = -1;
                }
            }
//            set_core_state(c->pm, core_id, CORE_READY);
            DEBUG_PRINT("action create done action_name %s action_index %d state_index %d\n",
                        c->actionRegistration[registration_id].action_name, action_index, state_index);
            snprintf(out, out_size, "action create done\n");
            return;
        }

        if (strcmp(tokens[1], "create_packet_out") == 0) {
            int action_index = atoi(tokens[2]);
            int core_id = atoi(tokens[3]);
            c->actionTable.actionTableEntry[action_index].action = transmit_packet;
            c->actionTable.actionTableEntry[action_index].core_id = core_id;
            c->actionTable.actionTableEntry[action_index].state = &(c->worker_list[core_id]);
            c->worker_list[core_id].txb = rte_zmalloc_socket("tx_buffer",
                                                             RTE_ETH_TX_BUFFER_SIZE(1024), 0,
                                                             1);
            rte_eth_tx_buffer_init(c->worker_list[core_id].txb, 1024);
            c->worker_list[core_id].last_action = true;
//            set_core_state(c->pm, core_id, CORE_READY);
            DEBUG_PRINT("action create_packet_out done %d, %d\n", action_index, core_id);
            snprintf(out, out_size, "action create_packet_out done\n");
            return;
        }


        if (strcmp(tokens[1], "install") == 0) {
            void *handle;
            int (*action)(int);
            int test;
            handle = dlopen("/home/anon/CLionProjects/NF_Platform/nf/libtest.so", RTLD_LAZY);
            if (!handle) {
                /* fail to load the library */
                fprintf(stderr, "Error: %s\n", dlerror());
                exit(1);
            }

            *(void **) (&action) = dlsym(handle, "action");
            if (!action) {
                /* no such symbol */
                fprintf(stderr, "Error: %s\n", dlerror());
                dlclose(handle);
            }

            test = action(2);
            dlclose(handle);
            DEBUG_PRINT("action install done: %d\n", test);
            snprintf(out, out_size, "action install done\n");
            return;
        }

        if (strcmp(tokens[1], "migrate") == 0) {
            int action_id = atoi(tokens[2]);
            int core_id = atoi(tokens[3]);
            c->actionTable.actionTableEntry[action_id].core_id = core_id;
            DEBUG_PRINT("action migration done\n");
            snprintf(out, out_size, "action migration done\n");
            return;
        }
    }


    if (strcmp(tokens[0], "coordinator") == 0) {
        if (strcmp(tokens[1], "send") == 0) {
            struct message *m = malloc(sizeof(struct message));
//            m->data = "hello world from the director\n";
//            m->event = COORDINATOR_ACTION_1;
            rte_ring_enqueue(c->worker_list[2].self_mailbox.recv_ring, m);
            DEBUG_PRINT("coordinator send done\n");
            snprintf(out, out_size, "coordinator send done\n");
        }
        return;
    }

    if (strcmp(tokens[0], "port") == 0) {
        if (strcmp(tokens[1], "init") == 0) {
            c->portParameter->n_queue = 24; // allocate 24 queues no matter what
            port_init(c->portParameter);
            DEBUG_PRINT("Port init done: %d queues\n", c->portParameter->n_queue);
            snprintf(out, out_size, "port init done!\n");
        }
        return;
    }
    if (strcmp(tokens[0], "dump") == 0) {
        if (n_tokens == 1) {
            char result[1000];
            int offset = 0;
//            action_id = snprintf(result + action_id, sizeof(result), "dactor %d\n", c->num_dactor);
            offset = snprintf(result + offset, sizeof(result), "nf %d\n", c->nf);
            snprintf(out, out_size, result);
            return;
        } else {
            if (strcmp(tokens[1], "stat") == 0) {
                current_time = rte_get_tsc_cycles();
                char result[1000];
                long total_packets = 0;
                int lcore_id = 0;
                for (int i = 0; i < c->pm->socket_num; i++) {
                    for (int j = 0; j < SOCKET_CORE_NUM; j++) {
                        if (c->pm->socketNodes[i].cores[j].state == CORE_RUNNING) {
                            lcore_id = c->pm->socketNodes[i].cores[j].lcore_id;
                            total_packets += c->worker_list[lcore_id].txs;
                        }
                    }
                }
                snprintf(result, sizeof(result), "%ld", total_packets);
                snprintf(out, out_size, result);
                return;
            }
            if (strcmp(tokens[1], "worker_stat") == 0) {
                int lcore_id = 0;
                char result[1000];
                for (int i = 0; i < c->pm->socket_num; i++) {
                    for (int j = 0; j < SOCKET_CORE_NUM; j++) {
                        if (c->pm->socketNodes[i].cores[j].state == CORE_RUNNING) {
                            lcore_id = c->pm->socketNodes[i].cores[j].lcore_id;
                            lcore_id = lcore_id;
                            DEBUG_PRINT("%ld\n", c->worker_list[lcore_id].txs);
                        }
                    }
                }
                snprintf(result, sizeof(result), "done");
                snprintf(out, out_size, result);
                return;
            }
            if (strcmp(tokens[1], "pm") == 0) {
                char string[1000];
                dump_physical_machine_state_str(c->pm, string);
                snprintf(out, out_size, string);
                return;
            }
            if (strcmp(tokens[1], "rss") == 0) {
                char result[1000];
                struct rte_eth_rss_reta_entry64 reta_conf[c->info.reta_size / RTE_RETA_GROUP_SIZE];
                uint32_t i;
                memset(reta_conf, 0, sizeof(reta_conf));
                for (i = 0; i < c->info.reta_size; i++)
                    reta_conf[i / RTE_RETA_GROUP_SIZE].mask = UINT64_MAX;

                for (i = 0; i < c->info.reta_size; i++) {
                    uint32_t reta_id = i / RTE_RETA_GROUP_SIZE;
                    uint32_t reta_pos = i % RTE_RETA_GROUP_SIZE;
                }
                for (int j = 0; j < c->info.reta_size / RTE_RETA_GROUP_SIZE;
                     j++)
                    reta_conf[j].mask = ~0LL;

                status = rte_eth_dev_rss_reta_query(0,
                                                    reta_conf,
                                                    c->info.reta_size);

                snprintf(result, sizeof(result), "done");
                snprintf(out, out_size, result);
                return;
            }
            if (strcmp(tokens[1], "state") == 0) {
                char string[1000];
                for (int i = 0; i < c->stateTable.num; i++) {
                    printf("%s: %d:%d\n",
                           c->stateRegistration[c->stateTable.stateTableEntry[i].state_registration_id].state_name,
                           c->stateTable.stateTableEntry[i].state->n_message,
                           c->stateTable.stateTableEntry[i].state->hash_table_size);
                }
                DEBUG_PRINT("%d", 10);
                snprintf(out, out_size, "done");
                return;
            }
        }
    }

    // please make sure that the
    if (strcmp(tokens[0], "worker") == 0) {
        /// switch_graph worker_id action_id action_id

        if (strcmp(tokens[1],
                   "first_action") == 0) {
            int worker_id = atoi(tokens[2]);
            int action_id = atoi(tokens[3]);
            int starting_bucket = atoi(tokens[4]);
            int num_bucket = atoi(tokens[5]);
            for (int i = starting_bucket; i < starting_bucket + num_bucket; i++) {
                c->worker_list[worker_id].buckets[i] = action_id;
            }
            c->worker_list[worker_id].first_action = true;
            DEBUG_PRINT("worker first_action done worker_id %d action_id %d starting_bucket %d num_bucket %d\n", worker_id, action_id, starting_bucket, num_bucket);
            snprintf(out, out_size, "worker first_action done\n");
            return;
        }

        if (strcmp(tokens[1], "run") == 0) {
//            for (int i = 0; i < c->pm->socket_num; i++) {
//                for (int j = 0; j < SOCKET_CORE_NUM; j++) {
//                    int lcore_id = c->pm->socketNodes[i].cores[j].lcore_id;
//                    if (lcore_id % 2 == 1)
//                        rte_eal_remote_launch((lcore_function_t *) worker_action, &(c->worker_list[lcore_id]),
//                                              lcore_id);
//                }
//            }
//
            for (int i = 1; i <= 47; i=i+2) {
                rte_eal_remote_launch((lcore_function_t *) worker_action, &(c->worker_list[i]),
                                      i);
            }

            c->running = true;
            DEBUG_PRINT("worker is running\n");
            snprintf(out, out_size, "worker is running\n");
            return;
        }
    }


    if (strcmp(tokens[1], "last_action") == 0) {
        int worker_id_1 = atoi(tokens[2]);
        c->worker_list[worker_id_1].txb = rte_zmalloc_socket("tx_buffer",
                                                             RTE_ETH_TX_BUFFER_SIZE(1024), 0,
                                                             1);
        rte_eth_tx_buffer_init(c->worker_list[worker_id_1].txb, 1024);
        c->worker_list[worker_id_1].last_action = true;
        DEBUG_PRINT("worker last_action done\n");
        snprintf(out, out_size, "worker last_action done\n");
        return;
    }

    if (strcmp(tokens[1], "first_action") == 0) {
        int worker_id_1 = atoi(tokens[2]);
        c->worker_list[worker_id_1].first_action = true;
        DEBUG_PRINT("worker first_action done\n");
        snprintf(out, out_size, "worker first_action done\n");
        return;
    }


    snprintf(out, out_size, MSG_CMD_UNKNOWN);
}