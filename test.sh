#! /usr/bin/env bash 

cat <<EOF
Welcome to the BrilliantServer!
EOF


ASCII_ART_PATH=etc/asciiart.txt
SERVER="./BrilliantServer"
CONFIG="./configs/www.config"
SRCS="./srcs"

L_CYAN="\033[1;96m"
RESET="\033[0m"

function quit_server(){
	pkill ${SERVER:2}
	exit 0
}

trap quit_server INT

head -6 $ASCII_ART_PATH

printf "\n\n\n"
mkdir -p $HOME/goinfre/www/post 
ln -s $HOME/goinfre/www www 
cp tests/ResponseFormatter/error.html www


function press_enter {
	printf "\n\n\n"
	read -p "Press enter to continue... " -s 
	printf "\n\n\n"
}

	pkill ${SERVER:2}

 # SECTION : a configuration file as argument, or use a default path ======================================

 echo -e $L_CYAN"#1 : 하나의 config 파일을 인자로 받거나, default 경로로 서버 설정 파일 읽기\n\n$RESET"

 echo -e "#1.0 : 유효하지 않은 설정 파일 경로를 인자로 받았을 때 (CONFIG = invalid_config)\n\n"


 CONFIG="invalid_config"

 ( set -x ; $SERVER $CONFIG )

 press_enter

 echo -e "#1.1 : config 파일을 인자로 받아서 서버 설정 파일 읽기\n\n"

 read -p "인자로 받을 config 경로 입력 : " CONFIG


 if [ -f $CONFIG ]; then
 	code $CONFIG
 	( set -x ; $SERVER $CONFIG & )
 	( set -x ; curl -v http://localhost:4141/ )
 	echo
 	pkill ${SERVER:2}
 else
 	echo "config 파일이 존재하지 않아요..."
 fi

 press_enter

 echo -e "#1.2 : default 경로로 서버 설정 파일 읽기\n\n"

 [ -f "./default.config" ] && code ./default.config

 ( set -x; $SERVER & )
 ( set -x; curl -v http://localhost:8282/ )
 echo
 pkill ${SERVER:2}

 press_enter

 # !SECTION ===============================================================================================



 # SECTION : can’t execve another web server ==============================================================

 echo -e $L_CYAN"#2 : 다른 웹서버 프로그램 execve 금지\n\n"$RESET

 echo "We use execve to execute ... "
 echo -e "\n" $(set -x; grep --color=always -n execve ${SRCS}/*.cpp)
 code -g `grep --color=always -n execve ./srcs/*.cpp | awk '{ print $1 }'`

 press_enter

 # !SECTION ===============================================================================================



 # SECTION : must be non-blocking and use only 1 poll() (or equivalent) ===================================

 echo -e $L_CYAN"#3 : poll 함수 하나만 사용해서 non-blocking io 구현\n\n"$RESET

 echo -e $( set -x ; nm -u ./BrilliantServer | grep --color=always -E "kqueue|epoll|poll|select" ) "\n"
 echo -e "\n>>>> kqueue <<<< 한 함수만 쓴 모습이 보이시나요? ^^\n"

 ( set -x ; $SERVER & )
 sleep 1
 ( set -x ;  lsof -p $(pgrep BrilliantServer) | grep --color=always -C 2 "KQUEUE" )

 echo -e "\n>>>> kqueue <<<< 개수도 단 한 개 입니다!!!!\n"

 pkill ${SERVER:2}

 press_enter

 # !SECTION ===============================================================================================



 # SECTION : poll() (or equivalent) must check read and write at the same time ============================
  # must never do a read or a write operation without going through poll() (or equivalent) =======

 echo -e $L_CYAN"#4 : kqueue 로 read & write 이벤트 동시 추적 (EVFILT_READ 와 EVFILT_WRITE)"$RESET
 echo -e $L_CYAN"#5 : kqueue 거치지 않은 I/O 금지\n\n"$RESET

 code -g ${SRCS}/HttpServer.cpp

 press_enter

 # !SECTION ===============================================================================================



 # SECTION : Checking the value of errno is strictly forbidden after a read or a write operation ==========

 echo -e $L_CYAN"#6 : read/write 후 errno 값 확인 금지\n\n"$RESET

 ( set -x ; grep --color=always -n -C 5 -E "read\(|recv\(|send\(|write\(|writev\(" ${SRCS}/*.cpp )

 press_enter

 # !SECTION ===============================================================================================



 # SECTION : A request to your server should never hang forever ===========================================
 
 echo -e $L_CYAN"#7 : 서버 요청이 무한정 hang 되지 않도록 구현\n\n"$RESET

 ( set -x ; $SERVER & )
 ( set -x ; nc localhost 8282 -c )

 pkill ${SERVER:2}

 code -g ${SRCS}/HttpServer.cpp:59

 press_enter

 # !SECTION ===============================================================================================



 # SECTION : HTTP response status codes must be accurate ==================================================

 echo -e $L_CYAN"#8 : 정확한 HTTP 응답 상태코드 전송\n\n"$RESET

 echo "test 할 url 리스트"
 ( set -x ; cat ./siege/status_code.txt )

 press_enter
 ( set -x ; $SERVER configs/www.config & )

 echo "응답 상태코드 확인"
 touch forbidden && chmod -r forbidden
 ( set -x ; siege -f ./siege/status_code.txt -c 1 --reps=once )
 rm forbidden

 press_enter
 echo  -e "501, 505 상태 코드 확인\n"

 echo "501 NOT IMPLEMENTED"
 ( set -x ; { echo -ne "GETGETGET / HTTP/1.1\r\nHost: ghan\r\n\r\n"; sleep 1; } | nc localhost 4141 )
 printf "\n\n"

 echo "505 HTTP VERSION NOT SUPPORTED"
 ( set -x ; { echo -ne "GET / HTTP/2.0\r\nHost: ghan\r\n\r\n"; sleep 1; } | nc localhost 4141 )
 printf "\n\n"

 pkill ${SERVER:2}

 press_enter


 # !SECTION ===============================================================================================



 # SECTION : default error pages if none are provided =====================================================

 echo -e "$L_CYAN#9 : 에러 페이지 없으면 default 에러 페이지 표시\n\n$RESET"

 code -g configs/www.config

 echo -e "#9.0 : config 에 에러 페이지 설정 && 에러 페이지 access 가능\n\n"

 ( set -x ; $SERVER configs/www.config & )
 printf "\n\n"

 ( set -x ; curl -v http://$(hostname):4143/file_does_not_exist )

 press_enter

 echo -e "#9.1 : config 에 에러 페이지 설정 되지 않았을 경우 (cwd 에서 error.html 찾기)\n\n"

 ( set -x ; curl -v http://localhost:4142/file_does_not_exist )

 press_enter

 echo -e "#9.2 : 에러 페이지에 access 불가능한 경우\n\n"

 ( set -x ; curl -v http://localhost:4144/file_does_not_exist )

 pkill ${SERVER:2}

 press_enter


 # !SECTION ===============================================================================================



# SECTION : can’t use fork for something else than CGI ===================================================

echo -e "$L_CYAN#10 : fork() 는 CGI 에만 사용\n\n$RESET"

( set -x ; grep --color=always -n -C 5 -E "fork\(|vfork\(" ${SRCS}/*.cpp )

code -g ${SRCS}/CgiManager.cpp:176

press_enter

# !SECTION ===============================================================================================


# SECTION : must be able to serve a fully static website =================================================

echo -e "$L_CYAN#11 : 엄.청.난. static page\n\n$RESET"

git clone https://github.com/woosetcho/html_css_piscine.git $HOME/goinfre/www/html_css_piscine

echo "브라우저를 켜서 http://localhost:8282/ 를 확인해보세요"

( set -x ; $SERVER configs/www.config >/dev/null & )

sleep 0.7

open "http://localhost:8282/"

press_enter

pkill ${SERVER:2}
sleep 1

# !SECTION ===============================================================================================

# SECTION : Clients must be able to upload files =========================================================

echo -e "$L_CYAN#12 : 파일 업로드\n\n$RESET"

( set -x ; $SERVER configs/www.config & )

echo -e "\n#12.0 : cli 로 업로드\n"

echo -e "기존 www/post"
( set -x ; ls www/post/ )
printf "\n\n"

echo -ne 'POST /post/chunked.txt HTTP/1.1\r\nHost: ghan\r\nConnection: close\r\ntransfer-Encoding: chunked\r\n\r\n4\r\nghan\r\n6\r\njiskim\r\n8\r\nyongjule\r\n0\r\n\r\n' | pbcopy
echo "Transfer-Encoding 테스트 직접 입력 (pbpaste 유 노)"
( set -x ; /usr/bin/nc -v localhost 4142 -c ) 
printf "\n\n"

( set -x ; curl -v -X POST localhost:4142/post/by_cURL.txt --data @CMakeLists.txt )
printf "\n\n"
( set -x ; curl -v -X POST localhost:4142/post/by_cURL.txt --data @CMakeLists.txt )
printf "\n\n"

echo -e "업로드 후 www/post"
( set -x ; ls www/post/ )

press_enter

echo -e "#12.1 : 브라우저로 업로드 "

echo -e "기존 /tmp ls"
( set -x ; ls /tmp )

sleep 0.7
open "http://localhost:4141/py_cgi/file_upload.html"

press_enter

echo -e "업로드 후 /tmp ls"
( set -x ; ls /tmp )

press_enter

echo -e "생성했던 by_cURL*.txt, chunked.txt 파일들 DELETE 로 제거\n\n"

( set -x ; curl -v -X DELETE localhost:4142/delete/by_cURL.txt )
printf "\n\n"

( set -x ; curl -v -X DELETE localhost:4142/delete/by_cURL_0.txt )
printf "\n\n"

( set -x ; curl -v -X DELETE localhost:4142/delete/chunked.txt )
printf "\n\n"

pkill ${SERVER:2}

press_enter


# !SECTION ===============================================================================================

tail -6 $ASCII_ART_PATH

