/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/bpf.h>

#include <bpf/bpf_helpers.h>

struct bpf_map_def SEC("maps") xsks_map = {
	.type = BPF_MAP_TYPE_XSKMAP,
	.key_size = sizeof(int),
	.value_size = sizeof(int),
	.max_entries = 64,  /* Assume netdev has no more than 64 queues */
};

struct bpf_map_def SEC("maps") uid_timestamps = {
	.type = BPF_MAP_TYPE_XSKMAP,
	.key_size = sizeof(unsigned long),
	.value_size = sizeof(unsigned long),
	.max_entries = 2 << 18, 
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
    int index = ctx->rx_queue_index;
    __u32 *pkt_count;

    pkt_count = bpf_map_lookup_elem(&xdp_stats_map, &index);
    if (pkt_count) {

        /* We pass every other packet */
        if ((*pkt_count)++ & 1)
            return XDP_PASS;
    }

    /* A set entry here means that the correspnding queue_id
     * has an active AF_XDP socket bound to it. */
    if (bpf_map_lookup_elem(&xsks_map, &index))
        return bpf_redirect_map(&xsks_map, index, 0);

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
