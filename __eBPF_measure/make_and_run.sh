ssh -i ~/.ssh/id_rsa que@192.168.1.11 "cd ~/Desktop/xdp-tutorial/eBPF_measure && make clean && make" &&
rsync -avIL -e "ssh -i ~/.ssh/id_rsa" --exclude-from="rsync-exclude.txt" que@192.168.1.11:Desktop/mymodule/* /home/que/Desktop/mymodule/ &&
echo "Running ebpf_measure_user ..."
sudo ./ebpf_measure_user -d ens33 --filename ebpf_measure_kern.o