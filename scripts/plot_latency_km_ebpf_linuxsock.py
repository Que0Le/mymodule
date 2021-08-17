import os.path
import sys
import matplotlib.pyplot as plt
import numpy as np

header_common_path = 'common.h'

log_path = "/home/que/Desktop/mymodule/logs/262k_total_test/"
path_log_export_km = log_path + "km_log_kernel_time.txt"
path_log_export_up = log_path + "user_processing_log_pull_time.txt"
km_path_log_export_server_linuxsocket = log_path + "km_server_log_arrival_time.txt"
#
path_log_export_xdp_kern = log_path + "xdp_kern_log_arrival_time.txt"
path_log_export_xdp_us = log_path + "xdp_us_log_arrival_time.txt"
#
ebpf_path_log_export_server_linuxsocket = log_path + "ebpf_server_log_arrival_time.txt"
path_log_export_ebpf_kern = log_path + "ebpf_kern_log_arrival_time.txt"
path_log_export_ebpf_us = log_path + "ebpf_us_log_arrival_time.txt"

full_log_paths = [
    path_log_export_km,
    path_log_export_up,
    km_path_log_export_server_linuxsocket,
    ebpf_path_log_export_server_linuxsocket,
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

with open(path_log_export_km) as km_log_f:
    for line in km_log_f:
        km_log.append(int(line))
with open(path_log_export_up) as up_log_f:
    for line in up_log_f:
        up_log.append(int(line))
with open(km_path_log_export_server_linuxsocket) as km_lus_log_f:
    for line in km_lus_log_f:
        km_lus_log.append(int(line))
#
with open(ebpf_path_log_export_server_linuxsocket) as ebpf_lus_log_f:
    for line in ebpf_lus_log_f:
        ebpf_lus_log.append(int(line))
with open(path_log_export_ebpf_kern) as ebpf_log_f:
    for line in ebpf_log_f:
        ebpf_log.append(int(line))
with open(path_log_export_ebpf_us) as usebpf_log_f:
    for line in usebpf_log_f:
        usebpf_log.append(int(line))

if len(km_log)!=len(up_log) or \
    len(km_log)!=len(km_lus_log) or \
        len(ebpf_log)!=len(usebpf_log) or \
            len(ebpf_log)!=len(ebpf_lus_log) or \
                len(km_lus_log)!=len(ebpf_lus_log):
    print("Files have diff length. Abort!")
    exit()

for i in range(0, len(km_log)):
    d_upkm = int((up_log[i] - km_log[i])/to_usec)
    d_uskm = int((km_lus_log[i] - km_log[i])/to_usec)
    d_usup = int((km_lus_log[i] - up_log[i])/to_usec)

    d_usebpf_ebpf = int((usebpf_log[i] - ebpf_log[i])/to_usec)
    d_lus_ebpf = int((ebpf_lus_log[i] - ebpf_log[i])/to_usec)
    d_lus_usebpf = int((ebpf_lus_log[i] - usebpf_log[i])/to_usec)

    for j in range(0, len(thresholds)):
        if (d_upkm >= v*thresholds[j][0]) and (d_upkm < v*thresholds[j][1]):
            diff_up_km[j] += 1
        if (d_uskm >= v*thresholds[j][0]) and (d_uskm < v*thresholds[j][1]):
            diff_lus_km[j] += 1
        if (d_usup >= v*thresholds[j][0]) and (d_usup < v*thresholds[j][1]):
            diff_lus_up[j] += 1
        ##
        if (d_usebpf_ebpf >= v*thresholds[j][0]) and (d_usebpf_ebpf < v*thresholds[j][1]):
            diff_usebpf_ebpf[j] += 1
        if (d_lus_ebpf >= v*thresholds[j][0]) and (d_lus_ebpf < v*thresholds[j][1]):
            diff_lus_ebpf[j] += 1
        if (d_lus_usebpf >= v*thresholds[j][0]) and (d_lus_usebpf < v*thresholds[j][1]):
            diff_lus_usebpf[j] += 1

print("diff_up_km")
print(diff_up_km)
print("diff_lus_km")
print(diff_lus_km)
print("diff_lus_up")
print(diff_lus_up)
#
print("diff_usebpf_ebpf")
print(diff_usebpf_ebpf)
print("diff_lus_ebpf")
print(diff_lus_ebpf)
print("diff_lus_usebpf")
print(diff_lus_usebpf)

""" Plotting """
x = np.arange(len(labels))  # the label locations
width = 0.10  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(
    x - 3*width, diff_up_km, width, 
    label='diff_up_km:')
rects2 = ax.bar(
    x - 2*width, diff_usebpf_ebpf, width, 
    label='diff_usebpf_ebpf:')
rects3 = ax.bar(
    x - 1**width, diff_lus_km, width, 
    label='diff_us_km')
#
rects4 = ax.bar(
    x + 1*width, diff_lus_ebpf, width, 
    label='diff_lus_ebpf')
rects5 = ax.bar(
    x + 2*width, diff_lus_up, width, 
    label='diff_lus_up')
rects6 = ax.bar(
    x + 3*width, diff_lus_usebpf, width, 
    label='diff_lus_usebpf')
# Add some text for labels, title and custom x-axis tick labels, etc.
if v==1000:
    ax.set_ylabel('Number of packets')
    ax.set_xlabel('Diff in nsec (*1000)')
    ax.set_title('Number of packets received sorted in latency. Total: ' + str(len(km_log)))
elif v==1:
    ax.set_ylabel('Number of packets')
    ax.set_xlabel('Diff in usec')
    ax.set_title('Number of packets received sorted in latency. Total: ' + str(len(km_log)))

ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend()

ax.bar_label(rects1, padding=3)
ax.bar_label(rects2, padding=3)
ax.bar_label(rects3, padding=3)
fig.set_size_inches(18.5, 10.5)
fig.tight_layout()

plt.show()