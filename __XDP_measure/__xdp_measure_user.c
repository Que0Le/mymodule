/* SPDX-License-Identifier: GPL-2.0 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <locale.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/resource.h>

#include <bpf/bpf.h>
#include <bpf/xsk.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_link.h>
#include <linux/if_ether.h>
#include <linux/ipv6.h>
#include <linux/icmpv6.h>


#include "../common/common_params.h"
#include "../common/common_user_bpf_xdp.h"
#include "../common/common_libbpf.h"

#include "/home/que/Desktop/mymodule/common.h"
int time_xsks_map_fd;
unsigned long *log_time_stamps;

// #define NUM_FRAMES         4096
// #define FRAME_SIZE         XSK_UMEM__DEFAULT_FRAME_SIZE
// #define RX_BATCH_SIZE      64
// #define INVALID_UMEM_FRAME UINT64_MAX

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

// struct xsk_umem_info {
// 	struct xsk_ring_prod fq;
// 	struct xsk_ring_cons cq;
// 	struct xsk_umem *umem;
// 	void *buffer;
// };

struct stats_record {
	uint64_t timestamp;
	uint64_t rx_packets;
	uint64_t rx_bytes;
	uint64_t tx_packets;
	uint64_t tx_bytes;
};

// struct xsk_socket_info {
// 	struct xsk_ring_cons rx;
// 	struct xsk_ring_prod tx;
// 	struct xsk_umem_info *umem;
// 	struct xsk_socket *xsk;

// 	uint64_t umem_frame_addr[NUM_FRAMES];
// 	uint32_t umem_frame_free;

// 	uint32_t outstanding_tx;

// 	struct stats_record stats;
// 	struct stats_record prev_stats;
// };

// static inline __u32 xsk_ring_prod__free(struct xsk_ring_prod *r)
// {
// 	r->cached_cons = *r->consumer + r->size;
// 	return r->cached_cons - r->cached_prod;
}

#define NANOSEC_PER_SEC 1000000000 /* 10^9 */
static uint64_t gettime(void)
{
	struct timespec t;
	int res;

	res = clock_gettime(CLOCK_MONOTONIC, &t);
	if (res < 0) {
		fprintf(stderr, "Error with gettimeofday! (%i)\n", res);
		exit(EXIT_FAIL);
	}
	return (uint64_t) t.tv_sec * NANOSEC_PER_SEC + t.tv_nsec;
}

static const char *__doc__ = "XDP measure, based on XDP_tutorial's AF_XDP kernel bypass example\n";

static const struct option_wrapper long_options[] = {

	{{"help",	 no_argument,		NULL, 'h' },
	 "Show help", false},

	{{"dev",	 required_argument,	NULL, 'd' },
	 "Operate on device <ifname>", "<ifname>", true},

	{{"skb-mode",	 no_argument,		NULL, 'S' },
	 "Install XDP program in SKB (AKA generic) mode"},

	{{"native-mode", no_argument,		NULL, 'N' },
	 "Install XDP program in native mode"},

	{{"auto-mode",	 no_argument,		NULL, 'A' },
	 "Auto-detect SKB or native mode"},

	{{"force",	 no_argument,		NULL, 'F' },
	 "Force install, replacing existing program on interface"},

	{{"copy",        no_argument,		NULL, 'c' },
	 "Force copy mode"},

	{{"zero-copy",	 no_argument,		NULL, 'z' },
	 "Force zero-copy mode"},

	{{"queue",	 required_argument,	NULL, 'Q' },
	 "Configure interface receive queue for AF_XDP, default=0"},

	{{"poll-mode",	 no_argument,		NULL, 'p' },
	 "Use the poll() API waiting for packets to arrive"},

	{{"unload",      no_argument,		NULL, 'U' },
	 "Unload XDP program instead of loading"},

	{{"quiet",	 no_argument,		NULL, 'q' },
	 "Quiet mode (no output)"},

	{{"filename",    required_argument,	NULL,  1  },
	 "Load program from <file>", "<file>"},

	{{"progsec",	 required_argument,	NULL,  2  },
	 "Load program in <section> of the ELF file", "<section>"},

	{{0, 0, NULL,  0 }, NULL, false}
};

