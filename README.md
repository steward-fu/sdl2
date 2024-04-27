# SDL2 Library with Virtual GPU Support for Miyoo Mini (Plus)
 - [Miyoo Mini (Plus)](#miyoo-mini-plus)
   - Build from Scratch
     - How to prepare the build environment (Docker)
     - How to build all libraries
     - How to pack the release build
     - How to delete the build environment (Docker)

&nbsp;

## Miyoo Mini (Plus)
![image](images/mmiyoo/mm.jpg) ![image](images/mmiyoo/mmp.jpg)  

&nbsp;

### Build from Scratch
#### How to prepare the build environment (Docker)
```
$ sudo docker build -t mmiyoo .
```

#### How to build all libraries (SDL2 and virtual GPU)
```
$ sudo docker run -it --rm -v $(pwd):/sdl2_miyoo mmiyoo /bin/bash
# cd /sdl2_miyoo
# rm -rf swiftshader/build/*
# make cfg
# make
```

#### How to build the SDL2 library only
```
$ sudo docker run -it --rm -v $(pwd):/sdl2_miyoo mmiyoo /bin/bash
# cd /sdl2_miyoo
# make clean
# make cfg
# make sdl2
```

#### How to build the virtual GPU (swiftshader) library only
```
$ sudo docker run -it --rm -v $(pwd):/sdl2_miyoo mmiyoo /bin/bash
# cd /sdl2_miyoo
# rm -rf swiftshader/build/*
# make clean
# make cfg
# make gpu
```

#### How to build the example
```
$ sudo docker run -it --rm -v $(pwd):/sdl2_miyoo mmiyoo /bin/bash
# make example
```

#### How to delete the build environment (Docker)
```
$ sudo docker image rm mmiyoo
```
