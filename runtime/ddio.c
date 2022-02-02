// borrow from https://github.com/intel/intel-cmt-cat
#include "ddio.h"

void print_dev_info(struct pci_dev *dev, struct pci_access *pacc)
{
	if(!dev){
		printf("No device found!\n");
		exit(1);
        }
	unsigned int c;
	char namebuf[1024], *name;
	printf("========================\n");
	printf("%04x:%02x:%02x.%d vendor=%04x device=%04x class=%04x irq=%d (pin %d) base0=%lx \n",
                        dev->domain, dev->bus, dev->dev, dev->func, dev->vendor_id, dev->device_id,
                        dev->device_class, dev->irq, c, (long) dev->base_addr[0]);
	name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
	printf(" (%s)\n", name);
	printf("========================\n");
}


struct pci_access * init_pci_access()
{
	struct pci_access *pacc = pci_alloc();           /* Get the pci_access structure */
	pci_init(pacc);               /* Initialize the PCI library */
	pci_scan_bus(pacc);           /* We want to get the list of devices */
    return pacc;
}


struct pci_dev* find_ddio_device(uint8_t nic_bus, struct pci_access *pacc)
{
     
	struct pci_dev* dev;
	for(dev = pacc->devices; dev; dev=dev->next) {
		pci_fill_info(dev,PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_NUMA_NODE | PCI_FILL_PHYS_SLOT);
		/* 
		 * Find the proper PCIe root based on the nic device
		 * For instance, if the NIC is located on 0000:17:00.0 (i.e., BDF)
		 * 0x17 is the nic_bus (B)
		 * 0x00 is the nic_device (D)
		 * 0x0	is the nic_function (F)
		 * TODO: Fix this for Haswell, i.e., 03:00.0 -- Dev and Fun might be different
		 */
		if(/*dev->func == 0 && dev->dev == 0  && */pci_read_byte(dev,PCI_SUBORDINATE_BUS) == nic_bus /*&& dev->numa_node==1 && dev->domain==0x10001*/) {
			return dev;
		}
	}
	printf("Could not find the proper PCIe root!\n");
	return NULL;
}


int ddio_status(uint8_t nic_bus, struct pci_access *pacc)
{
	uint32_t val;
	if(!pacc)
		init_pci_access(pacc);

	struct pci_dev* dev=find_ddio_device(nic_bus, pacc);
	if(!dev){
		printf("No device found!\n");
		exit(1);
	}
	val=pci_read_long(dev,SKX_PERFCTRLSTS_0);
	printf("perfctrlsts_0 val: 0x%" PRIx32 "\n",val);
	printf("NoSnoopOpWrEn val: 0x%" PRIx32 "\n",val&SKX_nosnoopopwren_MASK);
	printf("Use_Allocating_Flow_Wr val: 0x%" PRIx32 "\n",val&SKX_use_allocating_flow_wr_MASK);
	if(val&SKX_use_allocating_flow_wr_MASK)
		return 1;
	else
		return 0;
}


void ddio_enable(uint8_t nic_bus, struct pci_access *pacc)
{
	uint32_t val;
	if(!pacc)
		init_pci_access(pacc);

	if(!ddio_status(nic_bus, pacc))
	{
		struct pci_dev* dev=find_ddio_device(nic_bus, pacc);
		if(!dev){
                	printf("No device found!\n");
                	exit(1);
        	}
		val=pci_read_long(dev,SKX_PERFCTRLSTS_0);
		pci_write_long(dev,SKX_PERFCTRLSTS_0,val|SKX_use_allocating_flow_wr_MASK);
		printf("DDIO is enabled!\n");
	} else
	{
		printf("DDIO was already enabled!\n");
	}
}

void ddio_disable(uint8_t nic_bus, struct pci_access *pacc)
{
	uint32_t val;
	if(!pacc)
		init_pci_access(pacc);

        if(ddio_status(nic_bus, pacc))
        {
		struct pci_dev* dev=find_ddio_device(nic_bus, pacc);
		if(!dev){
                	printf("No device found!\n");
                	exit(1);
		}
                val=pci_read_long(dev,SKX_PERFCTRLSTS_0);
                pci_write_long(dev,SKX_PERFCTRLSTS_0,val&(~SKX_use_allocating_flow_wr_MASK));
		printf("DDIO is disabled!\n");
        } else 
	{
		printf("DDIO was already disabled\n");
	}
}
