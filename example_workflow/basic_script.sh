#!/bin/bash
echo  "Droping cache from Ram at" 
date
sudo sh -c 'echo 3 >  /proc/sys/vm/drop_caches'    
#echo 3 >  /proc/sys/vm/drop_caches
