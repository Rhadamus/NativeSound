@echo off

set PATH=%PATH%;D:\AndroidSdk\cmake\3.22.1\bin;D:\AndroidSdk\ndk\25.2.9519653\toolchains\llvm\prebuilt\windows-x86_64\bin
set TOOLCHAIN_FILE=D:\AndroidSdk\ndk\25.2.9519653\build\cmake\android.toolchain.cmake

set COMMON_OPTIONS=-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17 -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE% -GNinja
::set COMMON_OPTIONS=-DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_STANDARD=17 -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_FILE% -GNinja

if not exist armeabi-v7a\ (
	mkdir armeabi-v7a
)
if not exist x86\ (
	mkdir x86
)
if not exist arm64-v8a\ (
	mkdir arm64-v8a
)
if not exist x86_64 (
	mkdir x86_64
)

cmake -S .. -B armeabi-v7a\ -DANDROID_ABI=armeabi-v7a -DANDROID_PLATFORM=android-19 %COMMON_OPTIONS%
cmake -S .. -B x86\ -DANDROID_ABI=x86 -DANDROID_PLATFORM=android-19 %COMMON_OPTIONS%
cmake -S .. -B arm64-v8a\ -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21 %COMMON_OPTIONS%
cmake -S .. -B x86_64\ -DANDROID_ABI=x86_64 -DANDROID_PLATFORM=android-21 %COMMON_OPTIONS%

cmake --build armeabi-v7a\
cmake --build x86\
cmake --build arm64-v8a\
cmake --build x86_64\

llvm-strip --strip-unneeded armeabi-v7a\libNativeSound.so
llvm-strip --strip-unneeded x86\libNativeSound.so
llvm-strip --strip-unneeded arm64-v8a\libNativeSound.so
llvm-strip --strip-unneeded x86_64\libNativeSound.so