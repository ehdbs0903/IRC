# IRC(Internet Relay Chat)


## 프로그램 설명
이 프로젝트의 목적은 IRC(Internet Relay Chat) 서버의 기능을 재현하는 것입니다.

본 프로젝트는 다음 IRC RFC 문서를 따릅니다:
- [RFC1459](https://datatracker.ietf.org/doc/html/rfc1459)
- [Modern IRC Client Protocol](https://modern.ircdocs.horse/)

주요 기능은 다음과 같습니다:
- I/O 멀티플렉싱을 활용하여 단일 스레드에서 여러 클라이언트를 관리하는 서버를 구현합니다.
- 서버는 클라이언트로부터 받은 메시지를 현재 서버에 연결된 채널 및 클라이언트에 전달합니다.

## 실행 방법
### 서버 실행하기
```bash
make
./ircserv <port> <password>
```

### 서버에 클라이언트 연결하기
```
nc <host> <port>
```

구현된 커맨드에 대한 설명과 사용법은 [Command.md](./Command.md) 파일에 정리되어 있습니다.
