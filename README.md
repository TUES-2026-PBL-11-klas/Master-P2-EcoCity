# Master-P2-EcoCity
The repo for the second project of team Master

## Проблемът
Хората рядко осъзнават какво всъщност биха означавли дадени екологични проетки като ефект върху средата. Това не е от нежелание, а просто защото човек трудно преценява тези неща без личен опит.

## Цел на играта
EcoCity е игра от тип city builder, направена за училищен проект по "Единен проект за 11-ти клас". Играчът започва с малък град и неговата цел е да го развие. Играта се фокусира над екологичния аспект на един град. Ресурсите са вода, енергия, CO2(емисии) и пари, като населението на града също се следи. Целта на играта е да се постигне баланс между ресурсите, както и да се менежират нивата на емисии. Ако емисиите стигнат твърде високи нива или някой от ресурсите стигне 0, играта свършва. Резултатът зависи от населението. Играта няма някакъв край, целта на играча е да постигне колкото може по-висок резултат. Проектът цели да покаже ясно как различни екологични решения влияят на околната среда, както и да развие екологично-насоченото мислене в хората.

## Механики
Играта работи чрез петиции.На играчът биват предложени петиции с различни ефекти върху ресурсите. Някои носят пари, други подобряват добива на енергия и т.н. Всяка петиция има време за изпълнение(построяване на сградата), както и цена и изисквани ресурси, като всичко това е в описанието й.

## Архитектура
<img width="512" height="403" alt="unnamed" src="https://github.com/user-attachments/assets/f7a69515-1bfe-47e9-a080-c4ffb2bdb141" />


## Технологии
| Компонент     | Технология    |
| ------------- |:-------------:|
| Backend       | C++           |
| UI            | Rust          |
| Database      | MongoDB       |

## Структура

```
├── game_api/
│   └── v1/
│       └── api_types.proto          # Дефиниция на protobuf, който отговаря за
│                                    # обмяна между game engine и UI
│
├── game-engine/                     # C++ backend — runs the game simulation
│   │
│   ├── main.cpp                     # Главната част, навързва всички други компоненти
│   │
│   ├── Makefile                     # Build-ва програмата
│   │
│   ├── Logger.hpp                   # Структурира логове
│   │
│   ├── Tracer.hpp                   # Дефинира trace-ове
│   │
│   ├── api_types.pb.h / .pb.cc      # Код, генериран от  api_types.proto.
│   │
│   ├── domain/                      # Сърцевината на кода, само класове за информация
|   |                                # не прилагат логика
│   │
│   ├── services/                    # Класовете, които прилагат логика върху 
|   |                                #класовете от domain
│   ├── network/
│   │   └── SocketServer.hpp / .cpp  # Отговаря за комуникация между game engine и UI.
│   │
# at construction if the port cannot be bound.
│   │
│   ├── persistence/
│   │   └── MongoGameRepository.hpp / .cpp  # Запазва и зарежда играта
|   |
│   ├── exceptions/                  # Custom exception типове
│   │
│   └── tests/                       # Тесотве за backend - а
│
├── UI/                              # Rust frontend
│   
|
├── deployment/
│   └── build.sh                     # Shell script, който построява backend и UI
│
└── eco_city_tmp_ui.py               # Временен UI
```

## Tests
Backend tests live in `game-engine/tests`.

The project now has dedicated `Makefile` targets for Google Test:

```powershell
cd C:\Users\katie\Documents\Master-P2-EcoCity-DEV\game-engine
mingw32-make test
mingw32-make run-tests
mingw32-make test-resource
mingw32-make test-petition
mingw32-make test-game-service
```

### Google Test setup on Windows
The default test setup expects:

- `TEST_CXX=C:/msys64/ucrt64/bin/g++.exe`
- `GTEST_ROOT=C:/msys64/ucrt64`

If Google Test is installed somewhere else, you can override both values:

```powershell
mingw32-make test GTEST_ROOT=C:/path/to/gtest TEST_CXX=C:/path/to/g++.exe
```

If you use MSYS2/UCRT64, the expected package is:

```powershell
pacman -S mingw-w64-ucrt-x86_64-gtest
```

### Extra dependency for `GameServiceTest`
`GameServiceTest` now uses the real `GameService`, which depends on generated protobuf types from `api_types.pb.*`.

The default `Makefile` expects protobuf headers and libraries under:

- `PROTOBUF_ROOT=C:/vcpkg/installed/x64-mingw-dynamic`

If your protobuf installation is elsewhere, override it explicitly:

```powershell
mingw32-make test-game-service PROTOBUF_ROOT=C:/path/to/protobuf
```

If you use MSYS2/UCRT64 for protobuf too, install the development package and point `PROTOBUF_ROOT` to that toolchain root.
