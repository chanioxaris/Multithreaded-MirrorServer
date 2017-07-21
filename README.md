## Overview

This is an implementation of a Mirror Server using [multithreading](https://en.wikipedia.org/wiki/Thread_(computing)#Multithreading) and [TCP sockets](https://en.wikipedia.org/wiki/Network_socket). The purpose of a Mirror Server is to transfer and store files, which user requested from other servers. In this specific implementation three different programs are included:
- Mirror Server:
- Content Server: 
- Mirror Initiator:


The below image describes the operation of Mirror Server.

![structure](https://github.com/chanioxaris/Multithreaded-MirrorServer/blob/master/img/structure.png)

## Multithreading

### Shared Queue

### Mirror threads

### Worker threads

## Compile

`./makefile`

## Usage

`./ContentServer -p [port] -d [directory to share]`

`./MirrorServer -p [port] -m [directory to save files] -w [number of workers threads]`

`./MirrorInitiator -m [name of MirrorServer] -p [port of MirrorServer] -s [info for files to transfer]`
