# SimpleRobots
Lightweight C program to send command to clients from a main server

## Installation
Once the project is cloned, you'll need to generate the common library used both by the client and the server :

     cd common
     make

This makefile will create a static library inside the lib folder.

Then, compile the client :

    cd ../client
    make

And the server :

    cd ../server
    make

## Usage
Firstly, run the server. You can customize on which port the server will bind with the -p option. Other available option can be displayed with the -h option.

    ./server/bin/server -p 8888

To run the client, use :

    ./client/bin/client -p 8888 -a 127.0.0.1

You should now be able to send commands to the client, or download file to server !
