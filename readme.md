



```bash
# insert module
sudo insmod intercept-module.ko
# remove module
sudo rmmod intercept_module
# compile user app
gcc user-processing.c -o user-processing
# run user app:
sudo ./user-processing /proc/intercept_mmap
```

```bash
#
sudo dmesg -wH

#
scp -i ~/.ssh/id_rsa que@192.168.1.24:Desktop/mymodule/* /home/que/Desktop/mymodule/
```