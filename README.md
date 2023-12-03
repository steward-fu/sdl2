# SDL2 Library with Virtual GPU Support for Miyoo Mini (Plus) and TRIMUI SMART
 - [Miyoo Mini (Plus)](#miyoo-mini-plus)
   - Introduction
   - Build from Scratch
     - How to prepare the build environment (Docker)
     - How to build all libraries
     - How to pack the release build
     - How to delete the build environment (Docker)
 - [TRIMUI SMART](#trimui-smart)
   - Introduction
   - Build from Scratch
     - How to prepare the build environment (Docker)
     - How to build all libraries
     - How to pack the release build
     - How to delete the build environment (Docker)

&nbsp;

## Miyoo Mini (Plus)
![image](images/mmiyoo/mm.jpg) ![image](images/mmiyoo/mmp.jpg)  

### Introduction
TBD

&nbsp;

### Build from Scratch
#### How to prepare the build environment (Docker)
```
$ sudo docker build -t mmiyoo .
```

#### How to build all libraries (SDL2 and virtual GPU)
```
$ sudo docker run -it --rm -v $(pwd):/nds_miyoo mmiyoo /bin/bash
$ make clean
$ make config
$ make
```

#### How to build the SDL2 library only
```
$ sudo docker run -it --rm -v $(pwd):/nds_miyoo mmiyoo /bin/bash
$ make clean
$ make config
$ make sdl2
```

#### How to build the virtual GPU (swiftshader) library only
```
$ sudo docker run -it --rm -v $(pwd):/nds_miyoo mmiyoo /bin/bash
$ make clean
$ make config
$ make gpu
```

#### How to delete the build environment (Docker)
```
$ sudo docker image rm mmiyoo
```

&nbsp;

## TRIMUI SMART
![image](images/trimui/trimui.jpg)  

### Introduction
TBD

&nbsp;

### Build from Scratch
#### How to prepare the build environment (Docker)
```
$ sudo docker build -t mmiyoo .
```

#### How to build all libraries (SDL2 and virtual GPU)
```
$ sudo docker run -it --rm -v $(pwd):/nds_miyoo mmiyoo /bin/bash
$ make clean
$ make config MOD=trimui
$ make
```

#### How to build the SDL2 library only
```
$ sudo docker run -it --rm -v $(pwd):/nds_miyoo mmiyoo /bin/bash
$ make clean
$ make config MOD=trimui
$ make sdl2 MOD=trimui
```

#### How to build the virtual GPU (swiftshader) library only
```
$ sudo docker run -it --rm -v $(pwd):/nds_miyoo mmiyoo /bin/bash
$ make clean
$ make config MOD=trimui
$ make gpu MOD=trimui
```

#### How to delete the build environment (Docker)
```
$ sudo docker image rm mmiyoo
```
