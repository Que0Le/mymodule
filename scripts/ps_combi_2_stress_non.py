import os.path
import sys
import matplotlib.pyplot as plt
import numpy as np
import statistics
import math 

# raw_diffs_up_km = {}
# raw_diffs_s_km = {}
# raw_diffs_usebpf_ebpf = {}
# raw_diffs_s_ebpf = {}
# raw_diffs = [
#     raw_diffs_up_km, raw_diffs_s_km, 
#     raw_diffs_usebpf_ebpf, raw_diffs_s_ebpf,
# ]

translate_cases = {
    "32k_nonstress_3pkpms_1024Bytes": "Nbr packets: 32k. None Stress. Sending rate: 3k/s. Pkt size: 1024 Bytes",
    "32k_stress_3pkpms_1024Bytes": "Nbr packets: 32k. Stress 100%. Sending rate: 3k/s. Pkt size: 1024 Bytes",
    "65k_nonstress_6pkpms_512Bytes": "Nbr packets: 65k. None Stress. Sending rate: 6k/s. Pkt size: 512 Bytes",
    "65k_stress_6pkpms_512Bytes": "Nbr packets: 65k. Stress 100%. Sending rate: 6k/s. Pkt size: 512 Bytes",
    "524k_nonstress_50pkpms_64Bytes": "Nbr packets: 524k. None Stress. Sending rate: 50k/s. Pkt size: 64 Bytes",
    "524k_stress_50pkpms_64Bytes": "Nbr packets: 524k. Stress 100%. Sending rate: 50k/s. Pkt size: 64 Bytes",
}

to_usec = 1
v = 1000/to_usec

thresholds = [
    [0, 1],
    [1, 2],
    [2, 3],
    [3, 4],
    [4, 5],
    [5, 6],
    [6, 7],
    [7, 8],
    [8, 9],
    [9, 10],
    [10, 11],
    [11, 13],
    [13, 16],
    [16, 19],
    [19, 23],
    [23, 27],
    [27, 32],
    [32, 40],
    [40, 50],
    [50, 60],
    [60, 70],
    [70, 80],
    [80, 90],
    [90, 100],
    [100, 150],
    [150, 200],
    [200, 250],
    [250, 300],
    [300, 400],
    [400, 500],
    [500, 800],
    [800, 1000],
    [1000, 100000],
    [100000, 100000000],
]
labels = []
for i in range(0, len(thresholds)):
    l = str(thresholds[i][0]) + '-' + str(thresholds[i][1])
    labels.append(l)
labels[-1] = "1e3-1e5"
labels[-2] = "1e5-1e8"
# test_suit = testcases (6 case = stress + nonstress) * nbr_iteration = 30 files
nbr_iteration = 5
conditions = [
    {"stress": 0},
    {"nonstress": 1}
]

relative_log_path = "../logs/test_suit/"
all_dir = os.listdir(relative_log_path)
test_cases = []
for dir in all_dir:
    case = dir.split("#")[0]
    if not case in test_cases:
        test_cases.append(case)
# print(sorted(test_cases))
# exit()

