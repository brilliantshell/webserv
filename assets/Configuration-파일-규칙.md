# Configuration 파일 규칙

- 파일명은 `*.config`

- 서버가 여러개 있을 수 있다
    - [(0 < `port` ≤ 65535)](https://www.gnu.org/software/libc/manual/html_node/Ports.html)
        - 00800과 같이 앞에 0이 들어가는 경우도 허용한다.
    - 서버명을 설정할 수도 안 할 수도 있다.
    - 포트는 반드시 설정해야한다.
- `host:port` 의 첫 번째 서버는 해당 `host:port` 의 기본 서버가 된다.
(다른 서버에 속하지 않는 모든 쿼리에 응답)
    - `host != server_name`
- 에러 페이지 세팅 (없으면 기본 에러 페이지)

- `location` 세팅
    - 서버 블록 당 최소 한 개 이상 존재해야 함.
    - 해당 `location` 에 허용된 HTTP 메소드 나열
    - HTTP redirection
    - directory / file 경로 정의
    - client body size 제한 (없으면 INT_MAX)
    - `autoindex`  (directory listing) on/off
    - directory 요청 시 응답할 default file 설정
    - 특정 확장자의 cgi 실행
    - 파일 업로드 가능, 파일 저장 위치 설정
- `server`와 `location` 와 같이 여는 괄호 `{` 가 있는 경우는 키워드와 괄호 사이에 공백 하나만 허용한다.

- `cgi` 블록 세팅

## 암시적 규칙

- `location` 안에 method 가 정의 돼있지 않으면 `GET` 만 허용
- `location` 안에 `root` / `index` 없으면 `location` 경로가 기본 `root` 로 설정, `index.html` 이 기본 `index` 로 설정
- `server_name` 없으면 empty string
- 에러 페이지 없으면 `error.html`
- `location` 안에 `autoindex` 없으면 off

## `.config` file 예시

```
server {
	listen HOST:NUMBER
	server_name HOST
	error 40x.html
	
	location PATH {
		root PATH
		index FILE
		methods GET POST DELETE
		body_max NUMBER
		autoindex BOOLEAN
		upload_path PATH
		redirect_to ROUTE
	}

	cgi .php {
		root PATH	
		methods GET POST
		body_max NUMBER(only POST) 
	}
}
```

[server 디렉티브](https://www.notion.so/72d71470fd774b76bb9de66b934eea99)

[location 디렉티브](https://www.notion.so/6bd195489bdb4e7d9ed80d0b2dc6fcdd)

[cgi 디렉티브](https://www.notion.so/4ed713d8cd1c419d88e9a7ed3a1efd12)