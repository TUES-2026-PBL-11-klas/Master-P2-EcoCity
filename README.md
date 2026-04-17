# Master-P2-EcoCity
The repo for the second project of team Master

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