for test_case in test_cases:
    print(test_case + "--------------------------------------------------------------------")
    # i.e 
    # '524k_nonstress_50pkpms_64Bytes', '524k_stress_50pkpms_64Bytes', 
    # '65k_nonstress_6pkpms_512Bytes', '65k_stress_6pkpms_512Bytes'
    # '32k_nonstress_3pkpms_1024Bytes', '32k_stress_3pkpms_1024Bytes'
    logs_label = ["km_log", "up_log", "km_s_log", "ebpf_log", "usebpf_log", "ebpf_s_log"]
    logs_zeroed = [0] * 6#len(logs)
    ###
    count_diffs_up_km = {}
    count_diffs_s_km = {}
    count_diffs_usebpf_ebpf = {}
    count_diffs_s_ebpf = {}
    # sum_entry = 0

    count_diffs = [
        count_diffs_up_km, count_diffs_s_km,
        count_diffs_usebpf_ebpf, count_diffs_s_ebpf, 
    ]

    diffs_label = [
        "diffs_up_km", "diffs_s_km",
        "diffs_usebpf_ebpf", "diffs_s_ebpf",
    ]
    diffs_neg = [0] * len(diffs_label)

    to_subtract = [
        (1,0), # up_log - km_log
        (2,0), # km_s_log - km_log
        (4,3), # usebpf_log - ebpf_log
        (5,3), # ebpf_s_log - ebpf_log
    ]
    #############################
    for iteration in range(1, nbr_iteration+1): # 1-5
        print(test_case + f"#{str(iteration)}")
        path_km = relative_log_path + test_case + f"#{str(iteration)}/" + "KM_kern.txt"#+"\n"
        path_km_user = relative_log_path + test_case + f"#{str(iteration)}/" + "KM_user.txt"#+"\n"
        path_km_server_linuxsocket = relative_log_path + test_case + f"#{str(iteration)}/" + "KM_socket_server.txt"#+"\n"
        path_ebpf_server_linuxsocket = relative_log_path + test_case + f"#{str(iteration)}/" + "EBPF_socket_server.txt"#+"\n"
        path_ebpf_kern = relative_log_path + test_case + f"#{str(iteration)}/" + "EBPF_kern.txt"#+"\n"
        path_ebpf_us = relative_log_path + test_case + f"#{str(iteration)}/" + "EBPF_user.txt"#+"\n"
        # print(path_km, path_km_user, path_km_server_linuxsocket,path_ebpf_server_linuxsocket,path_ebpf_kern,path_ebpf_us)

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

        km_log = []
        up_log = []
        km_s_log = []
        ebpf_log = []
        usebpf_log = []
        ebpf_s_log = []

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
        with open(path_ebpf_kern) as ebpf_log_f:
            for line in ebpf_log_f:
                ebpf_log.append(int(line))
        with open(path_ebpf_us) as usebpf_log_f:
            for line in usebpf_log_f:
                usebpf_log.append(int(line))
        with open(path_ebpf_server_linuxsocket) as ebpf_s_log_f:
            for line in ebpf_s_log_f:
                ebpf_s_log.append(int(line))

        """ Check lengths """
        if len(km_log)!=len(up_log) or \
            len(km_log)!=len(km_s_log) or \
                len(ebpf_log)!=len(usebpf_log) or \
                    len(ebpf_log)!=len(ebpf_s_log) or \
                        len(km_s_log)!=len(ebpf_s_log):
            print("Files have diff length. Abort!")
            exit()

        """ Check data correctness """
        logs = [km_log, up_log, km_s_log, ebpf_log, usebpf_log, ebpf_s_log]
        # logs_label = ["km_log", "up_log", "km_s_log", "ebpf_log", "usebpf_log", "ebpf_s_log"]
        # logs_zeroed = [0] * len(logs)
        # ###
        # count_diffs_up_km = {}
        # count_diffs_s_km = {}
        # count_diffs_usebpf_ebpf = {}
        # count_diffs_s_ebpf = {}

        # count_diffs = [
        #     count_diffs_up_km, count_diffs_s_km,
        #     count_diffs_usebpf_ebpf, count_diffs_s_ebpf, 
        # ]

        # diffs_label = [
        #     "diffs_up_km", "diffs_s_km",
        #     "diffs_usebpf_ebpf", "diffs_s_ebpf",
        # ]
        # diffs_neg = [0] * len(diffs_label)

        # to_subtract = [
        #     (1,0), # up_log - km_log
        #     (2,0), # km_s_log - km_log
        #     (4,3), # usebpf_log - ebpf_log
        #     (5,3), # ebpf_s_log - ebpf_log
        # ]

        """ Calculate the diffs """
        for i in range(0, len(km_log)):
            ### Cal diffs
            for pair_th in range(0, len(to_subtract)):
                # sum_entry += 1
                pair = to_subtract[pair_th]
                ### Check zero
                z = 0
                if logs[pair[0]][i]==0:
                    logs_zeroed[pair[0]] = logs_zeroed[pair[0]] + 1
                    z+=1
                if logs[pair[1]][i]==0:
                    logs_zeroed[pair[1]] = logs_zeroed[pair[1]] + 1
                    z+=1
                if z!=0:
                    continue    # No need to calculate diff because one of the measurement is zero
                d = logs[pair[0]][i] - logs[pair[1]][i]
                # Check neg
                if d<0:
                    # print(f"{diffs_label[pair_th]} {d}")
                    diffs_neg[pair_th] += 1
                # Add to counter
                g = count_diffs[pair_th].get(d)
                if g:
                    count_diffs[pair_th][d] = g+1
                else:
                    count_diffs[pair_th][d] = 1

        # print("#################")
        # print("Zeroed: ")
        # for i in range(0, len(diffs_label)):
        #     if logs_zeroed[i] != 0:
        #         print(f"{str(logs_label[i])}: {str(logs_zeroed[i])}")
        # print("#################")
        # print("Negatived: ")
        # for i in range(0, len(diffs_label)):
        #     if diffs_neg[i] != 0:
        #         print(f"{str(diffs_label[i])}: {str(diffs_neg[i])}")
    """ Done collecting data for all iterations """

    # Because each zero value is counted twice, we modify them to correct value
    logs_zeroed = list(map(lambda v: int(v/2), logs_zeroed))

    print("#################")
    print("Zeroed: ")
    for i in range(0, len(diffs_label)):
        if logs_zeroed[i] != 0:
            print(f"{str(logs_label[i])}: {str(logs_zeroed[i])}")
    print("-----------------")
    # print("Negatived: ")
    # for i in range(0, len(diffs_label)):
    #     if diffs_neg[i] != 0:
    #         print(f"{str(diffs_label[i])}: {str(diffs_neg[i])}")
    # print("#################")
    # print("Max-Min: ")
    # for i in range(0, len(diffs_label)):
    #     print(f"{str(diffs_label[i])}: {str(max(count_diffs[i].keys()))}-{str(min(count_diffs[i].keys()))}")
    # print("#################")

    diffs_median = [0] * len(diffs_label)
    diffs_avg = [0] * len(diffs_label)
    diffs_stddev = [0] * len(diffs_label)
    # Average
    for i in range(0, len(diffs_label)):
        sumary = 0
        sum_square_deviations = 0
        nbr_values = sum(count_diffs[i].values())
        for key, value in count_diffs[i].items():
            sumary += key*value
        avg = sumary/nbr_values
        diffs_avg[i] = avg
        # print(f"Avg {diffs_label[i]} = {avg}")

        # Standard deviation
        for key, value in count_diffs[i].items():
            sum_square_deviations += ((key-avg)**2) * value
        stddev = math.sqrt(sum_square_deviations/nbr_values)
        diffs_stddev[i] = stddev
        # print(f"Standard deviation {stddev}")

    # Median
    for i in range(0, len(diffs_label)):
        nbr_values = sum(count_diffs[i].values())
        middle_nbr = int(nbr_values/2)
        current_nbr_values = 0
        sorted_keys = sorted(count_diffs[i].keys())
        for key in sorted_keys:
            current_nbr_values += count_diffs[i].get(key)
            if current_nbr_values >= middle_nbr:
                diffs_median[i] = key
                # print(f"Median: {str(key)}")
                break
    for i in range(0, len(diffs_label)):
        print(diffs_label[i])
        print(f"  Max-Min : {str(max(count_diffs[i].keys()))}-{str(min(count_diffs[i].keys()))}")
        print(f"  Negative: {str(diffs_neg[i])}")
        print(f"  Median  : {str(diffs_median[i])}")
        print(f"  Average : {str(diffs_avg[i])}")
        print(f"  Stddev  : {str(diffs_stddev[i])}")

    diff_up_km = [0] * len(thresholds)
    diff_s_km = [0] * len(thresholds)
    diff_usebpf_ebpf = [0] * len(thresholds)
    diff_s_ebpf = [0] * len(thresholds)

    diffs_count_in_threshold_ranges = [
        diff_up_km, diff_s_km, 
        diff_usebpf_ebpf, diff_s_ebpf, 
    ]

    """ Export unexpected values to file to evaluate later """
    with open("out_range.txt", 'w') as f:
        for i_cd in range(0, len(count_diffs)):
            to_pop = []
            for item in count_diffs[i_cd].items():
                diff = int(item[0]/to_usec)
                for i_thres in range(0, len(thresholds)):
                    if (diff >= v*thresholds[i_thres][0]) and (diff < v*thresholds[i_thres][1]):
                        # If value in threshold range, we increase the counter and add this item to pop list
                        diffs_count_in_threshold_ranges[i_cd][i_thres] += item[1]
                        to_pop.append(item[0])
                        break
            [count_diffs[i_cd].pop(x) for x in to_pop]
            # After that, this dictionatry should only contain value that are out of threshold (neg, too big, ...)
            f.write("--------------------------------\n")
            f.write(str(diffs_label[i_cd]) + "\n")
            for key in sorted(count_diffs[i_cd]):
                f.write(f"{key}: {count_diffs[i_cd][key]}\n")

    """ Plotting """
    x = np.arange(len(labels))  # the label locations
    width = 0.3  # the width of the bars

    fig, axs = plt.subplots(2,1)
    rects1 = axs[0].bar(
        x - 0.5*width, diff_up_km, width,
        label="T2-T1: User Processing - Kernel Module", color="tab:blue", edgecolor = "black")#___diff_up_km:')
    rects2 = axs[1].bar(
        x - 0.5*width, diff_usebpf_ebpf, width,
        label="T2-T1: User-space App - eBPF kernel program", color="tab:olive", edgecolor = "black")#diff_usebpf_ebpf")
    rects3 = axs[0].bar(
        x + 0.5*width, diff_s_km, width, 
        label="T3-T1: Linux Socket App - Kernel Module", color="tab:pink", hatch='//', edgecolor = "black")#diff_s_km")
    rects4 = axs[1].bar(
        x + 0.5*width, diff_s_ebpf, width, 
        label="T3-T1: Linux Socket App - eBPF kernel program", color="tab:orange", hatch='..', edgecolor = "black")#diff_s_ebpf")

    # Add some text for labels, title and custom x-axis tick labels, etc.
    title_postfix = f"{translate_cases.get(test_case)}"
    axs[0].set_title(f"Latency diffence between kernel module and Linux socket application.\n" + title_postfix)
    axs[1].set_title(f"Latency diffence between eBPF and Linux socket application.\n" + title_postfix)
    for i in range(0, 2):
        axs[i].set_ylabel('Nbr of packet in log scale')
        # if v==1000:
        #     axs[i].set_xlabel('Diff in usec')
        # elif v==1:
        #     axs[i].set_xlabel('Diff in nsec')
        axs[i].set_xticks(x)
        # axs[i].set_xticklabels(labels)
        axs[i].set_yscale('log')
        axs[i].legend()
        axs[i].set_xticklabels(labels, rotation=45)
        # axs[i].set_yticklabels(rotation='vertical')

    # axs[0].bar_label(rects1, padding=3)
    # axs[1].bar_label(rects2, padding=3)
    # axs[0].bar_label(rects3, padding=3)
    # axs[1].bar_label(rects4, padding=3)
    fig.set_size_inches(8, 6)
    fig.tight_layout()

    # plt.xticks(rotation=45)
    plt.yticks(rotation='vertical')
    # plt.subplots_adjust(bottom=0.1)
    # plt.show()
    plt.savefig(f"{test_case}.png")

        

    # break
# print(test_cases)

exit()


