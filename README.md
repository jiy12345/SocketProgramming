# SocketProgramming

- [v1.1 간단한 1대 1 에코 서버](#v1-1-간단한-1대-1-에코-서버) 
  - [주요 기능](#1-0-주요-기능)
  - [문제점](#1-0-개선할-점)
  - [실행 예시](#1-0-실행-예시)
- [v1.1 간단한 1대 다 에코 서버](#v1-1-간단한-1대-다-에코-서버) 
  - [주요 기능](#1-1-주요-기능)
  - [문제점](#1-1-개선할-점)
  - [실행 예시](#1-1-실행-예시)

## v1 1 간단한 1대 1 에코 서버
### 1 0 주요 기능
- 클라이언트 측 소켓
  - 블로킹 소켓 활용하여 서버와 연결 시도(connect)
  - 논 블로킹 소켓 활용하여 서버에 사용자로부터 입력받은 데이터 보내고 받기(send, recv)
- 서버 측 소켓
  - 리스닝 소켓
    - 클라이언트의 통신 요청을 받아들이기 위한 소켓 구현(bind, listen)
    - 클라이언트 통신 요청 받아들여 클라이언트와 통신하기위한 소켓 생성(accept)
  - 클라이언트와의 소통을 위한 소켓
    - 논 블로킹 소켓 활용하여 클라이언트의 데이터 받고 받은 데이터 다시 보내기
### 1 0 개선할 점
- 클라이언트 측
  - 클라이언트 측에서 논 블로킹 소켓을 활용하여 데이터를 보내고 받기는 했지만, fgets라는 입력 함수가 사용자로부터 입력이 들어올 때까지 제어를 반환하지 않으므로 실행이 막히게 된다. 따라서 논 블로킹 소켓을 활용하는 의미가 없어지게 된다.(1.1에서 해결)
  - 위 문제와 연결되는 문제인데, 데이터를 보내는 것과 받는 것이 같은 실행 흐름에서 이루어지다보니 사용자가 입력을 하여 데이터를 보내기 전까지 데이터를 받을 수 없다.(1.1에서 해결)
- 서버 측
  - accept를 하나만 받고 있으므로, 여러 클라이언트로부터의 통신 요청을 받는 것이 불가능하다.(1.1에서 해결)
### 1 0 실행 예시
![ResultImage1.0](https://github.com/jiy12345/SocketProgramming/blob/main/Images/Result%20Images/ResultImage1.0.png)
## v1 1 간단한 1대 다 에코 서버
### 1 1 주요 기능
- 클라이언트 측 소켓
  - 사용자 입력을 받고, 서버로 보내는 부분을 새로운 스레드 생성하여 구성
    - 따라서 1.0 버전과 달리 사용자가 입력했는지 여부와 상관 없이 서버로부터 입력을 바로 받아볼 수 있음
- 서버 측 소켓
  - 생성한 클라이언트 소켓별로 스레드 생성하여 각각의 연결마다 개별적인 스레드에서 send와 recv를 진행하도록 하기
    - 따라서 여러 클라이언트와 동시에 통신하는것이 가능해짐
### 1 1 개선할 점
- 공통 사항
  - 현재는 데이터를 보내고 받는 것만 가능하고, 데이터의 성질에 따른 다른 처리를 하는 것이 불가능함
- 서버측 소켓
  - 클라이언트 연결과 스레드가 1대 1 대응되도록 구현하였으므로, 지나치게 많은 스레드가 생성될 수 있음
### 1 1 실행 예시
![ResultImage1.1](https://github.com/jiy12345/SocketProgramming/blob/main/Images/Result%20Images/ResultImage2.0.png)