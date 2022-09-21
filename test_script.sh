#!/usr/bin/env bash

if [ "$#" -ne 1 ]; then
		echo "Usage: $0 file" >&2
		exit 1
fi

if ! [ -f "$1" ]; then
		echo "Error: $1 is not a file" >&2
		exit 1
fi

# ret=`find $1 -name "*.config"`


echo "execute server..."
./build/server $1 &
echo "execute client..."
sleep .5
./build/client $1 

# server_pid=`jobs -p` 
# kill -term $server_pid
