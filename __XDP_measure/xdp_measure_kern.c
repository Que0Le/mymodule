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

/* header parse */
// The parsing helper functions from the packet01 lesson have moved here
#include "../common/parsing_helpers.h"

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
    
    /* parsing. From packet-solutions/xdp_prog_kern_02.c */
    int eth_type, ip_type;
	struct ethhdr *eth;
	struct iphdr *iphdr;
	struct udphdr *udphdr;
	struct hdr_cursor nh = { .pos = data };

	eth_type = parse_ethhdr(&nh, data_end, &eth);
	if (eth_type != bpf_htons(ETH_P_IP)) {
	    return XDP_PASS;
	}
    ip_type = parse_iphdr(&nh, data_end, &iphdr);
	if (ip_type != IPPROTO_UDP) {
        return XDP_PASS;
	}
    if (parse_udphdr(&nh, data_end, &udphdr) != sizeof(struct Payload)) {
        return XDP_PASS;
    }
    // bpf_printk("test: %d\n", data_end-data);
    // bpf_printk("test: %d, port: %u, len: %u\n", data_end-data, udphdr->dest, udphdr->len);
    // udphdr->dest = bpf_htons(bpf_ntohs(udphdr->dest) - 1);
    if (udphdr->dest != bpf_htons(DEST_PORT)) {
        return XDP_PASS;
    }
    /* End parsing */
    
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
        }
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
