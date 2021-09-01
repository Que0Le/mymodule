import os.path
import sys
import matplotlib.pyplot as plt
import numpy as np

log_path = "/home/que/Desktop/mymodule/logs/262k_total_test/"
# path_log_export_km = log_path + "km_log_kernel_time.txt"
# path_log_export_up = log_path + "user_processing_log_pull_time.txt"
# km_path_log_export_server_linuxsocket = log_path + "km_server_log_arrival_time.txt"
# #
# path_log_export_xdp_kern = log_path + "xdp_kern_log_arrival_time.txt"
# path_log_export_xdp_us = log_path + "xdp_us_log_arrival_time.txt"
#


ebpf_path_log_export_server_linuxsocket = log_path + "ebpf_server_log_arrival_time.txt"
path_log_export_ebpf_kern = log_path + "ebpf_kern_log_arrival_time.txt"
path_log_export_ebpf_us = log_path + "ebpf_us_log_arrival_time.txt"

full_log_paths = [
    # path_log_export_km,
    # path_log_export_up,
    # km_path_log_export_server_linuxsocket,
    # ebpf_path_log_export_server_linuxsocket,
    path_log_export_ebpf_kern,
    path_log_export_ebpf_us
]

for path in full_log_paths:
    if not os.path.isfile(path):
        print("file: " + str(path) + " not existed! Exit now ...")
        exit()

to_usec = 1
v = 1000/to_usec

thresholds = [
    [-(sys.maxsize-1)/1000, 0],
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
    [ 30, sys.maxsize/1000],
]
labels = []
for i in range(0, len(thresholds)):
    if i==0:
        l = str("-maxsize") + '-' + str(thresholds[i][1])
    elif i==(len(thresholds)-1):
        l = str(thresholds[i][0]) + '-' + str("maxsize")
    else:
        l = str(thresholds[i][0]) + '-' + str(thresholds[i][1])
    labels.append(l)


diff_up_km = [0] * len(thresholds)
diff_lus_km = [0] * len(thresholds)
diff_lus_up = [0] * len(thresholds)

diff_usebpf_ebpf = [0] * len(thresholds)
diff_lus_ebpf = [0] * len(thresholds)
diff_lus_usebpf = [0] * len(thresholds)

km_log = []
up_log = []
km_lus_log = []
ebpf_lus_log = []
ebpf_log = []
usebpf_log = []

# with open(path_log_export_km) as km_log_f:
#     for line in km_log_f:
#         km_log.append(int(line))
# with open(path_log_export_up) as up_log_f:
#     for line in up_log_f:
#         up_log.append(int(line))
# with open(km_path_log_export_server_linuxsocket) as km_lus_log_f:
#     for line in km_lus_log_f:
#         km_lus_log.append(int(line))
# #
with open(ebpf_path_log_export_server_linuxsocket) as ebpf_lus_log_f:
    for line in ebpf_lus_log_f:
        ebpf_lus_log.append(int(line))
with open(path_log_export_ebpf_kern) as ebpf_log_f:
    for line in ebpf_log_f:
        ebpf_log.append(int(line))
with open(path_log_export_ebpf_us) as usebpf_log_f:
    for line in usebpf_log_f:
        usebpf_log.append(int(line))

# if len(km_log)!=len(up_log) or \
#     len(km_log)!=len(km_lus_log) or \
#         len(ebpf_log)!=len(usebpf_log) or \
#             len(ebpf_log)!=len(ebpf_lus_log) or \
#                 len(km_lus_log)!=len(ebpf_lus_log):
#     print("Files have diff length. Abort!")
#     exit()

print(len(usebpf_log))
print(len(ebpf_log))
if len(usebpf_log) != len(ebpf_log) != len(ebpf_lus_log):
    print("len diff: " + str(len(usebpf_log)) + " " + str(len(ebpf_log)) + " " + str(len(ebpf_lus_log)))
    exit
neg_count = 0
usebpf_zero = 0
ebpf_zeoro = 0
for i in range(0, len(ebpf_log)):
    if usebpf_log[i]==0:
        usebpf_zero += 1
    if ebpf_log[i]==0:
        ebpf_zeoro += 1
    d = usebpf_log[i]-ebpf_log[i]
    if  d< 0:
        neg_count += 1
        print("negative: uid=" + str(i) + \
            " d=" + str(d) +\
            " usebpf_log[i]="+str(usebpf_log[i])+ \
            " ebpf_log[i]="+str(ebpf_log[i]))
    else:
        diff_usebpf_ebpf.append(d)
print("neg_count: " + str(neg_count))
print("usebpf_zero: " + str(usebpf_zero))
print("ebpf_zeoro: " + str(ebpf_zeoro))
print("diff_usebpf_ebpf avg: " + str(sum(diff_usebpf_ebpf)/len(diff_usebpf_ebpf)))
exit
