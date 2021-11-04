cd external
cp -r SDL2-2.0.14-patch/* SDL2-2.0.14
cd SDL2-2.0.14
chmod u+x configure
./configure
make -j 2
cd ..
rm -rf SDL2_image-2.0.5
unzip SDL2_image-2.0.5.zip
cd SDL2_image-2.0.5
cp SDL_image.h ../SDL2-2.0.14/include
./configure
make -j 2
cd ..
rm -rf SDL2_ttf-2.0.15
unzip SDL2_ttf-2.0.15
cd SDL2_ttf-2.0.15
cp SDL_ttf.h ../SDL2-2.0.14/include
./configure
make -j 2
cd ..
cd ..
mkdir bin
g++  -shared -o libZED.so   -D_DEBUG  -DNO_OPENGL  -g    -DLINUX    -std=c++17 -fpermissive  -Wno-attributes -Wno-format -Wno-conversion-null -Wno-format-extra-args -Wno-unused-result -Wno-deprecated-declarations -Wno-trigraphs   -I"src" -I"external"   `pkg-config --cflags gtk+-3.0`    external/SDL_rotozoom/SDL_rotozoom.cpp   src/core.cpp  src/csv.cpp src/http_client.cpp  src/log.cpp   src/renderer_sdl2.cpp  src/renderer.cpp   src/server.cpp  src/server_tcp.cpp  src/server_udp.cpp  src/socket.cpp  src/socket_tcp.cpp src/socket_udp.cpp  src/timer.cpp   -ldl  -lpthread  "external/SDL2-2.0.14/build/.libs/libSDL2.a"  "external/SDL2_image-2.0.5/.libs/libSDL2_image.a"    "external/SDL2_ttf-2.0.15/.libs/libSDL2_ttf.a"  -lfreetype
g++  -o "ExampleTCPEchoServer.o" -std=c++17  -I"src"  examples/ExampleTCPEchoServer.cpp -L"." -lZED    `pkg-config --libs gtk+-3.0` -L/opt/vc/lib -lbcm_host
g++  -o "ExampleLatencyServer.o" -std=c++17  -I"src"  examples/ExampleLatencyServer.cpp -L"." -lZED    `pkg-config --libs gtk+-3.0` -L/opt/vc/lib -lbcm_host -lpthread

Works under RPI3:
Shared
g++   -shared -o libZED.so   -D_DEBUG    -g    -DLINUX    -std=c++17 -fpermissive  -Wno-attributes -Wno-format -Wno-conversion-null -Wno-format-extra-args -Wno-unused-result -Wno-deprecated-declarations -Wno-trigraphs   -I"src" -I"external"   `pkg-config --cflags gtk+-3.0`    external/SDL_rotozoom/SDL_rotozoom.cpp   src/core.cpp  src/csv.cpp src/http_client.cpp  src/log.cpp  src/blob.cpp src/sha512.cpp src/crc32.cpp   src/renderer_sdl2.cpp  src/renderer.cpp   src/renderer_opengl.cpp src/opengl_extensions.cpp     src/file.cpp  src/server.cpp  src/server_tcp.cpp  src/server_udp.cpp  src/socket.cpp  src/socket_tcp.cpp src/socket_udp.cpp  src/timer.cpp   -ldl  -lpthread -lSDL2 -lSDL2_ttf -lfreetype  -lGL

Static
g++  -c       -D_DEBUG    -g    -DLINUX    -std=c++17 -fpermissive  -Wno-attributes -Wno-format -Wno-conversion-null -Wno-format-extra-args -Wno-unused-result -Wno-deprecated-declarations -Wno-trigraphs   -I"src" -I"external"   `pkg-config --cflags gtk+-3.0`    external/SDL_rotozoom/SDL_rotozoom.cpp   src/core.cpp  src/csv.cpp src/http_client.cpp  src/log.cpp  src/blob.cpp src/sha512.cpp src/crc32.cpp   src/renderer_sdl2.cpp  src/renderer.cpp   src/renderer_opengl.cpp src/opengl_extensions.cpp     src/file.cpp  src/image.cpp  src/png.cpp  src/server.cpp  src/server_tcp.cpp  src/server_udp.cpp  src/socket.cpp  src/socket_tcp.cpp src/socket_udp.cpp  src/timer.cpp; ar rcs libZEDd.a *.o
g++  -c   -O3 -s    -DLINUX    -std=c++17 -fpermissive  -Wno-attributes -Wno-format -Wno-conversion-null -Wno-format-extra-args -Wno-unused-result -Wno-deprecated-declarations -Wno-trigraphs   -I"src" -I"external"   `pkg-config --cflags gtk+-3.0`    external/SDL_rotozoom/SDL_rotozoom.cpp   src/core.cpp  src/csv.cpp src/http_client.cpp  src/log.cpp  src/blob.cpp src/sha512.cpp src/crc32.cpp   src/renderer_sdl2.cpp  src/renderer.cpp   src/renderer_opengl.cpp src/opengl_extensions.cpp     src/file.cpp  src/image.cpp  src/png.cpp  src/server.cpp  src/server_tcp.cpp  src/server_udp.cpp  src/socket.cpp  src/socket_tcp.cpp src/socket_udp.cpp  src/timer.cpp; ar rcs libZED.a *.o