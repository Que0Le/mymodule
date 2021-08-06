



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

# runbox commands
cd /home/que/Desktop/mymodule
# copy code from dev machine
scp -r -i ~/.ssh/id_rsa que@192.168.1.24:Desktop/mymodule/* /home/que/Desktop/mymodule/
# Compile and run module
make clean && make
sudo rmmod intercept_module
sudo insmod intercept-module.ko
# Compile and run user space program
gcc -Wall user-processing.c -o user
sudo ./user
```

```bash
du -h /var/log/
# Clear journal log
journalctl --disk-usage
sudo journalctl --vacuum-size=500M
sudo -- sh -c "cat /dev/null >  /var/log/kern.log"
sudo rm /var/log/syslog.1
```

Change sublime text to default editor:
```bash
# https://askubuntu.com/a/397387
subl /usr/share/applications/defaults.list
# Search for all instances of gedit (org.gnome.gedit on some systems) 
# and replace them with sublime_text. 
# Save the file, log out and back in, and you should be all set.
```