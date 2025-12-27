#!/usr/bin/bash
sudo umount mnt
sudo rmmod osfs
make clean
rm -rf mnt