/* https://cirosantilli.com/linux-kernel-module-cheat#mmap */

// #include <asm-generic/io.h> /* virt_to_phys */
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h> /* min */
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h> /* copy_from_user, copy_to_user */
#include <linux/slab.h>
/*  */
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/getcpu.h>

#include<linux/string.h>

static const char *filename = "intercept_mmap";

enum { 
    BUFFER_SIZE = 1024,
    PKT_BUFFER_SIZE = 512,
    MAX_PKT = 100
};

struct mmap_info {
	char *data;
};

static unsigned char *buff_from_here;
unsigned int current_index = 0; // where to write pkt
unsigned int last_flush_index = 0;  // last pkt flush position
int status[MAX_PKT] = {0};

unsigned int count_pkt = 0;

/* After unmap. */
static void vm_close(struct vm_area_struct *vma)
{
	pr_info("vm_close\n");
}

/* First page access. */
static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct page *page;
	struct mmap_info *info;

	pr_info("vm_fault\n");
	info = (struct mmap_info *)vmf->vma->vm_private_data;
	if (info->data) {
		page = virt_to_page(info->data);
		get_page(page);
		vmf->page = page;
	}
	return 0;
}

/* After mmap. TODO vs mmap, when can this happen at a different time than mmap? */
static void vm_open(struct vm_area_struct *vma)
{
	pr_info("vm_open\n");
}

static struct vm_operations_struct vm_ops =
{
	.close = vm_close,
	.fault = vm_fault,
	.open = vm_open,
};

static int mmap(struct file *filp, struct vm_area_struct *vma)
{
	pr_info("mmap\n");
	vma->vm_ops = &vm_ops;
	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
	vma->vm_private_data = filp->private_data;
	vm_open(vma);
	return 0;
}

static int open(struct inode *inode, struct file *filp)
{
	struct mmap_info *info;

	pr_info("open\n");
	info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);
	pr_info("virt_to_phys = 0x%llx\n", (unsigned long long)virt_to_phys((void *)info));
	info->data = (char *)get_zeroed_page(GFP_KERNEL);
	memcpy(info->data, "asdf", 4);
	filp->private_data = info;
	return 0;
}

static ssize_t read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	struct mmap_info *info;
	ssize_t ret;

	pr_info("read\n");
	if ((size_t)BUFFER_SIZE <= *off) {
		ret = 0;
	} else {
		info = filp->private_data;
		ret = min(len, (size_t)BUFFER_SIZE - (size_t)*off);
		// if (copy_to_user(buf, info->data + *off, ret)) {
		if (copy_to_user(buf, buff_from_here + *off, ret)) {
			ret = -EFAULT;
		} else {
			*off += ret;
		}
	}
	return ret;
}

static ssize_t write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	struct mmap_info *info;

	pr_info("write\n");
	info = filp->private_data;
	if (copy_from_user(info->data, buf, min(len, (size_t)BUFFER_SIZE))) {
		return -EFAULT;
	} else {
		return len;
	}
}

static int release(struct inode *inode, struct file *filp)
{
	struct mmap_info *info;

	pr_info("release\n");
	info = filp->private_data;
	free_page((unsigned long)info->data);
	kfree(info);
	filp->private_data = NULL;
	return 0;
}

static const struct proc_ops pops = {
	.proc_mmap = mmap,
	.proc_open = open,
	.proc_release = release,
	.proc_read = read,
	.proc_write = write,
};


