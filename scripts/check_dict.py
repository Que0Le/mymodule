import os.path
import sys
import matplotlib.pyplot as plt
import numpy as np
import statistics

log_path = "/home/que/Desktop/mymodule/logs/"
path_km = log_path + "KM_kern.txt"
path_km_user = log_path + "KM_user.txt"
path_km_server_linuxsocket = log_path + "KM_socket_server.txt"
# #
# path_xdp_kern = log_path + "XDP_kern.txt"
# path_xdp_us = log_path + "XDP_user.txt"

path_ebpf_server_linuxsocket = log_path + "EBPF_socket_server.txt"
path_ebpf_kern = log_path + "EBPF_kern.txt"
path_ebpf_us = log_path + "EBPF_user.txt"


full_log_paths = [
    path_km,
    path_km_user,
    path_km_server_linuxsocket,
    path_ebpf_server_linuxsocket,
    path_ebpf_kern,
    path_ebpf_us
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
    [ 11, 13 ],
    [ 13, 16 ],
    [ 16, 19 ],
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
diff_s_km = [0] * len(thresholds)
diff_s_up = [0] * len(thresholds)

diff_usebpf_ebpf = [0] * len(thresholds)
diff_s_ebpf = [0] * len(thresholds)
diff_s_usebpf = [0] * len(thresholds)

diffs_count_in_threshold_ranges = [
    diff_up_km, diff_s_km, diff_s_up,
    diff_usebpf_ebpf, diff_s_ebpf, diff_s_usebpf
]

km_log = []
up_log = []
km_s_log = []
ebpf_s_log = []
ebpf_log = []
usebpf_log = []

""" Open logs and load data """
with open(path_km) as km_log_f:
    for line in km_log_f:
        km_log.append(int(line))
with open(path_km_user) as up_log_f:
    for line in up_log_f:
        up_log.append(int(line))
with open(path_km_server_linuxsocket) as km_s_log_f:
    for line in km_s_log_f:
        km_s_log.append(int(line))
# #
with open(path_ebpf_server_linuxsocket) as ebpf_s_log_f:
    for line in ebpf_s_log_f:
        ebpf_s_log.append(int(line))
with open(path_ebpf_kern) as ebpf_log_f:
    for line in ebpf_log_f:
        ebpf_log.append(int(line))
with open(path_ebpf_us) as usebpf_log_f:
    for line in usebpf_log_f:
        usebpf_log.append(int(line))

""" Check lengths """
if len(km_log)!=len(up_log) or \
    len(km_log)!=len(km_s_log) or \
        len(ebpf_log)!=len(usebpf_log) or \
            len(ebpf_log)!=len(ebpf_s_log) or \
                len(km_s_log)!=len(ebpf_s_log):
    print("Files have diff length. Abort!")
    exit()

""" Check data correctness """
logs = [km_log, up_log, km_s_log, ebpf_s_log, ebpf_log, usebpf_log]
logs_label = ["km_log", "up_log", "km_s_log", "ebpf_s_log", "ebpf_log", "usebpf_log"]
logs_zeroed = [0] * len(logs)
###
# diffs_up_km = [0] * len(km_log)
# diffs_s_km = [0] * len(km_log)
# diffs_s_up = [0] * len(km_log)
count_diffs_up_km = {}
count_diffs_s_km = {}
count_diffs_s_up = {}

# diffs_usebpf_ebpf = [0] * len(km_log)
# diffs_s_ebpf = [0] * len(km_log)
# diffs_s_usebpf = [0] * len(km_log)
count_diffs_usebpf_ebpf = {}
count_diffs_s_ebpf = {}
count_diffs_s_usebpf = {}

count_diffs = [
    count_diffs_up_km, count_diffs_s_km, count_diffs_s_up, 
    count_diffs_usebpf_ebpf, count_diffs_s_ebpf, count_diffs_s_usebpf
]
# diffs = [diffs_up_km, diffs_s_km, diffs_s_up, diffs_usebpf_ebpf, diffs_s_ebpf, diffs_s_usebpf]
diffs_label = ["diffs_up_km", "diffs_s_km", "diffs_s_up", "diffs_usebpf_ebpf", "diffs_s_ebpf", "diffs_s_usebpf"]
diffs_zeroed = [0] * len(diffs_label)

for i in range(0, len(km_log)):
    ### Check zero
    for j in range(0, len(logs)):
        if logs[j][i] == 0:
            logs_zeroed[j] = logs_zeroed[j] + 1
    ### Cal diffs
    d_up_km         = up_log[i] - km_log[i]
    if d_up_km<0:
        diffs_zeroed[0] += 1
    # elif (up_log[i]!=0 and km_log[i]!=0):
    #     diffs_up_km[i] = d_up_km
    g = count_diffs_up_km.get(d_up_km)
    if g:
        count_diffs_up_km[d_up_km] = g+1
    else:
        count_diffs_up_km[d_up_km] = 1
    #
    d_s_km          = km_s_log[i] - km_log[i]
    if d_s_km<0:
        diffs_zeroed[1] += 1
    # elif (km_s_log[i]!=0 and km_log[i]!=0):
    #     diffs_s_km[i] = d_s_km
    g = count_diffs_s_km.get(d_s_km)
    if g:
        count_diffs_s_km[d_s_km] = g+1
    else:
        count_diffs_s_km[d_s_km] = 1
    #
    d_s_up          = km_s_log[i] - up_log[i]
    if d_s_up<0:
        diffs_zeroed[2] += 1
    # elif (km_s_log[i]!=0 and up_log[i]!=0):
    #     diffs_s_up[i] = d_s_up
    g = count_diffs_s_up.get(d_s_up)
    if g:
        count_diffs_s_up[d_s_up] = g+1
    else:
        count_diffs_s_up[d_s_up] = 1
    #
    d_usebpf_ebpf   = usebpf_log[i] - ebpf_log[i]
    if d_usebpf_ebpf<0:
        diffs_zeroed[3] += 1
    # elif (usebpf_log[i]!=0 and ebpf_log[i]!=0):
    #     diffs_usebpf_ebpf[i] = d_usebpf_ebpf
    g = count_diffs_usebpf_ebpf.get(d_usebpf_ebpf)
    if g:
        count_diffs_usebpf_ebpf[d_usebpf_ebpf] = g+1
    else:
        count_diffs_usebpf_ebpf[d_usebpf_ebpf] = 1
    #
    d_s_ebpf        = ebpf_s_log[i] - ebpf_log[i]
    if d_s_ebpf<0:
        diffs_zeroed[4] += 1
    g = count_diffs_s_ebpf.get(d_s_ebpf)
    if g:
        count_diffs_s_ebpf[d_s_ebpf] = g+1
    else:
        count_diffs_s_ebpf[d_s_ebpf] = 1
    #
    d_s_usebpf      = ebpf_s_log[i] - usebpf_log[i]
    if d_s_ebpf<0:
        diffs_zeroed[5] += 1
    g = count_diffs_s_usebpf.get(d_s_usebpf)
    if g:
        count_diffs_s_usebpf[d_s_usebpf] = g+1
    else:
        count_diffs_s_usebpf[d_s_usebpf] = 1
    #

print("#################")
print("Zeroed: ")
for i in range(0, len(logs_zeroed)):
    print(f"{str(logs_label[i])}: {str(logs_zeroed[i])}")
print("#################")
print("Negatived: ")
for i in range(0, len(diffs_zeroed)):
    print(f"{str(diffs_label[i])}: {str(diffs_zeroed[i])}")
print("#################")
print("Max-Min: ")
for i in range(0, len(diffs_zeroed)):
    print(f"{str(diffs_label[i])}: {str(max(diffs_label[i]))}-{str(min(diffs_label[i]))}")
print("#################")

for i_cd in range(0, len(count_diffs)):
    for item in count_diffs[i_cd].items():
        diff = int(item[0]/to_usec)
        for i_thres in range(0, len(thresholds)):
            if (diff >= v*thresholds[i_thres][0]) and (diff < v*thresholds[i_thres][1]):
                diffs_count_in_threshold_ranges[i_cd][i_thres] += item[1]
                break
            


print("diff_up_km")
print(diff_up_km)
print(statistics.mean(diff_up_km))
print(sum(diff_up_km)/len(diff_up_km))
print("diff_s_km")
print(diff_s_km)
print(statistics.mean(diff_s_km))
print(sum(diff_s_km)/len(diff_s_km))
print("diff_s_up")
print(diff_s_up)
print(statistics.mean(diff_s_up))
print(sum(diff_s_up)/len(diff_s_up))
#
print("diff_usebpf_ebpf")
print(diff_usebpf_ebpf)
print(statistics.mean(diff_usebpf_ebpf))
print(sum(diff_s_up)/len(diff_s_up))
print("diff_s_ebpf")
print(diff_s_ebpf)
print(statistics.mean(diff_s_ebpf))
print(sum(diff_s_ebpf)/len(diff_s_ebpf))
print("diff_s_usebpf")
print(diff_s_usebpf)
print(statistics.mean(diff_s_usebpf))
print(sum(diff_s_usebpf)/len(diff_s_usebpf))

""" Plotting """
x = np.arange(len(labels))  # the label locations
width = 0.11  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(
    x - 3*width, diff_up_km, width, 
    label='diff_up_km:')
rects2 = ax.bar(
    x - 2*width, diff_usebpf_ebpf, width, 
    label='diff_usebpf_ebpf:')
rects3 = ax.bar(
    x - 1**width, diff_s_km, width, 
    label='diff_s_km')
#
rects4 = ax.bar(
    x + 1*width, diff_s_ebpf, width, 
    label='diff_s_ebpf')
# rects5 = ax.bar(
#     x + 2*width, diff_s_up, width, 
#     label='diff_s_up')
# rects6 = ax.bar(
#     x + 3*width, diff_s_usebpf, width, 
#     label='diff_s_usebpf')
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
ax.bar_label(rects4, padding=3)
# ax.bar_label(rects5, padding=3)
# ax.bar_label(rects6, padding=3)
fig.set_size_inches(18.5, 10.5)
fig.tight_layout()

plt.xticks(rotation='vertical')
plt.yticks(rotation='vertical')
plt.show()