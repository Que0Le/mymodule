/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/bpf.h>

#include <bpf/bpf_helpers.h>

#include <stddef.h>
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ipv6.h>
#include <linux/icmpv6.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

#include <string.h>
#include "/home/que/Desktop/mymodule/common.h"
#include <errno.h>

struct bpf_map_def SEC("maps") xsks_map = {
	.type = BPF_MAP_TYPE_XSKMAP,
	.key_size = sizeof(int),
	.value_size = sizeof(int),
	.max_entries = 64,  /* Assume netdev has no more than 64 queues */
};

struct bpf_map_def SEC("maps") uid_timestamps = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(unsigned int),
	.value_size = sizeof(unsigned long),
	.max_entries = 1 + MAX_LOG_ENTRY, 
};

struct bpf_map_def SEC("maps") pkt_payload = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(unsigned int),
	.value_size = sizeof(struct Payload),
	.max_entries = 1 + MAX_LOG_ENTRY, 
};

struct bpf_map_def SEC("maps") xdp_stats_map = {
	.type        = BPF_MAP_TYPE_PERCPU_ARRAY,
	.key_size    = sizeof(int),
	.value_size  = sizeof(__u32),
	.max_entries = 64,
};

SEC("xdp_sock")
int xdp_sock_prog(struct xdp_md *ctx)
{
    unsigned long now = bpf_ktime_get_ns();
    int index = ctx->rx_queue_index;
    __u32 *pkt_count;
    void *data_end = (void *)(long)ctx->data_end;
	void *data = (void *)(long)ctx->data;
    
    if (data_end >= data + 106) {
        if (data_end >= data+42+sizeof(struct Payload)) {
            // bpf_printk("test: %d\n", data_end-data);
            struct Payload *pl;
            pl = data+42;
            unsigned long *time_in_map = bpf_map_lookup_elem(&uid_timestamps, &pl->uid);
            if (time_in_map) {
                // bpf_printk("!! uid before: %lu\n", *value);
                *time_in_map = now;
#ifdef DEBUG_EBPF_INCOMING_PACKETS
                if (pl->uid % 10 == 0) {
                    bpf_printk("!! uid[%lu] timestamp after : %lu\n", pl->uid, *time_in_map);
                    }
#endif
            }

            /* Copying payload data*/
            /* 
            // This is one way to do that ...
            struct Payload *pl_in_map;
            pl_in_map = bpf_map_lookup_elem(&pkt_payload, &pl->uid);
            if (pl_in_map) {
                memcpy(pl_in_map, data+42, sizeof(struct Payload));
                struct Payload *pl_temp = bpf_map_lookup_elem(&pkt_payload, &pl->uid);
                if (pl_temp)
                    bpf_printk("!! uid[%lu] memcpied in map!\n", pl_temp->uid);
            }    
            */
            // The good way is to use update_elem
            int r = bpf_map_update_elem(&pkt_payload, &pl->uid, pl, BPF_ANY);
            if (r==-1) {
                bpf_printk("Failed add to pkt_payload uid[%lu]: ", pl->uid);
            }
#ifdef DEBUG_EBPF_INCOMING_PACKETS
            // Check real value in map. Not important
            struct Payload *pl_temp = bpf_map_lookup_elem(&pkt_payload, &pl->uid);
            if (pl_temp)
                bpf_printk("!! uid[%lu] memcpied in map!\n", pl_temp->uid);
#endif
            /* Counting */
            pkt_count = bpf_map_lookup_elem(&xdp_stats_map, &index);
            if (pkt_count) {
                (*pkt_count)++;
            }
            if (bpf_map_lookup_elem(&xsks_map, &index)){
                return bpf_redirect_map(&xsks_map, index, XDP_ABORTED);
            }
            return XDP_PASS;
        }
        return XDP_PASS;
    } else if ((data + 1000) == data_end) {
        /* A set entry here means that the correspnding queue_id
        * has an active AF_XDP socket bound to it. */
        if (bpf_map_lookup_elem(&xsks_map, &index)){
            // TODO: error handler
            bpf_redirect_map(&xsks_map, index, XDP_PASS);
            return XDP_PASS;
        }
    } else {
        return XDP_PASS;
    }

    return XDP_PASS;
}

/* 
Array Maps

Array maps are implemented in kernel/bpf/arraymap.c . 
All arrays restrict key size to 4 bytes (64 bits), and delete of values is not supported.

BPF_MAP_TYPE_ARRAY: Simple array. Key is the array index, and elements cannot be deleted.

BPF_MAP_TYPE_PERCPU_ARRAY: As above, but kernel programs implicitly write to a per-CPU 
allocated array which minimizes lock contention in BPF program context. 
When bpf_map_lookup_elem() is called, it retrieves NR_CPUS values. 
For example, if we are summing a stat across CPUs, we would do something like this:

long values[nr_cpus];
                        ...

ret = bpf_map_lookup_elem(map_fd, &next_key, values);
if (ret) {
        perror("Error looking up stat");
        continue;
}

for (i = 0; i < nr_cpus; i++) {
        sum += values[i];
}

Use of a per-cpu data structure is to be preferred in codepaths which are frequently executed, 
since we will likely be aggregating the results across CPUs in user-space 
much less frequently than writing updates.
*/

char _license[] SEC("license") = "GPL";
