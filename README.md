# BrilliantServer

# 목차

- [BrilliantServer](#brilliantserver)
- [목차](#목차)
- [0 Introduction](#0-introduction)
	- [0.0 기능 소개](#00-기능-소개)
	- [0.1 설치 및 실행](#01-설치-및-실행)
	- [0.2 설계](#02-설계)
		- [0.2.0 객체 관계도](#020-객체-관계도)
		- [0.2.1 객체 책임/역할](#021-객체-책임역할)
	- [0.2.2 HTTP 요청/응답 flowchart](#022-http-요청응답-flowchart)
- [1 socket 통신](#1-socket-통신)
	- [1.0 socket 이란](#10-socket-이란)
	- [1.1 주소 할당](#11-주소-할당)
	- [1.2 TCP 연결 수립 \& Passive vs. Active](#12-tcp-연결-수립--passive-vs-active)
	- [1.3 socket I/O](#13-socket-io)
	- [1.4 socket 설정](#14-socket-설정)
		- [1.4.0 SO\_REUSEADDR vs SO\_LINGER](#140-so_reuseaddr-vs-so_linger)
- [2 TCP Connection](#2-tcp-connection)
	- [2.0 TCP 상태 전이도와 헤더 구조](#20-tcp-상태-전이도와-헤더-구조)
	- [2.1 Establishing a Connection](#21-establishing-a-connection)
	- [2.2 Data Communication](#22-data-communication)
		- [2.2.0 TCP Window](#220-tcp-window)
	- [2.3 Closing Connection](#23-closing-connection)
		- [2.3.0 CLOSE\_WAIT \& TIME\_WAIT](#230-close_wait--time_wait)
- [3 I/O 다중 처리](#3-io-다중-처리)
	- [3.0 I/O Multiplexing 이란](#30-io-multiplexing-이란)
		- [3.0.1 NON-BLOCKING I/O](#301-non-blocking-io)
	- [3.1 kqueue \& kevent](#31-kqueue--kevent)
		- [3.1.1 kqueue 와 kevent 란](#311-kqueue-와-kevent-란)
		- [3.1.2 kqueue 를 선택한 이유](#312-kqueue-를-선택한-이유)
	- [3.2 최적화](#32-최적화)
		- [3.2.0 writev 와 send](#320-writev-와-send)
		- [3.2.1 Low water mark(SO\_SNDLOWAT) 와 writev](#321-low-water-markso_sndlowat-와-writev)
- [4 HTTP](#4-http)
	- [4.0 HTTP general](#40-http-general)
	- [4.1 HTTP/1.1](#41-http11)
		- [4.1.0 Host 헤더 필드로 name-based 라우팅 지원](#410-host-헤더-필드로-name-based-라우팅-지원)
		- [4.1.1 Transfer-Encoding: chunked](#411-transfer-encoding-chunked)
		- [4.1.1 Connection Management](#411-connection-management)
- [5 Common Gateway Interface](#5-common-gateway-interface)
	- [5.0 Common Gateway Interface 란](#50-common-gateway-interface-란)
	- [5.1 CGI 와 Server 의 소통](#51-cgi-와-server-의-소통)
	- [5.2 CGI 응답](#52-cgi-응답)
		- [5.2.0 Document Response](#520-document-response)
		- [5.2.1 Redirection Response](#521-redirection-response)
		- [5.2.1 Server 의 CGI 응답 처리](#521-server-의-cgi-응답-처리)
- [6 “서버는 죽지 않아!”](#6-서버는-죽지-않아)
- [7 References](#7-references)
	- [7.0 Socket 통신](#70-socket-통신)
	- [7.1 TCP Connection](#71-tcp-connection)
	- [7.2 I/O Multiplexing](#72-io-multiplexing)
	- [7.3 SO\_SNDLOWAT](#73-so_sndlowat)
	- [7.4 HTTP](#74-http)
	- [7.5 CGI](#75-cgi)

---

# 0 Introduction

- **BrilliantServer** 는 [RFC 9110](https://www.rfc-editor.org/rfc/rfc9110), [9112](https://www.rfc-editor.org/rfc/rfc9112) 에 정의된 규격을 따르는 **HTTP/1.1** 버전 origin server 의 구현입니다.
- 이 프로젝트는 [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) 를 따라 **C++98** 로 작성되었습니다.

## 0.0 기능 소개

- `kqueue` 를 활용한 event loop 기반 **non-blocking I/O multiplexing**
- [RFC 3875](https://www.rfc-editor.org/rfc/rfc3875) 를 따르는 CGI 지원
- static 파일에 대한 `GET`/`POST`/`DELETE` HTTP Request 처리
  - `POST` 메소드로 파일 업로드
- HTTP 규격에 맞는 HTTP 응답 생성
- name-based virtual hosting 지원
- directory listing 지원

## 0.1 설치 및 실행

```
git clone https://github.com/brilliantshell/webserv.git && \
cd webserv && \
make -j
```

```
./BrilliantShell [path/to/config]
```

config 파일 작성 규칙은 다음과 같습니다. [Configuration 파일 규칙](/assets/Configuration-파일-규칙.md)

## 0.2 설계

[설계 figma 페이지](https://www.figma.com/file/thjTrbquiaNrRcd7eCBLrp/BrilliantServer-%EA%B0%9D%EC%B2%B4-%EA%B4%80%EA%B3%84%EB%8F%84?node-id=0%3A1&t=vhAD9UNdbaZ3mlCC-1)

### 0.2.0 객체 관계도

![BrilliantServer 객체 관계도.png](/assets/object-relationship.png)

### 0.2.1 객체 책임/역할

![responsibilities.png](/assets/responsibilities.png)

## 0.2.2 HTTP 요청/응답 flowchart

![http_flowchart.png](/assets/http_flowchart.png)

# 1 socket 통신

## 1.0 socket 이란

- socket 통신은 IPC (Inter Process Communication) 의 한 종류다.
- socket 파일은 socket 통신의 종점으로 (endpoint) socket 파일을 연 프로그램은 socket 파일을 연 다른 프로그램과 connection 을 수립하거나, 서로의 주소로 datagram 을 전송하여 서로 통신할 수 있다.
- `socket` 함수로 socket 파일을 열 수 있으며, 성공 시 할당 된 fd 가 반환된다.

  ```c
  #include <sys/socket.h>
  int socket(int domain, int type, int protocol);
  ```
  - `domain` 로 어떤 도메인에서 통신할지 정할 수 있다. 여러개가 있지만 (OS X 에는 8개, Linux 에는 더 많다) 중요한 두개는 `PF_LOCAL` 과 `PF_INET` 이다.
    - `PF_LOCAL` - 도메인/UNIX socket 이라고 부르는 로컬 프로세스 간 통신을 위한 도메인
    - `PF_INET` - TCP socket 이라고 부르는 IP 통신을 위한 도메인
  - `type` 로 전송하는 데이터의 단위를 정할 수 있다.
    - SOCK_STREAM - connection 수립 & byte stream 으로 통신
    - SOCK_DGRAM - 상대의 주소로 datagram 으로 통신
    - SOCK_RAW - datagram 으로 통신, 내부 네트워크 인터페이스에 접근 가능
  - `protocol` 에는 소켓이 따를 프로토콜을 지정한다. 같은 프로토콜로 열린 소켓들끼리만 통신이 가능하다. TCP 는 6. (`/etc/protocols` 참고)

	> 💡 이 문서는 Web Server 에 관련된 문서이기 때문에 아래에서는 TCP socket 에 대해서만 설명한다.

## 1.1 주소 할당

- socket 생성 후 `bind` 함수로 해당 소켓에 주소/식별자를 부여할 수 있다.

  ```c
  #include <sys/socket.h>
  int bind(int socket, const struct sockaddr *address, socklen_t address_len);
  ```
  - `socket` 은 해당 socket 의 fd 이다.
  - `address` 는 할당할 주소 정보를 담는 구조체의 주소값이다. UNIX socket 의 경우 `struct sockaddr_un` 의 주소값을, TCP socket 의 경우 `struct sockaddr_in` (IPv4) / `struct sockaddr_in6` (IPv6) 의 주소값을 캐스팅해서 넣어준다.
  - `address_len` 에는 `address` 구조체의 길이를 넣어준다.
- IPv4 의 경우 주소 구조체는 아래와 같다.

  ```c
  struct in_addr {
  	in_addr_t s_addr;
  };

  struct sockaddr_in {
  	__uint8_t       sin_len;
  	sa_family_t     sin_family; // AF_INET
  	in_port_t       sin_port; // port number
  	struct  in_addr sin_addr; // listen 할 IP 주소 sin_addr.s_addr 에 설정
  	char            sin_zero[8];
  };
  ```

  - `sin_addr.s_addr` 는 Server 의 경우 `INADDR_ANY`(0) 로 설정하여 어떤 주소든지 `sin_port` 에 설정한 port 로 연결을 시도하면 listen 하게 설정한다.
  - port 와 address 는 network byte order 로 저장돼야하기 때문에 `<arpa/inet.h>` 함수들을 활용해야한다.

## 1.2 TCP 연결 수립 & Passive vs. Active

- HTTP Server 는 socket 파일을 열어 (`PF_INET`, `SOCK_STREAM`, 6) Client 프로그램들과 TCP connection 을 수립하여 통신한다. (HTTP/3.0 부터는 UDP 사용)
- Server 와 연결을 시도하는 Client 의 socket 은 “active” socket, Client 의 연결 시도를 기다리는 Server 의 socket 은 “passive” socket 이라고 부른다.
- socket 간의 TCP connection 이 수립되는 과정을 순차적으로 설명하면 아래와 같다.

  - `bind` 이후 Server 의 소켓은 `listen` 함수를 호출하여 “passive”/”listening” 상태로 전환한다.
  
    ```c
    #include <sys/socket.h>
    int listen(int socket, int backlog);
    ```
    - `socket` 은 해당 socket 의 fd 이다.
    - `backlog` 는 연결 수립을 기다리는 요청들의 queue 의 최대 길이이다. queue 가 꽉 차있는 상태에서 연결 수립 요청이 오면 Client 는 ECONNREFUSED 에러를 반환 받는다. (silent limit 128)
  - Server 는 “listening” socket 이 준비된 후 `accept` 함수로 block 하며 Client 의 연결 수립 요청을 기다린다.

    ```c
    #include <sys/socket.h>

    int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
    ```

    - `socket` 은 “passive” socket 의 fd 이다.
    - `address` 는 Client 의 “active” socket 의 주소 정보를 담을 구조체의 주소다.
    - `address_len` 에는 `address` 구조체의 길이를 넣어준다.

  - Client 소켓은 `connect` 함수를 호출하여 Server 에 TCP 연결 수립을 요청하는 “active” 역할을 수행한다.

    ```c
    #include <sys/types.h>
    #include <sys/socket.h>

    int connect(int socket, const struct sockaddr *address, socklen_t address_len);
    ```

    - `socket` 은 해당 socket 의 fd 이다.
    - `address` 에는 연결하고자 하는 Server 의 소켓에 할당된 주소가 입력된 구조체의 주소값을 넣어준다.
    - `address_len` 에는 `address` 구조체의 길이를 넣어준다.

  - Client 의 연결 수립 요청을 수신하면 `accept` 는 blocking 을 풀고 Client 의 “active” socket 과 연결이 수립할 새로운 `AF_INET` socket 을 생성하고, 연결이 수립되면 (TCP ESTABLISHED) 그 socket 의 fd 를 반환한다.

## 1.3 socket I/O

- Server 는 `accept` 함수가 반환한 fd 에 read/recv 하여 Client 가 보낸 요청을 읽고, write/send 하여 Client 에게 응답을 보낼 수 있다.

	> 💡 socket 에 read/write 할 때, 한번의 호출로 시스템의 TCP window size 를 넘을 수 없다.
`sysctl -a | grep buf` 로 max limit 을 확인할 수 있다. (auto * bufmax 가 window size)

	![tcp max buffer](/assets/tcp-max-buffer.png)


## 1.4 socket 설정

- socket 설정을 `getsockopt` 로 확인, `setsockopt` 로 변경할 수 있다.

  ```c
  #include <sys/socket.h>

  int getsockopt(int socket, int level, int option_name, void *restrict option_value,
  socklen_t *restrict option_len);

  int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
  ```

- `setsockopt` 의 `option_name` 인자로 `send`/`recv` buffer size (`SO_SNDBUF`/`SO_RCVBUF`) 등 여러가지 속성을 변경할 수 있다.

### 1.4.0 SO_REUSEADDR vs SO_LINGER

- 특정 ip + port 로 `bind` 되어있는 passive socket 이 Server 종료 시 혹은 실행 중 어떤 이유로 닫혔을 때 해당 socket 은 `TIME-WAIT` 상태가 되고 특별한 설정이 없었다면 2MSL 동안 해당 ip + port 로의 `bind` 가 불가능해진다.
- Server 가 재실행하기 위해 종료 후 2MSL 을 기다려야하는 것은 너무 불편하기 때문에, 이를 해결하기 위해 BrilliantServer 의 passive socket 들은 `SO_REUSEADDR` 로 설정되었다. 특정 ip + port 로 `bind` 하기 전에 `SO_REUSEADDR` 설정을 하면, Server 는 2MSL 을 기다리지 않고 바로 해당 ip + port 를 재사용 (다시 `bind` ) 할 수 있다. `TIME-WAIT` socket 들이 남지만 이는 정상적인 종료 절차이고, Server 에게 문제가 되지 않는다.

  ```c
  // opt 에는 0 이 아닌 숫자가 들어가면 된다 (bool 같은 역할)
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    PRINT_ERROR("address cannot be reused : " << strerror(errno));
    close(fd);
    return -1;
  }
  errno = 0;
  if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    PRINT_ERROR("socket for " << kPort
                              << " cannot be bound : " << strerror(errno));
    close(fd);
    return -1;
  }
  listen(fd, BACKLOG);
  ```
- `SO_LINGER` 옵션으로도 이 문제를 해결할 수 있다. `SO_LINGER` 옵션은 `struct linger` 의 주소를 `setsockopt` 의 `option_value` 로 넘겨주며, `l_onoff` 가 0 이 아니고 `l_linger` 가 양의 정수로 설정될 경우, Server 가 socket 을 `close` 했을 때 아직 보내지지 않은 데이터가 남아 있다면 `l_linger` 초 만큼 `close` 를 block 하게 설정하는데 사용된다. `l_linger` 값을 0 으로 설정하면 정상적인 TCP 연결 종료 절차가 시작되지 않고, TCP 연결에서 `RST` control bit 이 보내지며 `close` 한 socket 이 `TIME-WAIT` 상태에 빠지지 않는다. 하지만 비정상적으로 TCP 연결을 끊기 때문에 이전 TCP 연결이 제대로 정리되지 않아 Connection Reset by Peer 에러가 발생할 위험이 크다.

  ```c
  struct  linger {
  	int     l_onoff;                /* option on (0)/off (non-zero) */
  	int     l_linger;               /* linger time (sec) */
  };
  ```

---

# 2 TCP Connection

## 2.0 TCP 상태 전이도와 헤더 구조

- TCP connection state diagram

  ```
                              +---------+ ---------\      active OPEN
                              |  CLOSED |            \    -----------
                              +---------+<---------\   \   create TCB
                                |     ^              \   \  snd SYN
                   passive OPEN |     |   CLOSE        \   \
                   ------------ |     | ----------       \   \
                    create TCB  |     | delete TCB         \   \
                                V     |                      \   \
            rcv RST (note 1)  +---------+            CLOSE    |    \
         -------------------->|  LISTEN |          ---------- |     |
        /                     +---------+          delete TCB |     |
       /           rcv SYN      |     |     SEND              |     |
      /           -----------   |     |    -------            |     V
  +--------+      snd SYN,ACK  /       \   snd SYN          +--------+
  |        |<-----------------           ------------------>|        |
  |  SYN   |                    rcv SYN                     |  SYN   |
  |  RCVD  |<-----------------------------------------------|  SENT  |
  |        |                  snd SYN,ACK                   |        |
  |        |------------------           -------------------|        |
  +--------+   rcv ACK of SYN  \       /  rcv SYN,ACK       +--------+
     |         --------------   |     |   -----------
     |                x         |     |     snd ACK
     |                          V     V
     |  CLOSE                 +---------+
     | -------                |  ESTAB  |
     | snd FIN                +---------+
     |                 CLOSE    |     |    rcv FIN
     V                -------   |     |    -------
  +---------+         snd FIN  /       \   snd ACK         +---------+
  |  FIN    |<----------------          ------------------>|  CLOSE  |
  | WAIT-1  |------------------                            |   WAIT  |
  +---------+          rcv FIN  \                          +---------+
    | rcv ACK of FIN   -------   |                          CLOSE  |
    | --------------   snd ACK   |                         ------- |
    V        x                   V                         snd FIN V
  +---------+               +---------+                    +---------+
  |FINWAIT-2|               | CLOSING |                    | LAST-ACK|
  +---------+               +---------+                    +---------+
    |              rcv ACK of FIN |                 rcv ACK of FIN |
    |  rcv FIN     -------------- |    Timeout=2MSL -------------- |
    |  -------            x       V    ------------        x       V
     \ snd ACK              +---------+delete TCB          +---------+
       -------------------->|TIME-WAIT|------------------->| CLOSED  |
                            +---------+                    +---------+
  ```
- TCP Header Format

  ```
      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |          Source Port          |       Destination Port        |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                        Sequence Number                        |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                    Acknowledgment Number                      |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     | Offset| Rsrvd |W|C|R|C|S|S|Y|I|            Window             |
     |       |       |R|E|G|K|H|T|N|N|                               |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |           Checksum            |         Urgent Pointer        |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                           [Options]                           |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                                                               :
     :                             Data                              :
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            Note that one tick mark represents one bit position.
  ```

  - Seqeunce Number : 32 bits
    - 해당 세그먼트의 첫 데이터 octet 의 sequence 번호. 예외로 SYN 컨트롤 비트가 세팅될 때는 sequence 번호는 ISN, 첫 데이터 octet 은 ISN + 1 로 설정된다.
  - Acknowledgement Number : 32 bits
    - ACK 컨트롤 비트가 세팅되면, 다음에 받을 것으로 예상되는 세그먼트의 시퀀스 번호가 설정된다. 한 번 연결이 수립되면 항상 전송된다.
  - Reserved (Rsrvd) : 4 bits
    - 컨트롤 비트를 표시한다.
  - Window : 16 bits
    - 발신자가 받을 수 있는 TCP Window 크기 (unsigned number)

## 2.1 Establishing a Connection

- three-way-handshake

  ```
      TCP Peer A                                           TCP Peer B

  1.  CLOSED                                               LISTEN

  2.  SYN-SENT    --> <SEQ=100><CTL=SYN>               --> SYN-RECEIVED

  3.  ESTABLISHED <-- <SEQ=300><ACK=101><CTL=SYN,ACK>  <-- SYN-RECEIVED

  4.  ESTABLISHED --> <SEQ=101><ACK=301><CTL=ACK>       --> ESTABLISHED

  5.  ESTABLISHED --> <SEQ=101><ACK=301><CTL=ACK><DATA> --> ESTABLISHED
  ```

  - 새 커넥션을 만들기 위해서는 각 Peer 별로 32bit 크기의 `ISN`(Initial Sequence Number) 을 생성하고 식별자로써 사용한다. `ISN` 을 사용하면 포트를 재사용 할 경우 각 커넥션을 구분하여 데이터가 혼재되지 않고 SEQ 를 추측하기 어려워지므로 보안이 강화된다.
  - 데이터를 주고 받을 수 있는 상태가 되려면 두 peer 모두 `ESTABLISHED` 상태가 되어야 한다

  1. 서버 (위 그림에서 Peer B) 측에서 passive open 으로 `LISTEN` 상태가 되면 클라이언트 (Peer A) 가 active open 을 시도하길 기다린다.
  2. 클라이언트가 passive open 하면 자신의 ISN 을 시퀀스 넘버 (SEQ) 로 SYN 컨트롤 비트와 함께 서버에게 보내고 `SYN_SENT` 로 전환한다. SYN 을 받았으므로 서버는 `SYN_RECEIVED` 상태로 전환한다.
  3. 세그먼트를 받으면 서버는 자신의 `ISN`을 시퀀스 넘버(SEQ) 로, 받았던 세그먼트의 SEQ + 1 값을 ACK 으로 설정하여 보낸다. 컨트롤 비트 SYN, ACK 로 확인 응답을 받은 클라이언트가 `ESTABLISHED` 로 전환한다.
  4. 클라이언트가 ACK 를 보내고 응답 받은 서버도 `ESTABLISHED` 로 전환한다.
  5. 둘 모두 `ESTABLISHED` 인 상태에서 data 를 주고 받을 수 있다.
     [1.2 TCP 연결 수립 & Passive vs. Active](#12-tcp-연결-수립--passive-vs-active)

## 2.2 Data Communication

### 2.2.0 TCP Window

- TCP Window 란 각 peer 가 임시로 데이터를 받을 수 있는 버퍼이다. 응용 프로그램 측에서 버퍼를 읽어가면 clear 한다. 세그먼트의 Window 헤더 필드로 남은 Window size 를 상대편 peer 에게 알려줄 수 있다. 남은 TCP Window 사이즈가 0에 가까워 지면 버퍼가 비워졌다는 업데이트가 갈 때 까지 전송이 중단된다.
- socket read 가 지연되면 Window size 가 작아져 상대편 peer 의 전송이 중단된다.

## 2.3 Closing Connection

- four-way-handshaking

```
TCP Peer A                                           TCP Peer B

1.  ESTABLISHED                                          ESTABLISHED

2.  (Close)
    FIN-WAIT-1  --> <SEQ=100><ACK=300><CTL=FIN,ACK>  --> CLOSE-WAIT

3.  FIN-WAIT-2  <-- <SEQ=300><ACK=101><CTL=ACK>      <-- CLOSE-WAIT

4.                                                       (Close)
    TIME-WAIT   <-- <SEQ=300><ACK=101><CTL=FIN,ACK>  <-- LAST-ACK

5.  TIME-WAIT   --> <SEQ=101><ACK=301><CTL=ACK>      --> CLOSED

6.  (2 MSL)
    CLOSED
```

- TIME_WAIT & CLOSE_WAIT
  - 서버 (위 그림에서 Peer A) 측에서 `close` 하고 클라이언트 \*\*\*\*(위 그림에서 Peer B) 측에서 ACK 를 보내기 전 프로세스를 종료해 버릴 경우 서버 측에서 답장을 받지 못했기 때문에 `TIME_WAIT` 상태에 걸리게 된다. 프로그램 상에서 `TIME-WAIT` 상태를 처리하기 어렵고, 재 연결을 위해서는 2MSL (Maximum Segment Lifetime) 만큼 기다려야 하므로 서버 측의 `close` 는 신중하게 사용해야 한다.
  - 어쩔 수 없는 경우 `shutdown` 을 이용해 socket 의 write 부터 닫는다.

### 2.3.0 CLOSE_WAIT & TIME_WAIT

- CLOSE 절차 중 특정 상태에서 pending 되면 정상적인 연결이 이루어지지 못하는 문제가 생길 수 있다.

1. CLOSE_WAIT
   - passive close 하는 경우, active close 한 상대편 peer 가 보낸 `FIN` 을 받고 `close` 하기 전 상태이다.
   - `CLOSE_WAIT` 은 정상적인 `close` 요청으로 처리하는 `FIN_WAIT` 나 `TIME_WAIT` 과는 다르게 일정 시간이 지나도 사라지지 않으므로 프로세스의 종료나 네트워크 재시작으로 해결해야 한다.
   - Brilliant Server 에서는 kqueue TIMER 이벤트를 이용하여 일정 시간이 지나면 명시적으로 `close` 요청을 보내는 방법으로 해결하였다.
     ```cpp
     if (events[i].filter == EVFILT_TIMER) {
             ClearConnectionResources(events[i].ident);
     }
     ```
     TIMER event 의 `ident` 와 바인딩 된 소켓 의 `fd` 를 동일하게 설정하여 `ClearConnectionResources` 에서 `close` 를 호출하고 자원정리한다.
2. TIME_WAIT
   - active close 후 상대편의 `FIN`을 기다리는 상태이다. 2MSL 이 지난 후에 `CLOSED` 상태가 되어야 다시 해당 포트에 바인딩 할 수 있다.
   - `TIME_WAIT` 이 없고 바로 연결이 닫히는 경우 문제되는 상황
     1. `close` 되는 소켓의 send 버퍼에 아직 보내지 않은 데이터가 남아있을 수 있다.
     2. 상대편 peer 에서 보내고 아직 도달하지 못한 데이터가 있을 수 있다. 상대편은 아직 ACK 을 받지 못했기 때문에 계속 재전송을 시도하거나 데이터 손실이 일어난다.
     3. 상대편 peer 가 `LAST_ACK`상태에서 보낸 `FIN` 을 받을 수 없으므로 응답도 줄 수 없다. 상대편은 아직 `ACK` 을 받지 못했기 때문에 계속 재전송을 시도하거나 데이터 손실이 일어난다.
   - `TIME_WAIT` 이 남아있어도 새 `bind` 를 하고 싶은 경우
     `setsockopt` 를 이용할 수 있다.
     1. `SO_LINGER` 를 사용한 방법
        - 일반적인 경우 `close` 이후에 위 경우들이 모두 처리되지만 `l_linger` 를 매우 작게 설정하여 `SO_LINGER` 를 사용하면 데이터 손실 뿐만 아니라 `RST` 컨트롤 비트를 이용하여 소켓을 종료하므로 상대편에서 `Connection reset by peer` 오류를 발생시킬 수 있다.
     2. `SO_REUSEADDR` 을 사용한 방법
        - `TIME_WAIT` 상태에서도 새로운 socket 을 같은 주소에 `bind` 할 수 있게 한다.
     - Brilliant Server 에서의 해결법
       [1.4.0 SO_REUSEADDR vs SO_LINGER](#140-so_reuseaddr-vs-so_linger)

---

# 3 I/O 다중 처리

## 3.0 I/O Multiplexing 이란

I/O Multiplexing 은 하나의 `event loop` 에서 여러개의 `I/O events` 를 처리하는 방식이다.

각 I/O 는 `non-block` 으로 이루어지며 I/O 작업이 일어나는 `FD` 들을 감시하는 시스템 콜(`select`, `poll`, `epoll`, `kqueue` … ) 을 활용하여 `FD` 에 `event` 의 발생 여부를 감시하고, 만약 이벤트가 있다면 적절한 작업을 해야한다.

`kqueue` 를 예로 들면, `non-block I/O` 를 호출 한 뒤 `kevent` 로 `block` 을 시키고, `FD` 에 발생한 `event` 가 있는지 확인한다. `kevent` 는 하나의 `FD` 뿐만 아니라, 여러개의 `FD` 에 대한 `event` 를 감지할 수 있어서 여러의 I/O 를 한 프로세스에서 관리할 수 있게 된다.

### 3.0.1 NON-BLOCKING I/O

기존 `read/write` 호출은 프로세스를 `block` 시키고 I/O 작업이 완료되기를 기다리지만, `non-block` I/O 는 `read/write` 호출시 `read/write`가 가능하다면 작업이 수행되고, 그렇지 않다면 `-1` 을 반환하며 `errno`가`EAGAIN|EWOULDBLOCK` 로 설정된다.

![non-blocking I/O 설명](https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Fk.kakaocdn.net%2Fdn%2FC3Sie%2Fbtry2TzbVIY%2FhovPcmvjCQQj31nE3Ue7wK%2Fimg.png)

> 출처 : [https://ecsimsw.tistory.com/entry/Web-server-with-socket-API](https://ecsimsw.tistory.com/entry/Web-server-with-socket-API)

## 3.1 kqueue & kevent

### 3.1.1 kqueue 와 kevent 란

- `kqueue` 와 `kevent` 는 kernel event notification mechanism 이며 각각 kernel queue, kernel event 를 뜻한다.

	```c
	#include <sys/types.h>
	#include <sys/event.h>
	#include <sys/time.h>

	int kqueue(void);

	int kevent(int kq, const struct kevent *changelist, int nchanges,
		struct kevent *eventlist, int nevents, const struct timespec *timeout);

	struct kevent {
	       uintptr_t       ident;          /* identifier for this event */
	       int16_t         filter;         /* filter for event */
	       uint16_t        flags;          /* general flags */
	       uint32_t        fflags;         /* filter-specific flags */
	       intptr_t        data;           /* filter-specific data */
	       void            *udata;         /* opaque user data identifier */
	};

	EV_SET(&kev, ident, filter, flags, fflags, data, udata);

	```

- `kqueue()` 시스템 콜은 새로운 `kqueue` `FD` 를 반환한다. 이 `FD` 는 filters 라고 하는 kernel code 의 결과를 기반으로 kernel event 가 발생하거나 조건을 충족하면, 사용자에게 알려주는 일반적인 방법을 제공한다.
  ![kqueue fd status](/assets/kqueue-fd-status.png)
- `kevent` 구조체는 (`ident`, `filter`, `udata(optional)` ) 튜플로 식별되며 `kevent` 구조체에 해당 튜플에 대해 알림을 받을 조건을 지정한다. I/O event의 경우 `ident` 로 FD 가 들어가고, `filter` 에 `EVFILT_READ, EVFILT_WRITE` 값을 넣어서 `read/write` 이벤트를 등록 할 수 있다.

	```cpp
	void HttpServer::InitKqueue(void) {
	  kq_ = kqueue(); // kqueue 생성
	  if (kq_ == -1) {
	    PRINT_ERROR("HttpServer : kqueue failed : " << strerror(errno));
	    exit(EXIT_FAILURE);
	  }
		// kevent 구조체 동적 할당
	  struct kevent* sock_ev =
	      new (std::nothrow) struct kevent[passive_sockets_.size()];
	  if (sock_ev == NULL) {
	    PRINT_ERROR("HttpServer : failed to allocate memory");
	    exit(EXIT_FAILURE);
	  }
	  int i = 0;
	  for (ListenerMap::const_iterator it = passive_sockets_.begin();
	       it != passive_sockets_.end(); ++it) {
			// kevent 구조체 배열 초기화 (ident: fd)
	    EV_SET(&sock_ev[i++], it->first, EVFILT_READ, EV_ADD, 0, 0, NULL);
	  }
		// kevent에 changlist, nchanges를 인자로 넘겨 이벤트 등록
	  if (kevent(kq_, sock_ev, passive_sockets_.size(), NULL, 0, NULL) == -1) {
	    PRINT_ERROR("HttpServer : failed to listen : " << strerror(errno));
	    exit(EXIT_FAILURE);
	  }
	  delete[] sock_ev;
	}
	```

- `kevent()` 함수는 `changelist` 에 감시할 `kevent 구조체`의 포인터를 받아 이벤트를 등록한다.

	```cpp
	while (true) {
		// 이벤트가 발생할 때 까지 block
	  int number_of_events = kevent(kq_, NULL, 0, events, MAX_EVENTS, NULL);
	  if (number_of_events == -1) {
	    PRINT_ERROR("HttpServer : kevent failed : " << strerror(errno));
	  }
	  for (int i = 0; i < number_of_events; ++i) {
	    if (events[i].filter == EVFILT_READ) {
				/* READ 이벤트 발생, read 작업 수행하기 */
	    } else if (events[i].filter == EVFILT_WRITE) {
	      /* Write 이벤트 발생, write 작업 수행하기 */
	    }
	  }
	}
	```

- `eventlist` 에는 이벤트 발생시 이벤트 데이터를 받아올 `kevent 구조체`의 포인터를 받고, 이벤트 발생시 발생한 이벤트의 개수가 반환되고, `eventlist` 에 넣은 `kevent 구조체` 에 데이터가 담겨온다.
- I/O 의 경우 kevent 구조체의 ident 를 FD 로 넘기고, filter 에 EVFILT_READ|WRITE 를 주면 다음과 같은 경우에 이벤트가 발생한다.
  - READ 의 경우 FD 에 읽을 수 있는 데이터가 있을 때
  - WRITE 의 경우 FD 에 데이터를 쓸 수 있을 때
- 이벤트가 발생한 경우 적절한 READ / WRITE 호출을 해주면, non-block I/O 임에도 적절하게 I/O 를 처리 할 수 있다.

### 3.1.2 kqueue 를 선택한 이유

`kqueue` vs `select` vs `poll`

- `select` 작동 방식

  ```cpp
  #include <sys/select.h>

  int select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds,
  fd_set *restrict errorfds, struct timeval *restrict timeout);
  ```

  - `select()` 는 인자로 감시할 `fd` 의 개수(`nfds`)를 받는다.
  - `select()` 호출 시 (0 ~ `nfds-1`)개의 배열을 순회하며 이벤트를 탐지한다. $O(nfds)$
  - 이벤트 발생시 데이터가 변경된 파일의 개수가 반환되어, 배열을 다시 순회하며 어떤 `fd` 에서 이벤트가 발생했는지 찾아야 한다.
  - `nfds` 가 1024 를 넘을 수 없다.
  - `fd` 마다 `bitmasking` 을 활용하여 `3bit` 만으로 한 `fd` 의 상태를 추적할 수 있다.

- `poll` 작동 방식

  ```cpp
  #include <poll.h>

  int poll(struct pollfd fds[], nfds_t nfds, int timeout);
  ```

  - `poll()` 은 `fd` 와 이벤트 정보가 담긴 `pollfd` 배열과, 실제로 감시하는 `fd` 의 수인 `nfds` 를 인자로 받는다.
  - `select()` 에선 `nfds` 사이즈의 배열을 순회하며 이벤트가 발생한 `fd` 를 찾아야 했지만, `poll()` 실제로 감시하는 `fd` 의 개수만큼 순회를 할 수 있다. $O(fd\_count)$
  - 감시 가능한 `fd` 의 수가 무제한이나, 한 구조체당 `64bit` 의 크기를 가져 많은 이벤트를 다룰 경우 `select()` 보다 성능이 떨어질 수 있다.

- `select(), poll()` 의 문제점
  - `select(), poll()` 은 호출 할 때 마다, 전체 `fd` 배열을 인자로 넘겨야 하며, 이 배열이 `user-space` 에서 `kernel-space` 로 복사될때 상당한 오버헤드가 존재한다. (95% 는 불필요한 복사)
  - `kernel` 에서 이벤트가 발생하면, `kernel-space` 에서는 이미 이벤트가 발생한 `fd` 를 아는데도 불구하고 `user-space` 에서 발생한 이벤트를 찾기 위해 배열을 순회해야 한다.
- `kqueue(), kevent()` 의 장점
  - `kevent` 는 `kernel` 에서 실제로 이벤트가 발생한 `fd` `list` 만 반환하여, `application` 에서 이벤트를 바로 추적할 수 있다.
  - I/O event 뿐만 아니라 process event, signal event, timer event 등을 등록 할 수 있다.
- `kqueue(), kevent()` 의 단점
  - FreeBSD 계열에 한정된 시스템 콜 이라서 호환성이 좋지 않다. 도커를 활용하여 리눅스에서 실행하고 싶었는데 실패했다.

## 3.2 최적화

### 3.2.0 writev 와 send

- 기존엔 `send` 를 이용하여 `response` 를 보냈으나 `response` 의 `header` 와 `content` 가 분리되어 있는 상황에서 `send` 를 사용하기 위해선 `content` 의 불필요한 복사가 일어나는 문제가 있었다.

	```c
	#include <sys/uio.h>
	ssize_t writev(int fildes, const struct iovec *iov, int iovcnt);
	struct iovec {
	   char   *iov_base;  /* Base address. */
	   size_t iov_len;    /* Length. */
	};
	```

- `writev` 를 사용하면, `header` 와 `content` 가 다른 버퍼에 있더라도, `iovec` 구조체에 `header` 와 `content` 의 주소를 넘겨주면, 하나의 버퍼로 `write` 하는 것과 같은 효과가 있다. 따라서 불필요한 복사도 일어나지 않고, `write` 시스템 콜도 줄일 수 있다.

### 3.2.1 Low water mark(SO_SNDLOWAT) 와 writev

- 다수의 클라이언트가 정의한 `BUFFER_SIZE` 보다 큰 파일을 요청 했을 때, `content` 가 `content-length` 보다 적게 전송되는 문제가 있었다. 이를 `setsocketopt` 함수로 `socket` 에 `SO_SNDLOWAT` 옵션을 줘서 해결했다.
  - [1.4 socket 설정](#1.4-socket-설정)
  ```c
  setsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, &buf_size, sizeof(int))
  ```
- `SO_SNDLOWAT` 옵션은 `non-block socket` 에서 `socket buffer` 에 `output` 을 위한 `buf_size` 만큼의 `bytes` 가 비어있거나, 한번에 모든 데이터를 `socket buffer` 에 `write` 할 수 있을 때 (데이터의 크기가 `buf_size` 보다 작을 때) `send()` 가 가능해지며, 그렇지 않으면 `send()` 가 아무 데이터도 전송하지 않고 에러가 발생한다.
- `kevent` 의 `EVFILT_WRITE` 에 `SO_SNDLOWAT` 옵션이 적용된 `socket` 을 등록하게 되면, 해당 `socket` 에 `buf_size` 만큼 `socket buffer` 에 `write` 할 수 있을때 이벤트가 발생하게 된다.
- `buf_size` 를 `SEND_BUFFER_SIZE` 인 `32kb` 로 정의 했으나, 서버가 너무 많은 요청을 받는 상태에서 여전히 `content` 가 덜 전송되는 문제가 발생하여 `SEND_BUFFER_SIZE` 의 1.5배로 설정하여 어느정도 해결했다.

---

# 4 HTTP

## 4.0 HTTP general

- HTTP 메시지는 아래의 섹션들로 이뤄져있다.
  - control data (request line / response line)
  - header table
  - octet-stream content
  - trailer table
- HTTP 메시지의 시작과 끝을 framing 이라고 하고, framing 은 아래와 같은 형식으로 결정된다.
  - body 가 없는 경우
    - (시작) control data
    - header
    - `CRLF` `CRLF` (끝)
  - body 가 있는 경우
    - (시작) control data
    - header (`Content-Length: (positive integer)`/`Transfer-Encoding: chunked`)
    - `CRLF` `CRLF`
    - `Content-Length` 길이 만큼의 octet-stream bytes content / chunked 메시지가 끝날 때까지 (끝)
  - body length 결정
    1. 1xx, 204 OR 304 status code 를 갖는 응답은 header fields + CRLF + CRLF 로 끝난다.
    2. `Transfer-Encoding` & `Content-Length` 둘 다 있는 메시지가 수신된다면 `Transfer-Encoding` 이 `Content-Length` 의 값을 override 한다. 이런 메시지는 [request smuggling](https://www.rfc-editor.org/rfc/rfc9112#request.smuggling)/ [response splitting](https://www.rfc-editor.org/rfc/rfc9112#response.splitting) 의 시도일 수 있고 에러로 처리되어야한다.
    3. `Transfer-Encoding` header field 가 있고
       1. `chunked` 가 마지막 encoding 일 때, 메시지 body length 는 transfer coding 이 끝났다고 알려줄 때까지 읽으면서 결정한다. (`chunked-size` 에 0 이 올 때까지 읽으라는 말인 것 같다.)
       2. 응답 메시지의 경우, `chunked` 가 마지막 encoding 이 아닐 때, 서버가 연결을 끊을 때까지 읽는다.
       3. 요청 메시지의 경우, `chunked` 가 마지막 encoding 이 아닐 때, Bad Request (400) & 연결을 끊는다.
    4. `Transfer-Encoding` 이 없고, `Content-Length` 가 유효하지 않으면, 메시지 framing 이 유효하지 않다 (a 경우 외에). 서버는 Bad Request (400) & 연결을 끊는다.
       1. `Content-Length: 400,400,400,400` (같은 숫자 & ‘,’ separated list 이면 해당 반복되는 숫자로 지정)
    5. `Transfer-Encoding` 이 없고, `Content-Length` 가 유효한 경우, `Content-Length` 에 명시된 수가 body length. `Content-Length` 에 명시된 수 만큼의 octets 가 수신되기 전 만약 송신자가 연결을 끊거나 수신자가 time out 되면 해당 메시지는 미완성으로 (incomplete) 으로 보고 연결을 끊어야한다 (MUST).
    6. 요청 메시지 & 위의 어떤 케이스에도 해당하지 않으면 body length 는 0.
    7. 응답 메시지 & 위의 어떤 케이스에도 해당하지 않으면 서버가 응답을 close 하기 전에 보낸 만큼이 body length.
    - 연결이 끝나서 메시지가 끝난 건지, 네트워크 에러로 연결이 끊겼는지 판별하는게 어렵기 때문에 서버는 encoding 혹은 length 명시를 꼭 해줘야한다 (SHOULD).
      - close-delimiting 은 HTTP/1.0 과의 하위 호환성을 위해 지원한다.
      - 요청 메시지는 절대 close-delimiting 을 하지 않는다. 항상 `Content-Length` / `Transfer-Encoding` 으로 body 의 끝을 알려준다 (MUST).
    - 요청 메시지에 body 가 있는데 `Content-Length` 가 없는 경우 서버는 Length Required (411) 로 응답할 수 있다 (MAY).
    - 요청 메시지에 `Transfer-Encoding` 이 있는데 `chunked` 이외의 coding 이 적용됐고, 메시지 길이를 알 수 있다면 클라이언트는 `chunked` 를 사용하는 것보다 유효한 `Content-Length` 를 명시하는 걸 우선해야한다 (SHOULD). 서버들이 `chunked` coding 을 해석할 수 있어도 Length Required (411) 로 응답할 수도 있기 때문이다.

## 4.1 HTTP/1.1

### 4.1.0 Host 헤더 필드로 name-based 라우팅 지원

- 같은 IP + port 안에서 여러개의 Server 호스팅이 가능해졌다. Name-based virtual server 는 요청의 `Host` 헤더 필드 값으로 요청이 향하는 Server 를 라우팅한다.
- 아래와 같이 서버가 설정되어 있을 때, 요청의 `Host` 헤더 필드 값이 ghan 이면 첫번째 Server, yongjule 면 세번째 Server 로 라우팅된다.

  ```c
  server {
  	listen 80
  	server_name ghan
  	...
  }

  server {
  	listen 80
  	server_name jiskim
  	...
  }

  server {
  	listen 80
  	server_name yongjule
  	...
  }
  ```

### 4.1.1 Transfer-Encoding: chunked

- HTTP/1.1 은 `Transfer-Encoding: chunked` 헤더 필드로 content 전체 길이를 모르는 content stream 을 length-delimited buffer 의 연속으로 전송할 수 있게 해준다.

  ```yaml
  chunked-body   = *chunk
  last-chunk
  trailer-section
  CRLF

  chunk          = chunk-size [ chunk-ext ] CRLF
  chunk-data CRLF
  chunk-size     = 1*HEXDIG
  last-chunk     = 1*("0") [ chunk-ext ] CRLF

  chunk-data     = 1*OCTET ; a sequence of chunk-size octets
  ```

- 아래 예시와 같이, 16진수로 명시된 `chunk-size` 다음 줄에 해당 사이즈 만큼의 octet stream 이 따른다. `chunk-size` 0 으로 transfer coding 의 끝을 알리고, `trailer section` 이 이어질 수도 있고, CRLF 로 body 의 끝을 표시한다.

  ```yaml
  HTTP/1.1 200 OK
  Content-Type: text/plain
  Transfer-Encoding: chunked

  4\r\n
  ghan\r\n
  6\r\n
  jiskim\r\n
  8\r\n
  yongjule\r\n
  0\r\n
  \r\n
  ```

- 수신자는 `chunked` transfer coding 을 파싱할 수 있어야한다 (MUST).
- 수신자는 매우 큰 `chunk-size` 가 올 수 있다는 걸 예상하고, overflow, precision loss 와 같은 파싱 에러를 방지해야한다 (MUST). (HTTP/1.1 은 제한을 설정하지 않았다.)
- `chunked` 에 parameter 가 붙으면 에러 (SHOULD).
- `chunk-ext` 가 아래의 포맷으로 `chunk-size` 옆에 나올 수도 있다. 수신자는 이해할 수 없는 (unrecognized) chunk extension 을 무시해야한다 (MUST). (다 이해할 수 없기로 하자… 문법 체크만 하자…)

  ```yaml
  chunk-ext      = *( BWS ";" BWS chunk-ext-name
  [ BWS "=" BWS chunk-ext-val ] )

  chunk-ext-name = token
  chunk-ext-val  = token / quoted-string
  ```

- `chunked` coding 을 없애는 수신자는 trailer fields 를 유지할지 없앨지 정할 수 있다 (MAY). 유지한다면, header field 와는 다른 구조체에 저장해야한다.

### 4.1.1 Connection Management

- Persistence
  - HTTP/1.1 은 기본적으로 persistent connection 을 사용하며, 하나의 연결 에서 여러개의 요청과 응답을 주고 받을 수 있다. 이는 가장 최근에 받은 `protocol version` 이나 `Connection` 헤더 필드에 의해 결정되는데, 아래와 같은 우선순위에 의하여 결정된다.
    1. `Connection` 헤더 필드에 `close` 가 있다면, 현재 응답 이후에 connection 은 더이상 지속되지 않는다. else;
    2. 받은 요청이 `HTTP/1.1` 이면 연결은 계속 지속된다. else;
    3. 받은 요청이 `HTTP/1.0` 이고 `Connection` 헤더 필드가 `keep-alive` 면 연결은 계속 지속된다.
    4. 현재 연결 이후에 연결은 닫힌다.
- pipelining
  - `persistent connection` 을 지원하는 클라이언트는 요청을 `pipeline` (응답을 기다리지 않고 여러개의 요청을 보내는것)을 할 수 있다. 서버는 `pipeline` 으로 오는 요청이 모두 안전한 `method` 를 가진 요청이라면, 이를 병렬적으로 처리할 수 있지만 각 요청에 상응하는 응답을 받은 것과 같은 순서로 보내줘야 한다.

---

# 5 Common Gateway Interface

## 5.0 Common Gateway Interface 란

- Common Gateway Interface (CGI) 는 HTTP Server 가 플랫폼에 상관 없이 외부 프로그램을 실행시킬 수 있게 해주는 인터페이스다.
- [RFC 3875](https://www.rfc-editor.org/rfc/rfc3875) 에 규격이 정의되어 있다.

## 5.1 CGI 와 Server 의 소통

- Server 는 CGI script 를 `execve` 로 호출하며 아래의 meta-variable 들을 env 로 설정해준다 (`execve` 의 세번째 인자로 전달).
  ```c
  "AUTH_TYPE=" // 서버가 사용자 인증에 사용하는 방법.
  "CONTENT_LENGTH=" // 요청 메시지 body 의 크기를 십진수로 표현
  "CONTENT_TYPE=" // 요청 메시지 body 의 Internet Media Type
  "GATEWAY_INTERFACE=CGI/1.1" // server 가 적용하는 CGI version
  "PATH_INFO=" // Script-URI 의 script-path 에 이어 나오는 부분
  "PATH_TRANSLATED=" // PATH_INFO 기반으로 해당 resource 의 local 절대 경로
  "QUERY_STRING=" // URL-encoded 검색/parameter 문자열
  "REMOTE_ADDR=" // 요청을 보내는 client 의 네트워크 주소
  "REMOTE_HOST=" // 요청을 보내는 client 의 도메인
  "REMOTE_IDENT="
  "REMOTE_USER=" // 사용자 인증을 위해 client 가 제공하는 사용자 식별자
  "REQUEST_METHOD=" // 요청 method
  "SCRIPT_NAME=" // 요청에서 cgi script 까지의 경로
  "SERVER_NAME=" // 서버 명
  "SERVER_PORT=" // 서버 포트
  "SERVER_PROTOCOL=" // 서버 프로토콜
  "SERVER_SOFTWARE=" // 서버 프로그램 명
  ```
- CGI 는 표준 출력에 CGI 응답을 작성하고 `EOF` 로 응답의 끝을 Server 에게 알린다.
- CGI 의 응답을 받기 위해 Server 는 `execve` 전에 `pipe` 를 열어 CGI 로 부터 응답 받을 채널을 준비한다.

## 5.2 CGI 응답

- CGI 응답은 header 필드와 body 로 구성된다.
- header 필드는 CGI-field (`Content-Type` | `Location` | `Status`) + HTTP field (선택) + extension field (선택) 로 이루어진다.
- body 는 `EOF` 까지 쓰인 octet-stream 이다.
- CGI 응답은 Document Response, Local Redirection Response, Client Redirection Response, Client Redirection Response with Document 로 나뉜다.

### 5.2.0 Document Response

```bash
document-response = Content-Type [ Status ] *other-field NL
                    response-body
```

- 일반적인 문서 반환, `Content-Type` 필드는 필수, `Status` 는 없으면 200 으로 간주한다.

### 5.2.1 Redirection Response

- `Location` 필드가 필수이다.
  ```bash
  Location        = local-Location | client-Location
  client-Location = "Location:" fragment-URI NL
  local-Location  = "Location:" local-pathquery NL
  fragment-URI    = absoluteURI [ "#" fragment ]
  fragment        = *uric
  local-pathquery = abs-path [ "?" query-string ]
  abs-path        = "/" path-segments
  path-segments   = segment *( "/" segment )
  segment         = *pchar
  pchar           = unreserved | escaped | extra
  extra           = ":" | "@" | "&" | "=" | "+" | "$" | ","
  ```

1. Local Redirect

   ```bash
   local-redir-response = local-Location NL
   ```

   - CGI 는 `Location` 필드 값에 리다이렉트 할 경로를 적어준다.
   - Server 는 그 경로로 요청이 온 것처럼 요청을 처리한다.

2. Client Redirect

   ```bash
   client-redir-response = client-Location *extension-field NL
   ```

   - CGI 는 `Location` 필드 값에 Client 가 리다이렉트 해야할 경로를 적어준다.
   - Server 는 `302 Found` 상태 코드와 함께 `Location` 헤더 필드를 Client 에게 전달하며 Client 가 리다이렉션을 수행할 수 있게 한다.

3. Client Redirect with Document

   ```
   client-redirdoc-response = client-Location Status Content-Type
                                *other-field NL response-body
   ```

   - CGI 는 `Location` 필드 값에 Client 가 리다이렉트 해야할 경로를 적어주며, `Content-Type` 에는 반환하는 문서의 미디어 타입을 알려준다.
   - Server 는 `302 Found` 상태 코드와 함께 `Location` 헤더 필드를 Client 에게 전달하며 Client 가 리다이렉션을 수행할 수 있게 한다.

### 5.2.1 Server 의 CGI 응답 처리

- CGI 의 응답을 받은 Server 는 CGI 가 보낸 header 필드들이 의미하는 바가 Server 가 설정하는 응답 헤더 필드값과 상충된다면 어떤 값을 넣을지 결정해야한다.
- Server 는 CGI 의 응답이 HTTP 규격에 맞는지 점검하고 Client 에게 전달해야한다.

---

# 6 “서버는 죽지 않아!”

- Server 는 어떤 상황에도 꺼지지 않으며
- 최대한 자원 할당은 생성자에서, 해제는 소멸자에서 처리한다. (RAII)
- BrilliantServer 는 heap use after free/double free/pointer being freed was not allocated 를 피하기 위해 할당 해제 후 포인터 `NULL` 로 설정한다.
- 한정된 `fd` 테이블이 재사용되기 때문에 socket, file, pipe 의 I/O event 시 사용하는 `fd` 가 전혀 다른 device 를 가리킬 수 있다. 이를 방지하기 위해 재사용 되는 `fd` 변수들은 `close` 이후 -1 로 설정한다.

---

# 7 References

## 7.0 Socket 통신

- [Sockets (The GNU C Library)](https://www.gnu.org/software/libc/manual/html_node/Sockets.html)
- [What is the meaning of SO_REUSEADDR (setsockopt option) - Linux?](https://stackoverflow.com/questions/3229860/what-is-the-meaning-of-so-reuseaddr-setsockopt-option-linux)
- [When is TCP option SO_LINGER (0) required?](https://stackoverflow.com/questions/3757289/when-is-tcp-option-so-linger-0-required)

## 7.1 TCP Connection

- [Transmission Control Protocol (TCP)](https://www.rfc-editor.org/rfc/rfc9293#name-close-call)

- [practical tcp serises the tcp window](https://www.networkdatapedia.com/post/2016/10/27/practical-tcp-series-the-tcp-window)

- [CLOSE_WAIT & TIME_WAIT 최종 분석](https://tech.kakao.com/2016/04/21/closewait-timewait/)

- [close() 이후 대기 시간이 있어야 하는 이유](https://stackoverflow.com/questions/71975992/what-really-is-the-linger-time-that-can-be-set-with-so-linger-on-sockets/71975993#71975993)

## 7.2 I/O Multiplexing

- [Multiplexing - Wikipedia](https://en.wikipedia.org/wiki/Multiplexing)

- [네이버 클라우드 플랫폼 (NAVER Cloud Platform) : 네이버 블로그](https://blog.naver.com/n_cloudplatform/222189669084)

- [FreeBSD kqueue & kevent](https://people.freebsd.org/~jlemon/papers/kqueue_freenix.pdf)

## 7.3 SO_SNDLOWAT

- [What's the purpose of the socket option SO_SNDLOWAT](https://stackoverflow.com/questions/8245937/whats-the-purpose-of-the-socket-option-so-sndlowat)

## 7.4 HTTP

- [RFC9110](https://httpwg.org/specs/rfc9110.html)

- [RFC9112](https://httpwg.org/specs/rfc9112.html)

## 7.5 CGI

- [RFC 3875: The Common Gateway Interface (CGI) Version 1.1](https://www.rfc-editor.org/rfc/rfc3875)
