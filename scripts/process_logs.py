import os.path
import sys
import matplotlib.pyplot as plt
import numpy as np

header_common_path = 'common.h'
# TODO: read c header file for these info.
path_log_export_km = "/home/que/Desktop/mymodule/logs/km_log_kernel_time.txt"
path_log_export_up = "/home/que/Desktop/mymodule/logs/user_processing_log_pull_time.txt"
path_log_export_us = "/home/que/Desktop/mymodule/logs/server_log_arrival_time.txt"
path_log_export_xdp_kern = "/home/que/Desktop/mymodule/logs/xdp_kern_log_arrival_time.txt"
path_log_export_xdp_us = "/home/que/Desktop/mymodule/logs/xdp_us_log_arrival_time.txt"

if (not os.path.isfile(path_log_export_km)) or \
    (not os.path.isfile(path_log_export_up)) or \
    (not os.path.isfile(path_log_export_us)) or \
    (not os.path.isfile(path_log_export_xdp_kern)) or \
    (not os.path.isfile(path_log_export_xdp_us)):
    print("file(s) not existed! Exit now ...")
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
diff_us_km = [0] * len(thresholds)
diff_us_up = [0] * len(thresholds)

km_log = []
up_log = []
us_log = []

with open(path_log_export_km) as km_log_f:
    for line in km_log_f:
        km_log.append(int(line))
with open(path_log_export_up) as up_log_f:
    for line in up_log_f:
        up_log.append(int(line))
with open(path_log_export_us) as us_log_f:
    for line in us_log_f:
        us_log.append(int(line))

if len(km_log)!=len(up_log) or len(km_log)!=len(us_log) or len(us_log)!=len(up_log):
    print("Files has diff length. Abort!")
    exit()

for i in range(0, len(km_log)):
    d_upkm = int((up_log[i] - km_log[i])/to_usec)
    d_uskm = int((us_log[i] - km_log[i])/to_usec)
    d_usup = int((us_log[i] - up_log[i])/to_usec)

    for j in range(0, len(thresholds)):
        if (d_upkm >= v*thresholds[j][0]) and (d_upkm < v*thresholds[j][1]):
            diff_up_km[j] += 1
        if (d_uskm >= v*thresholds[j][0]) and (d_uskm < v*thresholds[j][1]):
            diff_us_km[j] += 1
        if (d_usup >= v*thresholds[j][0]) and (d_usup < v*thresholds[j][1]):
            diff_us_up[j] += 1
    # for j in d_upkm:
    # if d_upkm<0:
    #     print(d_upkm)
    # print(d_usup)

print("diff_up_km")
print(diff_up_km)
print("diff_us_km")
print(diff_us_km)
print("diff_us_up")
print(diff_us_up)

""" Plotting """
x = np.arange(len(labels))  # the label locations
width = 0.15  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(
    x - 3*width/2, diff_up_km, width, 
    label='diff_up_km: Latency from Kernel Module\n to the USERSPACE PROCESSING component')
rects2 = ax.bar(
    x , diff_us_km, width, 
    label='diff_us_km: Latency from the normal linux socket USERSPACE SERVER\n to the KERNEL MODULE component')
rects3 = ax.bar(
    x + 3*width/2, diff_us_up, width, 
    label='diff_us_up: : Different in Arrival time between USERSPACE SERVER \n and USERSPACE PROCESSING component')

# Add some text for labels, title and custom x-axis tick labels, etc.
if v==1000:
    ax.set_ylabel('Number of packets')
    ax.set_xlabel('Diff in nsec (*1000)')
    ax.set_title('Number of packets received softed in latency. Total: ' + str(len(km_log)))
elif v==1:
    ax.set_ylabel('Number of packets')
    ax.set_xlabel('Diff in usec')
    ax.set_title('Number of packets received softed in latency. Total: ' + str(len(km_log)))

ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend()

ax.bar_label(rects1, padding=3)
ax.bar_label(rects2, padding=3)
ax.bar_label(rects3, padding=3)
fig.set_size_inches(18.5, 10.5)
fig.tight_layout()

plt.show()