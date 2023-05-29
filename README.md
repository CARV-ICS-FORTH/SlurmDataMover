# Slurm Data Manager

A Library that manage data dependencies between jobs on a Workflow. This library is responsible sending data on nodes that are allocated executing a job on Slurm

# Requirements

Arax requires the following packages:

- cmake
- make
- gcc + g++

Optionaly:

- ccmake (configuration ui)
- libpoco (vdf tool)
- libncurses (vtop)
- doxygen (documentation)

## ArchLinux

    sudo pacman -S cmake # poco ncurses doxygen

## CentOS

    sudo yum install cmake # poco-devel poco-foundation poco-net ncurses-devel doxygen

## Ubuntu

    sudo apt-get install cmake # cmake-curses-gui libpoco-dev libncursesw5-dev libncurses-dev doxygen

# Folder layout
