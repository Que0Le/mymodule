import os.path
import sys



header_common_path = 'common.h'
# TODO: read c header file for these info.
km_log_path = "/home/que/Desktop/mymodule/logs/km_log_kernel_time.txt"
up_log_path = "/home/que/Desktop/mymodule/logs/user_processing_log_pull_time.txt"
us_log_path = "/home/que/Desktop/mymodule/logs/server_log_arrival_time.txt"

if (not os.path.isfile(km_log_path)) or \
    (not os.path.isfile(up_log_path)) or \
    (not os.path.isfile(us_log_path)):
    print("file(s) not existed! Exit now ...")
    exit()

thresholds = [
    [-(sys.maxsize-1), 0],
    [0,1],
    [ 1, 2],
    [ 2,3 ],
    [ 3, 4],
    [ 5,6 ],
    [ 7, 8],
    [ 9,10 ],
    [ 11,12 ],
    [ 13,14 ],
    [ 15,16 ],
    [ 17,18 ],
    [ 19,20 ],
    [ 20, 25],
    [ 25, 30],
    [ 30, sys.maxsize],
]

diff_up_km = [0] * len(thresholds)
diff_us_km = [0] * len(thresholds)
diff_us_up = [0] * len(thresholds)

km_log = []
up_log = []
us_log = []

with open(km_log_path) as km_log_f:
    for line in km_log_f:
        km_log.append(int(line))
with open(up_log_path) as up_log_f:
    for line in up_log_f:
        up_log.append(int(line))
with open(us_log_path) as us_log_f:
    for line in us_log_f:
        us_log.append(int(line))

if len(km_log)!=len(up_log) or len(km_log)!=len(us_log) or len(us_log)!=len(up_log):
    print("Files has diff length. Abort!")
    exit()
to_usec = 1
for i in range(0, len(km_log)):
    d_upkm = int((up_log[i] - km_log[i])/to_usec)
    d_uskm = int((us_log[i] - km_log[i])/to_usec)
    d_usup = int((us_log[i] - up_log[i])/to_usec)

    for j in range(0, len(thresholds)):
        if (d_upkm >= thresholds[j][0]) and (d_upkm <= thresholds[j][1]):
            diff_up_km[j] += 1
        if (d_uskm >= thresholds[j][0]) and (d_uskm <= thresholds[j][1]):
            diff_us_km[j] += 1
        if (d_usup >= thresholds[j][0]) and (d_usup <= thresholds[j][1]):
            diff_us_up[j] += 1

print("diff_up_km")
print(diff_up_km)
print("diff_us_km")
print(diff_us_km)
print("diff_us_up")
print(diff_us_up)