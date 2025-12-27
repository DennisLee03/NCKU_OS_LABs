#!/usr/bin/bash
make 
sudo insmod osfs.ko
mkdir mnt
sudo mount -t osfs none mnt/
cd mnt
sudo touch test.txt
sudo bash -c "echo 'I LOVE OSLAB' > test.txt"
cat test.txt