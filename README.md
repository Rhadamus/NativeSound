NativeSound is a rewrite of the sound engine of the Android runtime in Clickteam Fusion 2.5, with the objective of providing better performance and loading times.

**Note:** Currently there are stability issues for MP3 files. Please convert all MP3s to OGG to prevent these issues.

# Installing
Download one of the releases provided. Find RuntimeAndroid.zip in the Data/Runtime/Android folder of your Fusion installation. Drop the contents of NativeSound.zip into
RuntimeAndroid.zip.

# Building
**Note:** This section is for those that wish to build NativeSound from source, for example if you have modified it and want to use your modification, or you want to
contribute code. Otherwise, follow just the installation and ignore this.

Required dependencies:
* [OpenAL Soft](https://github.com/kcat/openal-soft)
* [libogg](https://gitlab.xiph.org/xiph/ogg)
* [libvorbis](https://gitlab.xiph.org/xiph/vorbis)
* [dr_mp3](https://github.com/mackron/dr_libs/tree/master)

Install the Android NDK and CMake 3.22.1 from the Android SDK Manager. My builds use NDK r25. Other versions should work, but note that NDK r26 and newer dropped support for
Android 4.4 KitKat.

In app/src/main/cpp/build, open BuildAll.bat in a text editor, and edit ANDROID_SDK_PATH to the location of the Android SDK on your machine, and NDK_VERSION to the NDK version
you installed.

Create a folder named "lib" in app/src/main/cpp and drop the sources for the dependencies there. libogg should be in a folder named "ogg-master", libvorbis in "vorbis-master",
OpenAL Soft in "openal-soft" and dr_mp3.h should just be in the root of the lib folder. OpenAL Soft needs to built manually before building NativeSound, check their building
instructions to do so. I personally use an edited version of BuildAll.bat to build it. All the other dependencies will be built automatically when NativeSound is built.

Run BuildAll.bat. Put the compiled libopenal.so and libNativeSound.so files in the runtime's jniLibs folder.
