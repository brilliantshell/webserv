# Coding Standard

- 이 프로젝트는 [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) 를 따른다.
- `-Wall -Werror -Wextra -std=c++98` 컴파일 플래그를 통과해야한다.
- C++98 로 작성 돼야한다.

# Coding Practice

- 이 프로젝트는 Test-Driven Development 의 원칙을 따른다.
- [GoogleTest](https://google.github.io/googletest/) 테스팅 프레임워크를 사용한다.

# Git Branch Convention

- github flow 사용
- 브랜치 설명
    - `main` : 모든 브랜치가 merge 되는 브랜치
    - `role` : 객체의 역할 단위로 생성해서 개발
    - `etc` : github action script, 문서 update 시 생성
- Reference : [Github Flow Document](https://docs.github.com/en/get-started/quickstart/github-flow)

# Commit Convention

- 한글로 작성

```
# (gitmoji) <타입> : <제목><이슈번호>

##### 제목은 이슈 번호와 함께 최대 50 글자까지 한 줄로 입력 ############## -> |

# 본문은 위에 작성
######## 본문은 한 줄에 최대 72 글자까지만 입력 ########################### -> |

# --- COMMIT END ---
# <타입> 리스트
#   ✨(:sparkles:) feat    : 기능 (새로운 기능)
#   🐛(:bug:) fix     : 버그 (버그 수정)
#   ♻(:recycle:) refactor : 리팩토링
#   💄(:lipstick:) style   : UI 스타일 변경
#   📝(:memo:) docs    : 문서 (문서 추가, 수정, 삭제)
#   ✅(:white_check_mark:) test    : 테스트 (테스트 코드 추가, 수정, 삭제: 비즈니스 로직에 변경 없음)
#   🔨(:hammer:) chore   : 기타 변경사항 (빌드 스크립트 수정 등)
# ------------------
#     제목은 명령문으로
#     제목 끝에 마침표(.) 금지
#     제목과 본문을 한 줄 띄워 분리하기
#     본문은 "어떻게" 보다 "무엇을", "왜"를 설명한다.
#     본문은 한 줄을 작성하고 . 마침표를 찍어서 분리한다.
# ------------------
```

- Reference
    
    [커밋 템플릿 · innovationacademy-kr/slabs-saver Wiki](https://github.com/innovationacademy-kr/slabs-saver/wiki/%EC%BB%A4%EB%B0%8B-%ED%85%9C%ED%94%8C%EB%A6%BF)
    

# File Header

[vscode doxygen extension](https://marketplace.visualstudio.com/items?itemName=cschlosser.doxdocgen)을 이용하여 아래와 같이 설정함.

```json
"doxdocgen.generic.authorTag": "@author ghan, jiskim, yongjule",
"doxdocgen.file.fileOrder": [
	"file",
	"author",
	"brief",
	"date",
	"empty",
	"copyright",
],
```