/* begin net helpers */
/* rxhPacketIn function is called when a packet is received on a registered netdevice */
rx_handler_result_t rxhPacketIn(struct sk_buff **ppkt) {
    
    struct sk_buff* pkt;
    struct iphdr *ip_header;
    // int cpuid;
    // struct tcphdr *tcp_header;
    struct udphdr *udp_udphdr;
    // uint8_t *tcp_headerflags;

    pkt = *ppkt;
    ip_header = (struct iphdr *)skb_network_header(pkt);

    /* Check if IPv4 packet */
    if(ip_header->version != 4){
        // printk(KERN_INFO "[RXH] Got packet that is not IPv4\n");
        return RX_HANDLER_PASS;
    }
    
    /* If not UDP, pass the packet */
    if(ip_header->protocol != 17) {
        // /usr/include/linux/in.h
        return RX_HANDLER_PASS;
    }
    // printk(KERN_INFO "pkt size=%d data_len=%d \n", pkt->len, pkt->data_len);

    /* Report CPU id, source IP, destination IP, and Protocol version */
    // cpuid = raw_smp_processor_id();
    // printk(KERN_INFO "[RXH] CPU id [%i], Source IP [%x], Destination IP [%x], Protocol [%i]\n",
    //     cpuid, ip_header->saddr, ip_header->daddr, ip_header->protocol);
    

    // 14 ethernet
    // 20 ip
    // 8 udp

    /* Parse UDP header */
    udp_udphdr = (struct udphdr *)skb_transport_header(pkt);
    if ((unsigned int)ntohs(udp_udphdr->dest) != 8080) {
        return RX_HANDLER_PASS;
    }
    // __be16	source;
	// __be16	dest;
	// __be16	len;
	// __sum16	check;
    printk(KERN_INFO "UDP sourcePort [%u], destinationPort[%u], len[%u], check[%u], payload[%d]\n",
        (unsigned int)ntohs(udp_udphdr->source) ,(unsigned int)ntohs(udp_udphdr->dest), 
        (unsigned int)ntohs(udp_udphdr->len), (unsigned int)ntohs(udp_udphdr->check), 
        (unsigned int)ntohs(udp_udphdr->len) - 8);

    unsigned char *tt = (unsigned char *) kmalloc(100, GFP_KERNEL);
    memset(tt, '\0', 100);
    memcpy(tt, pkt->data+28,  (unsigned int)ntohs(udp_udphdr->len) - 8);   // 8 bytes udp header
    printk(KERN_INFO "Content of pkt (length=%zu): %s\n", strlen(tt), tt);
    kfree(tt);
    
    /* Write data into the module's cache */
    int try = 0;
    while (current_index<MAX_PKT && current_index!=0) {
        memcpy((buff_from_here+current_index*PKT_BUFFER_SIZE), 
                pkt + (unsigned int)ntohs(udp_udphdr->len), 
                );
        current_index++;
        break;
    }

    // count_pkt += 1;
    // char str[4]; 
    // snprintf(str, 3, "%d", count_pkt);
    // memcpy((buff_from_here), str, 4);


                                    
    /* Parse TCP header */
    // tcp_header = (struct tcphdr *)skb_transport_header(pkt) ;
    // tcp_headerflags = ((uint8_t *)&tcp_header->ack_seq) + 5;
    // printk(KERN_INFO "TCP sourcePort [%u], destinationPort[%u], TCP flags 8 bit [%u], CWR [%u], ECE [%u], URG [%u], ACK [%u], PSH [%u], RST [%u], SYN [%u], FIN [%u]\n",
    //       (unsigned int)ntohs(tcp_header->source) ,(unsigned int)ntohs(tcp_header->dest), *tcp_headerflags,
    //       (uint)tcp_header->cwr, (uint)tcp_header->ece,(uint)tcp_header->urg,(uint)tcp_header->ack,(uint)tcp_header->psh,(uint)tcp_header->rst,
    //       (uint)tcp_header->syn,(uint)tcp_header->fin);



    // int tried = 0;
    // while(tried<MAX_PKT*5) {
    //     if (status[current_index]==0) {

    //     }
    // }

    return RX_HANDLER_PASS;

     /* This was derived from linux source code net/core/net.c .
       Valid return values are RX_HANDLER_CONSUMED, RX_HANDLER_ANOTHER, RX_HANDER_EXACT, RX_HANDLER_PASS.
       If your intention is to handle the packet here in your module code then you should 
       return RX_HANDLER_CONSUMED, in which case you are responsible for release of skbuff
       and should be done via call to kfree_skb(pkt).
    */
}

int registerRxHandlers(void) {
    struct net_device *device;
    int regerr;
    read_lock(&dev_base_lock);
    device = first_net_device(&init_net);
    while (device) {
        printk(KERN_INFO "[RXH] Found [%s] netdevice\n", device->name);
        /* Register only net device with name lo (loopback) */
        if(!strcmp(device->name,"ens33")) {
            rtnl_lock();
            regerr = netdev_rx_handler_register(device,rxhPacketIn,NULL);
            rtnl_unlock();
            if(regerr) {
                printk(KERN_INFO "[RXH] Could not register handler with device [%s], error %i\n", device->name, regerr);
            } else {
                printk(KERN_INFO "[RXH] Handler registered with device [%s]\n", device->name);
            }
        	  device = NULL;
        } else {
            device = next_net_device(device);
	    }
    }
    read_unlock(&dev_base_lock);

    return 0;
}

void unregisterRxHandlers(void) {
    struct net_device *device;
    read_lock(&dev_base_lock);
    device = first_net_device(&init_net);
    while (device) {
        /* Unregister only lo (loopback) */
        if(!strcmp(device->name,"ens33")) {
            rtnl_lock();
            netdev_rx_handler_unregister(device);
            rtnl_unlock();
            printk(KERN_INFO "[RXH] Handler un-registered with device [%s]\n", device->name);
      	    device = NULL;
        } else {
            device = next_net_device(device);
      	}
    }
    read_unlock(&dev_base_lock);
}
/* End net helpers */

static int myinit(void)
{
    buff_from_here = (unsigned char *) kmalloc(PKT_BUFFER_SIZE*MAX_PKT, GFP_KERNEL);
    // bzero(buff_from_here, PKT_BUFFER_SIZE*MAX_PKT);
    memset(buff_from_here, '\0', PKT_BUFFER_SIZE*MAX_PKT);

	proc_create(filename, 0, NULL, &pops);

    /*  */
    int i = 0;
    printk(KERN_INFO "[RXH] Kernel module loaded!\n");
    i=registerRxHandlers();
    /*  */

	return 0;
}

static void myexit(void)
{
    unregisterRxHandlers();
    printk(KERN_INFO "[RXH] Kernel module unloaded.\n");
    /*  */
	remove_proc_entry(filename, NULL);

    kfree(buff_from_here);
}

module_init(myinit)
module_exit(myexit)
MODULE_LICENSE("GPL");
