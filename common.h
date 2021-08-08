#define DEBUG_KM_10
// #define DEBUG_KM_INCOMING_PACKETS
// #define DEBUG_UP_INCOMING_PACKETS
// #define DEBUG_US_INCOMING_PACKETS
// #define DEBUG_READ_FROM_US

const char *path_prefix = "/proc/";
const char *proc_filename = "intercept_mmap";
const char *path_log_export_km = "/home/que/Desktop/mymodule/logs/km_log_kernel_time.txt";
const char *path_log_export_up = "/home/que/Desktop/mymodule/logs/user_processing_log_pull_time.txt";
const char *path_log_export_us = "/home/que/Desktop/mymodule/logs/server_log_arrival_time.txt";

enum { 
    BUFFER_SIZE = 1024,
    PKT_BUFFER_SIZE = 128,
    PKTS_PER_BUFFER = 8,                // = BUFFER_SIZE / PKT_BUFFER_SIZE
    SIZE_OF_PAGE_HC = 4096,             // hardcode
    MAX_PKT = 100,
    MAX_TRY = 100,
    /* SHIFT x == 2^(x+1) 15=65.536 16=131.072 17=262.144 18=524.288 19=1.048.576 */
    MAX_LOG_ENTRY = 2 << 17,
    /* PAGES_PER_LOG_BUFF = 2^PAGES_ORDER = 64 pages */
    PAGES_ORDER = 6,
    /* 64pages * 4KB/page = 2^18B (262144B) = 32768 unsigned long */
    MAX_ENTRIES_PER_LOG_BUFF = (2 << (PAGES_ORDER-1))*SIZE_OF_PAGE_HC/8,
    NUM_LOG_BUFF = MAX_LOG_ENTRY / MAX_ENTRIES_PER_LOG_BUFF
};

enum  {
    PL_KEEP_ALIVE = 1,
    PL_DATA = 2,
    PL_LOG = 3
};

struct Payload {
    unsigned long client_uid;
    unsigned long uid;
    unsigned long type;
    unsigned long created_time;         //nsec
    unsigned long ks_time_arrival_1;    //nsec
    unsigned long ks_time_arrival_2;    //nsec
    unsigned long us_time_arrival_1;    //nsec
    unsigned long us_time_arrival_2;    //nsec
};

/* //TODO: Better debug
#ifdef DEBUG
#if (DEBUG > 0) && (DEBUG < 2)
printf("Debugging level 1");
#endif

#if (DEBUG > 1) && (DEBUG < 3)
printf("Debugging level 2");
#endif

#if (DEBUG > n-1) && (DEBUG < n)
printf("Debugging level n");
#endif
#endif
*/