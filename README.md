# eTutor

eTutor is a intelligent tutoring assistant inspired by
[Lucida](http://lucida.ai). The project is released under 
[BSD license](LICENSE), except certain submodules contain their own 
specific licensing information.

This project differs from the original Lucida project in the following 
ways:
- Uses gRPC instead of Thrift.
- Build works on Ubuntu and OSX.
- Improved dependency management.
- Uses Google services.

## Overview

- `site`: web site and back-end service clients.
  Currently, there are 4 categories of back-end services:
  1. "ASR" (automatic speech recognition): We use Google's Speech API.
  2. "IMM" (image matching): We use Google's Vision API.
  3. "QA" (question answering): Based on OpenEphyra.
  4. "DIAL" (dialog): Based on OpenDial.
  
  The site determines which services are needed based on the user input,
  sends requests to them, and returns response to the user.
  
- `deps`: dependencies necessary for compiling eTutor.
  Due to the fact that services share some common dependencies,
  all services should be compiled after these dependencies are installed.
  
- `src`: common code for gRPC service support.

We use autotools for the make system. A lot of people prefer CMake but
this is the devil I know.   

## Initializing the Build

This must be done once. From this directory run:
```
./scripts.autogen.sh
./configure
make init
```

The command `make init` will install all missing dependencies so may 
need root access. We assume your username is in the sudoers list so 
make sure it is. **Do not** run `make init` as root because this will
create a whole bunch of files in the build tree owned by root.

## Building

- From this directory, type: `make`.

- Start all services:
  ```
  make start_all
  ```

  This will spawn a gnu screen for each backend service as well as the 
  web site. To see the list of active screens run `screen -list`. Once
  they all start running,  open your browser and visit `http://localhost:3000/`.
  
- Stop all services:
  ```
  make stop_all
  ```
  
### Java Support

Many of the backend services run on Java. Java source is built using
gradle. The make system runs gradle to create the java backend services
but if you want to build java without make then run `gradle build`. 

#### Build Javadocs

Run `gradle alljavadoc`. The docs will be located at build/docs/. 

#### IntelliJ Integration for Java Code

The root project file [build.gradle](build.gradle) uses the idea plugin.
To create your IDEA project run `gradle idea`. To rebuild the project run
`gradle cleanIdea idea`.
 
### Testing

To execute all tests run `make test`.

## Adding Backend Services

### Java Services Based on External Source

1. Create a new directory under the top level directory. Name it according
   to the service provided. We refer to it as \<service-name\>.
2. Add the build tree for the existing source as a subdir of \<service-name\>.
   Add a build.gradle to this directory.
3. Create a build.gradle file in \<service-name\>. Use the file [qna/build.gradle](qna/build.gradle)
   as a reference.
4. Add the new projects to [settings.gradle](settings.gradle).


For example:
```
qna[+] <service-name=qna>
    |
    +-- build.gradle
    |
    +-- OpenEphyra [+]- build.gradle
    |               |
    |               '-- (external sourcetree)
    |
    +-- src [+] -- main/java/ai/lucida/openephyra [+]
             |                                     |
             |                                     '-- source for qna gRPC service
             |
             '---- test/java/ai/lucida/openephyra [+]
                                                   |
                                                   '-- tests for qna gRPC service
```

### C++ Services Based on Existing Source

TODO:

### New C++ Services

Add your source code to src/main/cpp/\<backend-component-name\>. Add your 
tests to src/test/cpp/\<backend-component-name\>