# Slurm Data Mover

A tool for managing data dependencies between jobs on a Workflow.
This tool is responsible sending data on nodes that are allocated executing a job on Slurm

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

## Usage
```
Produce file.txt:
  sdm --produce file.txt
Consume file.txt:
  sdm --consume file.txt
Start Node Server:
  sdm --serve

Options:
  -r<host:port>   --redis=<host:port>   Redis socket address
  -b<port>        --bulk=<port>         Bulk Port
  -w<port>        --wui=<port>          Webui Port
  -c<file>        --consume=<file>      Consume input file
  -p<file>        --produce=<file>      Produce file
  -s              --serve               Start Node server
  -i<ip>          --ip=<ip>             Extra IP for this node
  -m<name@path>   --mount=<name@path>   Add mount to this node
  -n              --nocache             Drop FS caches before exec
  -e<command>     --exec=<command>      Command to execute
  -               --daemon              Run application as a daemon.
  -<mask>         --umask=<mask>        Set the daemon's umask (octal, e.g. 027).
  -<path>         --pidfile=<path>      Write the process ID of the application to given file.
  -h              --help                Print Help

You must pass at least one of Consume,Produce,Serve or Wui.
```
