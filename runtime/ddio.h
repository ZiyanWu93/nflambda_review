
// borrow from https://github.com/intel/intel-cmt-cat, https://github.com/aliireza/ddio-bench
#ifndef MULTICORE_FWDING_DPDK_DDIO_H
#define MULTICORE_FWDING_DPDK_DDIO_H
#include <stdio.h>
#include <stdlib.h>
#include <pci/pci.h>
#include <sys/io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>

#define SKX_PERFCTRLSTS_0	0x180
#define SKX_use_allocating_flow_wr_MASK 0x80
#define SKX_nosnoopopwren_MASK	0x8


struct pci_access * init_pci_access();

void print_dev_info(struct pci_dev *dev, struct pci_access *pacc);

struct pci_dev* find_ddio_device(uint8_t nic_bus, struct pci_access *pacc);

int ddio_status(uint8_t nic_bus, struct pci_access *pacc);

void ddio_enable(uint8_t nic_bus, struct pci_access *pacc);

void ddio_disable(uint8_t nic_bus, struct pci_access *pacc);

#endif //MULTICORE_FWDING_DPDK_DDIO_H