static bool global_exit;

// static struct xsk_umem_info *configure_xsk_umem(void *buffer, uint64_t size)
// {
// 	struct xsk_umem_info *umem;
// 	int ret;

// 	umem = calloc(1, sizeof(*umem));
// 	if (!umem)
// 		return NULL;

// 	ret = xsk_umem__create(&umem->umem, buffer, size, &umem->fq, &umem->cq,
// 			       NULL);
// 	if (ret) {
// 		errno = -ret;
// 		return NULL;
// 	}

// 	umem->buffer = buffer;
// 	return umem;
// }

// static uint64_t xsk_alloc_umem_frame(struct xsk_socket_info *xsk)
// {
// 	uint64_t frame;
// 	if (xsk->umem_frame_free == 0)
// 		return INVALID_UMEM_FRAME;

// 	frame = xsk->umem_frame_addr[--xsk->umem_frame_free];
// 	xsk->umem_frame_addr[xsk->umem_frame_free] = INVALID_UMEM_FRAME;
// 	return frame;
// }

// static void xsk_free_umem_frame(struct xsk_socket_info *xsk, uint64_t frame)
// {
// 	assert(xsk->umem_frame_free < NUM_FRAMES);

// 	xsk->umem_frame_addr[xsk->umem_frame_free++] = frame;
// }

// static uint64_t xsk_umem_free_frames(struct xsk_socket_info *xsk)
// {
// 	return xsk->umem_frame_free;
// }

// static struct xsk_socket_info *xsk_configure_socket(struct config *cfg,
// 						    struct xsk_umem_info *umem)
// {
// 	struct xsk_socket_config xsk_cfg;
// 	struct xsk_socket_info *xsk_info;
// 	uint32_t idx;
// 	uint32_t prog_id = 0;
// 	int i;
// 	int ret;

// 	xsk_info = calloc(1, sizeof(*xsk_info));
// 	if (!xsk_info)
// 		return NULL;

// 	xsk_info->umem = umem;
// 	xsk_cfg.rx_size = XSK_RING_CONS__DEFAULT_NUM_DESCS;
// 	xsk_cfg.tx_size = XSK_RING_PROD__DEFAULT_NUM_DESCS;
// 	xsk_cfg.libbpf_flags = 0;
// 	xsk_cfg.xdp_flags = cfg->xdp_flags;
// 	xsk_cfg.bind_flags = cfg->xsk_bind_flags;
// 	ret = xsk_socket__create(&xsk_info->xsk, cfg->ifname,
// 				 cfg->xsk_if_queue, umem->umem, &xsk_info->rx,
// 				 &xsk_info->tx, &xsk_cfg);

// 	if (ret)
// 		goto error_exit;

// 	ret = bpf_get_link_xdp_id(cfg->ifindex, &prog_id, cfg->xdp_flags);
// 	if (ret)
// 		goto error_exit;

// 	/* Initialize umem frame allocation */

// 	for (i = 0; i < NUM_FRAMES; i++)
// 		xsk_info->umem_frame_addr[i] = i * FRAME_SIZE;

// 	xsk_info->umem_frame_free = NUM_FRAMES;

// 	/* Stuff the receive path with buffers, we assume we have enough */
// 	ret = xsk_ring_prod__reserve(&xsk_info->umem->fq,
// 				     XSK_RING_PROD__DEFAULT_NUM_DESCS,
// 				     &idx);

// 	if (ret != XSK_RING_PROD__DEFAULT_NUM_DESCS)
// 		goto error_exit;

// 	for (i = 0; i < XSK_RING_PROD__DEFAULT_NUM_DESCS; i ++)
// 		*xsk_ring_prod__fill_addr(&xsk_info->umem->fq, idx++) =
// 			xsk_alloc_umem_frame(xsk_info);

