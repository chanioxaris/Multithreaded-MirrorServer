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

### Mirror threads
Their job is to connect to each Content server requested, send a "LIST" command and put the information of the files or directories need to be transfered to the shared queue. The number of Mirror threads created are the same as the requested Content servers (each thread corresponds to a Content Server).

### Worker threads
Their job is to read from the shared buffer and start transferring the file or directory from a specific Content server to the Mirror server under a specific folder. The number of Mirror threads is determided by the user, using the command line arguments (-w). Their operation continues until there is no file or directory left to transfer.

### Shared Queue
A shared [queue](https://en.wikipedia.org/wiki/Queue_(abstract_data_type)) is used among Mirror and Worker threads to send information, about the files or directories that is about to be transfered. The access to the queue is protected and synchronized with the use of mutexes and condition variables to prevent [busy waiting](https://en.wikipedia.org/wiki/Busy_waiting).

## Compile

`./makefile`

## Usage

`./ContentServer -p [port] -d [directory to share]`

`./MirrorServer -p [port] -m [directory to save files] -w [number of workers threads]`

`./MirrorInitiator -m [name of MirrorServer] -p [port of MirrorServer] -s [info for files to transfer]`
