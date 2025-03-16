#!/bin/bash
sudo mount -t vfat -o loop boot/boot.img tmp/
sudo rm -rf tmp/*
sudo cp ./boot/loader.bin tmp/
sudo cp ./kernel/kernel.bin tmp/
sudo sync
sudo umount tmp/