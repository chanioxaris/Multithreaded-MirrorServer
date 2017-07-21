## Overview

This is an implementation of a Mirror Server using [multithreading](https://en.wikipedia.org/wiki/Thread_(computing)#Multithreading) and [TCP sockets](https://en.wikipedia.org/wiki/Network_socket). The purpose of a Mirror Server is to transfer and store files, which user requested from other servers. In this specific implementation three different programs are included:

- Mirror Server: the main server that uses two types of threads (Mirror threads and Worker threads, more details below) to transfer and store the files or directories requested by user.
- Content Server: the servers that contain the files or directories to be transfered. Works alongside with Mirror and Worker threads.
- Mirror Initiator: informs the Mirror Server which files or directories from which Content Servers, user requested to transfer and store. The template of the -s input argument which inlude the above information is: `ContentServerAddress1:ContentServerPort1:dirorfile1:delay1,ContentServerAddress2:ContentServerPort2:dirorfile2:delay2,....ContentServerAddressN:ContentServerPortN:dirorfileN:delayN`. Delay refers to how many seconds the Content Server sleeps before transfer a file or directory.
<br />
<br />
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
