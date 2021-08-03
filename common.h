
const char *path_prefix = "/proc/";
const char *filename = "intercept_mmap";

enum { 
    BUFFER_SIZE = 1024,
    PKT_BUFFER_SIZE = 128,
    PKTS_PER_BUFFER = 8,    // = BUFFER_SIZE / PKT_BUFFER_SIZE
    MAX_PKT = 100,
    MAX_TRY = 100
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
    unsigned long created_time; //nsec
    unsigned long ks_time_arrival_1; //nsec
    unsigned long ks_time_arrival_2; //nsec
    unsigned long us_time_arrival_1; //nsec
    unsigned long us_time_arrival_2; //nsec
};