// 	xsk_ring_prod__submit(&xsk_info->umem->fq,
// 			      XSK_RING_PROD__DEFAULT_NUM_DESCS);

// 	return xsk_info;

// error_exit:
// 	errno = -ret;
// 	return NULL;
// }

// static void complete_tx(struct xsk_socket_info *xsk)
// {
// 	unsigned int completed;
// 	uint32_t idx_cq;

// 	if (!xsk->outstanding_tx)
// 		return;

// 	sendto(xsk_socket__fd(xsk->xsk), NULL, 0, MSG_DONTWAIT, NULL, 0);


// 	/* Collect/free completed TX buffers */
// 	completed = xsk_ring_cons__peek(&xsk->umem->cq,
// 					XSK_RING_CONS__DEFAULT_NUM_DESCS,
// 					&idx_cq);

// 	if (completed > 0) {
// 		for (int i = 0; i < completed; i++)
// 			xsk_free_umem_frame(xsk,
// 					    *xsk_ring_cons__comp_addr(&xsk->umem->cq,
// 								      idx_cq++));

// 		xsk_ring_cons__release(&xsk->umem->cq, completed);
// 		xsk->outstanding_tx -= completed < xsk->outstanding_tx ?
// 			completed : xsk->outstanding_tx;
// 	}
// }

static inline __sum16 csum16_add(__sum16 csum, __be16 addend)
{
	uint16_t res = (uint16_t)csum;

	res += (__u16)addend;
	return (__sum16)(res + (res < (__u16)addend));
}

static inline __sum16 csum16_sub(__sum16 csum, __be16 addend)
{
	return csum16_add(csum, ~addend);
}

static inline void csum_replace2(__sum16 *sum, __be16 old, __be16 new)
{
	*sum = ~csum16_add(csum16_sub(~(*sum), old), new);
}

static bool process_packet(struct xsk_socket_info *xsk,
			   uint64_t addr, uint32_t len)
{
	unsigned long now = gettime();
	uint8_t *pkt = NULL;
	struct Payload *pl;
	pl = (void *) pkt+42;
	if (pl->uid <MAX_LOG_ENTRY && pl->uid >= 0) {
		log_time_stamps[pl->uid] = now;
	}
	// unsigned long v = 0;
	// if (bpf_map_lookup_elem(time_xsks_map_fd, &pl->uid, &v) == 0) {
	// 	printf("uid[%lu] kern[%lu] us[%lu] delta[%lu]\n",
	// 	pl->uid, v, now, now-v);
	// }
	return false;
}

static void rx_and_process(struct config *cfg,
			   struct xsk_socket_info *xsk_socket)
{
	struct pollfd fds[2];
	int ret, nfds = 1;

	memset(fds, 0, sizeof(fds));
	fds[0].fd = xsk_socket__fd(xsk_socket->xsk);
	fds[0].events = POLLIN;

	while(!global_exit) {
		if (cfg->xsk_poll_mode) {
			ret = poll(fds, nfds, -1);
			if (ret <= 0 || ret > 1)
				continue;
		}
		handle_receive_packets(xsk_socket);
	}
}

static double calc_period(struct stats_record *r, struct stats_record *p)
{
	double period_ = 0;
	__u64 period = 0;

	period = r->timestamp - p->timestamp;
	if (period > 0)
		period_ = ((double) period / NANOSEC_PER_SEC);

	return period_;
}

static void stats_print(struct stats_record *stats_rec,
			struct stats_record *stats_prev)
{
	uint64_t packets, bytes;
	double period;
	double pps; /* packets per sec */
	double bps; /* bits per sec */

	char *fmt = "%-12s %'11lld pkts (%'10.0f pps)"
		" %'11lld Kbytes (%'6.0f Mbits/s)"
		" period:%f\n";

