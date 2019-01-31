#!/bin/bash
#qemu-system-x86_64 -m 64M -hda /home/kenny/cmpt300/debian_squeeze_amd64_standard.qcow2 -append "root=/dev/sda1 console=tty0 console=ttyS0,115200n8" -kernel /home/kenny/cmpt300/linux_v4.13.9/arch/x86_64/boot/bzImage -nographic -net nic,vlan=1 -net user,vlan=1 -redir tcp:2222::22
qemu-system-x86_64 -m 64M -hda /home/kenny/cmpt300/debian_squeeze_amd64_standard.qcow2 -append "root=/dev/sda1 console=tty0 console=ttyS0,115200n8" -kernel /home/kenny/cmpt300/linux_patch/linux_v4.13.9/arch/x86_64/boot/bzImage -nographic -net nic,vlan=1 -net user,vlan=1 -redir tcp:2222::22
