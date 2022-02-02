#include <rte_ring.h>
#include <rte_errno.h>
#include "common.h"


struct mailbox create_mailbox(int i, int ring_size) {
    char s[64];
    snprintf(s, sizeof(s), "worker:%d recv ring", i);
    struct mailbox m;
    m.recv_ring = rte_ring_create(s, ring_size, rte_socket_id(), NULL);
    if (m.recv_ring == NULL)
        rte_exit(EXIT_FAILURE, "%s\n", rte_strerror(rte_errno));
    return m;
}

int chain_action(struct switch_table *switchTable, int action_id_1, int action_id_2) {
    switchTable->offset[action_id_1] = action_id_2;
    return 1;
}

void register_action(struct action_registration *actionRegistration, void (*action)(void *, struct message *),
                     char *action_name) {
    strcpy(actionRegistration->action_name, action_name);
    actionRegistration->action = action;
}

//void register_state(struct state_registration *stateRegistration, void *(*init)(void *), char *state_name) {
//    strcpy(stateRegistration->state_name, state_name);
//    stateRegistration->init = init;
//}