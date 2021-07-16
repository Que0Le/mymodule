



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