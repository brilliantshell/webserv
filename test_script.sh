#!/usr/bin/env bash

make -C build

if [ $? -ne 0 ]; then
		echo -e "${RED}Build failed${RESET}"
		exit 1
fi

CONFIG_PATH="./configs/tests/IOMultiplexing/"

GREEN="\033[0;32m"
RED="\033[0;31m"
RESET="\033[0m"

function quit_server(){
	kill $!
	exit 0
}

trap quit_server INT

function TEST() {
	echo "TEST $1"
	echo "=================================================="
	echo "execute server..."
	./build/server "${CONFIG_PATH}s_${2}.config" &
	echo "execute client..."
	sleep .5
	./build/client "${CONFIG_PATH}s_${2}.config" $3

	if [ $? -eq 0 ]; then
		echo -e "\n${GREEN}TEST $1 PASSED${RESET}"
	else
		echo -e "\n${RED}TEST $1 FAILED${RESET}"
	fi

	echo -e "==================================================\n"

	sleep 1

	kill $!

	# sp="/-\|"
	# sc=0
	# spin() {
	# 	printf "%s\b${sp:sc++:1}"
	# 	((sc==${#sp})) && sc=0``
	# }

	# while [[ -n $(jobs -r) ]]
	# do
	# 	spin
	# 	sleep .3
	# done

}


TEST "00" "00" 1
TEST "01" "00" 10
TEST "02" "00" 100
TEST "03" "00" 1000
TEST "04" "00" 2000
TEST "05" "01" 10


TEST "05" "01" 1000
