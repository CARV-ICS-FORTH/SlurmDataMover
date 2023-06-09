# Slurm Data Manager

A Library that manage data dependencies between jobs on a Workflow. This library is responsible sending data on nodes that are allocated executing a job on Slurm

# Requirements

SDM requires the following packages:

- cmake
- make
- gcc + g++

Optionaly:

- ccmake (configuration ui)
- libpoco
- doxygen (documentation)

## ArchLinux

    sudo pacman -S cmake # poco doxygen

## CentOS

    sudo yum install cmake # poco-devel poco-foundation poco-net doxygen

## Ubuntu

    sudo apt-get install cmake # libpoco-dev doxygen

# Folder layout