	period = calc_period(stats_rec, stats_prev);
	if (period == 0)
		period = 1;

	packets = stats_rec->rx_packets - stats_prev->rx_packets;
	pps     = packets / period;

	bytes   = stats_rec->rx_bytes   - stats_prev->rx_bytes;
	bps     = (bytes * 8) / period / 1000000;

	printf(fmt, "AF_XDP RX:", stats_rec->rx_packets, pps,
	       stats_rec->rx_bytes / 1000 , bps,
	       period);

	packets = stats_rec->tx_packets - stats_prev->tx_packets;
	pps     = packets / period;

	bytes   = stats_rec->tx_bytes   - stats_prev->tx_bytes;
	bps     = (bytes * 8) / period / 1000000;

	printf(fmt, "       TX:", stats_rec->tx_packets, pps,
	       stats_rec->tx_bytes / 1000 , bps,
	       period);

	printf("\n");
}

static void *stats_poll(void *arg)
{
	unsigned int interval = 2;
	struct xsk_socket_info *xsk = arg;
	static struct stats_record previous_stats = { 0 };

	previous_stats.timestamp = gettime();

	/* Trick to pretty printf with thousands separators use %' */
	setlocale(LC_NUMERIC, "en_US");

	while (!global_exit) {
		sleep(interval);
		// xsk->stats.timestamp = gettime();
		// stats_print(&xsk->stats, &previous_stats);
		// previous_stats = xsk->stats;
	}
	return NULL;
}

static void exit_application(int signal)
{
	signal = signal;
	global_exit = true;
}

