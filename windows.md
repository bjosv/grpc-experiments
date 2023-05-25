# Build and run on Windows

## Prepare
> Install choco using admin, see https://chocolatey.org/install
choco install nasm
choco install bazel
(choco install ninja)

net use G: \\10.10.10.78\grpc /persistent:yes
net use X: \\10.10.10.78\grpc2 /persistent:yes

### Build x64
> Start "x64 Native Tools Command Prompt" via search
or
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

## Build grpc using CMake (~30min)
> Default install prefix is "C:\Program Files (x86)\grpc"
cd cmake
md build
cd build
cmake -DCMAKE_INSTALL_PREFIX=C:/Users/%USERNAME%/grpc-vm/ -GNinja ../..
cmake --build .
cmake --build . --target install

Run/Search for "Edit environment variables for your account"
Add to PATH:  %USERPROFILE%\grpc

## Build examples
cmake -DCMAKE_PREFIX_PATH=C:/Users/%USERNAME%/grpc-vm/ ..
cmake --build .

## Run examples
set GRPC_VERBOSITY=debug
server\Debug\server.exe 127.0.0.1:5001
client\Debug\client.exe 127.0.0.1:5001

client\Debug\client.exe 10.10.10.78:5001
client-dscp\Debug\client-dscp.exe 10.10.10.78:5001


## Build grpc using bazel
choco install bazelisk
bazel build :all

## Other
### Build as x86
> Start "Developer Command Prompt" via search
or run from Command
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
> If grpc was built using x86, give architecture when examples are built with x64:
cmake -AWin32 ..


## Links
https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170
https://sanoj.in/2020/05/07/working-with-grpc-in-windows.html
