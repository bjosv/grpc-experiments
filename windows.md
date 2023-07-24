# Build and run on Windows

## Prepare
> Install choco using admin
> see https://chocolatey.org/install
choco install -y nasm

choco install -y msys2
choco install -y bazel


choco install -y --no-progress vcredist140
choco install -y --no-progress visualstudio2019community visualstudio2019-workload-vctools visualstudio2019-workload-nativedesktop visualstudio2019buildtools
choco install -y windows-sdk-10-version-2104-all
path
setx BAZEL_SH "C:\tools\msys64\usr\bin\bash.exe"
setx BAZEL_VC "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC

set BAZEL_WINSDK_FULL_VERSION=10.0.10240.0

net use G: \\<ip>\grpc /persistent:yes
net use X: \\<ip>\grpc-experiments /persistent:yes

> Enable long filename via powershell:
New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force

### Build x64
> Start "x64 Native Tools Command Prompt" via search
or
"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

## Build grpc using CMake (~30min)
> Default install prefix is "C:\Program Files (x86)\grpc"
cd cmake
md build
cd build
cmake -DCMAKE_INSTALL_PREFIX=C:/Users/%USERNAME%/grpc-vm/ -GNinja ../..
cmake --build .
> or to include build of tests, and generate compile commands file
cmake -DCMAKE_INSTALL_PREFIX=C:/Users/%USERNAME%/grpc-vm/ -GNinja -DgRPC_BUILD_TESTS=ON ../..
cmake -DCMAKE_INSTALL_PREFIX=C:/Users/%USERNAME%/grpc-vm/ -GNinja -DgRPC_BUILD_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ../..
cmake -v --build .
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



## Windows other

### Features
https://hahndorf.eu/blog/WindowsFeatureViaCmd.html
```
powershell

# Install feature
Install-WindowsFeature -Name qWave
Uninstall-WindowsFeature -Name qWave

# Get feature status
dism.exe -online -get-features
..
  Feature Name : QWAVE
  State : Enabled
..


get-windowsfeature | select Name | sort name

# Get last exit code
> SHELL
echo %errorlevel%

> POWERSHELL
$LASTEXITCODE
```
## Links
https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170
https://sanoj.in/2020/05/07/working-with-grpc-in-windows.html
