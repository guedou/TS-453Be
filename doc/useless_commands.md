GV 20190910: check if the following commands are usefull in this documentation

```
ls -l /media/boot/initrd.boot
-rw-r--r-- 1 root root 14626106 Dec 12  2016 /media/boot/initrd.boot                                                          

cat /media/boot/initrd.boot.cksum
2679584251 14626106 initrd.boot

cksum /media/boot/initrd.boot
2679584251 14626106 /media/boot/initrd.boot
```

```
xz -dc < ../initrd.boot | cpio --quiet -i --make-directories
# edit /etc/fstab
find . 2> /dev/null | cpio --quiet -c -o | xz -9 --format=lzma > ../initrd_new.boot                                           
cksum ../initrd_new.boot > ../initrd_new.boot.cksum
# edit ../initrd_new.boot.cksum and change the path to /media/boot/initrd.boot                                                
```

