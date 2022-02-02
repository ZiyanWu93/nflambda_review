//
// Created by anon on 5/2/21.
//

#include "mq.h"

void mq_action(struct mq * p_mq){
//	unsigned lcore_id = rte_lcore_id();
//
//	printf("Starting core %u\n", lcore_id);
//	while (!c->dp_quit){
//		void *msg;
//		if (rte_ring_dequeue(p_mq->recv_ring, &msg) < 0){
//			usleep(5);
//			continue;
//		}
//		printf("core %u: Received '%s'\n", lcore_id, (char *)msg);
//		// rte_mempool_put(message_pool, msg);
//	}
}

void mq_init(struct Config *c){
//    struct rte_ring *send_ring, *recv_ring;
//    unsigned flags = 0;
//    unsigned ring_size = 64;
//    struct mq* p_mq = malloc(sizeof(struct mq));
//    char s[64];
//    // snprintf(s, sizeof(s), "mq %d", rte_lcore_id());
//    // p_mq->send_ring_plist = rte_ring_create(s, ring_size, rte_socket_id(), flags);
//
//    snprintf(s, sizeof(s), "mq %d", rte_lcore_id());
//	p_mq->recv_ring = rte_ring_create(s, ring_size, rte_socket_id(), flags);
//    c->mq_list = p_mq;
}