int main(int argc, char **argv)
{
	/* user space log */
    log_time_stamps = (unsigned long *) malloc(MAX_LOG_ENTRY*8);
    if (!log_time_stamps) {
        printf("Malloc log_time_stamps failed\n!");
        return -1;
    }
    memset(log_time_stamps, 0, MAX_LOG_ENTRY*8);
    for (int i=0; i<MAX_LOG_ENTRY; i++) {
        if (log_time_stamps[i] != 0) {
            printf("memset log_time_stamps failed\n!");
            return -1;
        }
    }

	int ret;
	int xsks_map_fd;
	void *packet_buffer;
	uint64_t packet_buffer_size;
	struct rlimit rlim = {RLIM_INFINITY, RLIM_INFINITY};
	struct config cfg = {
		.ifindex   = -1,
		.do_unload = false,
		.filename = "",
		.progsec = "xdp_sock"
	};
	struct xsk_umem_info *umem;
	struct xsk_socket_info *xsk_socket;
	struct bpf_object *bpf_obj = NULL;
	pthread_t stats_poll_thread;

	/* Global shutdown handler */
	signal(SIGINT, exit_application);

	/* Cmdline options can change progsec */
	parse_cmdline_args(argc, argv, long_options, &cfg, "__doc__");

	/* Required option */
	if (cfg.ifindex == -1) {
		fprintf(stderr, "ERROR: Required option --dev missing\n\n");
		usage(argv[0], "__doc__", long_options, (argc == 1));
		return EXIT_FAIL_OPTION;
	}

	/* Unload XDP program if requested */
	if (cfg.do_unload)
		return xdp_link_detach(cfg.ifindex, cfg.xdp_flags, 0);

	/* Load custom program if configured */
	if (cfg.filename[0] != 0) {
		struct bpf_map *map;
		struct bpf_map *time_map;

		bpf_obj = load_bpf_and_xdp_attach(&cfg);
		if (!bpf_obj) {
			/* Error handling done in load_bpf_and_xdp_attach() */
			exit(EXIT_FAILURE);
		}

		/* We also need to load the xsks_map */
		map = bpf_object__find_map_by_name(bpf_obj, "xsks_map");
		xsks_map_fd = bpf_map__fd(map);
		if (xsks_map_fd < 0) {
			fprintf(stderr, "ERROR: no xsks map found: %s\n",
				strerror(xsks_map_fd));
			exit(EXIT_FAILURE);
		}

		/* Time Log map */
		time_map = bpf_object__find_map_by_name(bpf_obj, "uid_timestamps");
		time_xsks_map_fd = bpf_map__fd(time_map);
		if (time_xsks_map_fd < 0) {
			fprintf(stderr, "ERROR: no xsks map found: %s\n",
				strerror(time_xsks_map_fd));
			exit(EXIT_FAILURE);
		}
	}

	/* Allow unlimited locking of memory, so all memory needed for packet
	 * buffers can be locked.
	 */
	// if (setrlimit(RLIMIT_MEMLOCK, &rlim)) {
	// 	fprintf(stderr, "ERROR: setrlimit(RLIMIT_MEMLOCK) \"%s\"\n",
	// 		strerror(errno));
	// 	exit(EXIT_FAILURE);
	// }

	/* Allocate memory for NUM_FRAMES of the default XDP frame size */
	// packet_buffer_size = NUM_FRAMES * FRAME_SIZE;
	// if (posix_memalign(&packet_buffer,
	// 		   getpagesize(), /* PAGE_SIZE aligned */
	// 		   packet_buffer_size)) {
	// 	fprintf(stderr, "ERROR: Can't allocate buffer memory \"%s\"\n",
	// 		strerror(errno));
	// 	exit(EXIT_FAILURE);
	// }

	/* Initialize shared packet_buffer for umem usage */
	// umem = configure_xsk_umem(packet_buffer, packet_buffer_size);
	// if (umem == NULL) {
	// 	fprintf(stderr, "ERROR: Can't create umem \"%s\"\n",
	// 		strerror(errno));
	// 	exit(EXIT_FAILURE);
	// }

	/* Open and configure the AF_XDP (xsk) socket */
	// xsk_socket = xsk_configure_socket(&cfg, umem);
	// if (xsk_socket == NULL) {
	// 	fprintf(stderr, "ERROR: Can't setup AF_XDP socket \"%s\"\n",
	// 		strerror(errno));
	// 	exit(EXIT_FAILURE);
	// }

	/* Start thread to do statistics display */
	if (verbose) {
		ret = pthread_create(&stats_poll_thread, NULL, stats_poll,
				     xsk_socket);
		if (ret) {
			fprintf(stderr, "ERROR: Failed creating statistics thread "
				"\"%s\"\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	/* Receive and count packets than drop them */
	rx_and_process(&cfg, xsk_socket);

	/* Export log file for userspace*/
	printf("\nExporting log file ...\n");   // enter new line to avoid the Ctrl+C (^C) char
    FILE *fp;
    fp = fopen(path_log_export_xdp_us, "w");
    for (unsigned long i=0; i<MAX_LOG_ENTRY; i++) {
        fprintf(fp, "%lu\n", log_time_stamps[i]);
		// if (log_time_stamps[i] != 0) {
		// 	printf("uid[%lu] us: %lu\n", i, log_time_stamps[i]);
		// }
    }
    fclose(fp);
	free(log_time_stamps);

	/* Export log file for kernel space (from map) */
	FILE *fp2;
	fp2 = fopen(path_log_export_xdp_kern, "w");
	unsigned long v = 0;
	for (unsigned i=0; i<MAX_LOG_ENTRY; i++) {
		if (bpf_map_lookup_elem(time_xsks_map_fd, &i, &v) == 0) {
			fprintf(fp2, "%lu\n", v);
			// if (v!=0) {
				// printf("uid[%d] ks[%lu]\n", i, v);
			// }
		} else {
			printf("Error reading from time log map with bpf_map_lookup_elem!\n");
		}
	}
	fclose(fp2);

	/* Cleanup */
	// xsk_socket__delete(xsk_socket->xsk);
	// xsk_umem__delete(umem->umem);
	xdp_link_detach(cfg.ifindex, cfg.xdp_flags, 0);

	return EXIT_OK;
}
