# mymodule
## Note on setup system
We used `scp` to copy code and executable files from dev machine to run-machine. However, in order to exclude unwanted data/folder, we switched to `rsync`:
```bash
# old
scp -r -i ~/.ssh/id_rsa que@192.168.1.11:Desktop/mymodule/* /home/que/Desktop/mymodule/
# new
rsync -av -e "ssh -i ~/.ssh/id_rsa" que@192.168.1.11:Desktop/mymodule/* /home/que/Desktop/mymodule/
```
## Note on kernel module

### Memory
- Share data with userspace program is done using `mmap` inform of a file (we create a file in ). The user space endpoint 





## Note on ebpf kernel program
```c
// boundary check is tricky. Check boundary should be done before any packet access. Otherwise error:
/* */
// ; if (data+sizeof(struct Payload) <= data_end) {
// 8: (bf) r3 = r1
// 9: (07) r3 += 64
// ; if (data+sizeof(struct Payload) <= data_end) {
// 10: (2d) if r3 > r2 goto pc+296
//  R1_w=pkt(id=0,off=0,r=64,imm=0) R2_w=pkt_end(id=0,off=0,imm=0) R3_w=pkt(id=0,off=64,r=64,imm=0) R6_w=inv2 R10=fp0 fp-8=mmmm????
// ; memcpy(&PL, data+42, 60/* sizeof(struct Payload) */);
// 11: (71) r3 = *(u8 *)(r1 +89)
// invalid access to packet, off=89 size=1, R1(id=0,off=89,r=64)
// R1 offset is outside of the packet
// processed 12 insns (limit 1000000) max_states_per_insn 0 total_states 0 peak_states 0 mark_read 0

// libbpf: -- END LOG --
// libbpf: failed to load program 'xdp_sock'
// libbpf: failed to load object 'xdp_measure_kern.o'
// ERR: loading BPF-OBJ file(xdp_measure_kern.o) (-22): Invalid argument
// ERR: loading file: xdp_measure_kern.o
/* */
if (data_end >= data+42+sizeof(struct Payload))  // this should work
```
