#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>

#include <rte_common.h>
#include <rte_launch.h>
#include <rte_timer.h>

#include "Config.h"
#include "cli.h"
#include "conn.h"

struct Config *c;

static const char welcome[] = "hello";

static const char prompt[] = "";

static struct app_params {
    struct conn_params conn;
    char *script_name;
} app = {
        .conn = {
                .welcome = welcome,
                .prompt = prompt,
                .addr = "0.0.0.0",
                .port = 8093,
                .buf_size = 1024 * 1024,
                .msg_in_len_max = 1024,
                .msg_out_len_max = 1024 * 1024,
                .msg_handle = cli_process,
        },
        .script_name = NULL,
};

static void
signal_handler(int signum) {
    unsigned lcore_id;
    c->dp_quit = true;
    c->cp_quit = true;
}


int main(int argc, char *argv[]) {
    struct conn *conn;
    int status;
    init_EAL("/home/anon/CLionProjects/NF_Platform/run.txt");
    config_init("/home/anon/project/NF_Platform/config.yaml");

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Connectivity */
    conn = conn_init(&app.conn);
    if (conn == NULL) {
        printf("Error: Connectivity initialization failed (%d)\n",
               status);
        return status;
    }

    while (!(c->cp_quit)) {
        output_log(c);
        conn_poll_for_conn(conn);
        conn_poll_for_msg(conn);
    }

    close(conn->fd_server);
    rte_eal_mp_wait_lcore();
    return 1;
}
