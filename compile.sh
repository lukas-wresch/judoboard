mkdir bin/tournaments

#Compile yaml-cpp
mkdir dep
cd dep
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp
mkdir build
cmake .
make
cd ../..

#Compile ZED
cd ZED
g++  -c   -O3 -s         -DLINUX -DNO_OPENGLES    -std=c++17 -fpermissive                 -Wno-attributes -Wno-format -Wno-conversion-null -Wno-format-extra-args -Wno-unused-result -Wno-deprecated-declarations -Wno-trigraphs   -I"src" -I"external"   `pkg-config --cflags gtk+-3.0`    external/SDL_rotozoom/SDL_rotozoom.cpp   src/core.cpp  src/csv.cpp src/http_client.cpp  src/log.cpp  src/blob.cpp src/sha256.cpp src/sha512.cpp src/crc32.cpp   src/renderer_sdl2.cpp  src/renderer.cpp   src/renderer_opengl.cpp src/opengl_extensions.cpp    src/file.cpp  src/image.cpp  src/png.cpp  src/server.cpp  src/server_tcp.cpp  src/server_udp.cpp  src/socket.cpp  src/socket_tcp.cpp src/socket_udp.cpp  src/timer.cpp
ar rcs libZED.a *.o; rm *.o
g++  -c    -g   -D_DEBUG -DLINUX -DNO_OPENGLES    -std=c++17 -fpermissive                 -Wno-attributes -Wno-format -Wno-conversion-null -Wno-format-extra-args -Wno-unused-result -Wno-deprecated-declarations -Wno-trigraphs   -I"src" -I"external"   `pkg-config --cflags gtk+-3.0`    external/SDL_rotozoom/SDL_rotozoom.cpp   src/core.cpp  src/csv.cpp src/http_client.cpp  src/log.cpp  src/blob.cpp src/sha256.cpp src/sha512.cpp src/crc32.cpp   src/renderer_sdl2.cpp  src/renderer.cpp   src/renderer_opengl.cpp src/opengl_extensions.cpp    src/file.cpp  src/image.cpp  src/png.cpp  src/server.cpp  src/server_tcp.cpp  src/server_udp.cpp  src/socket.cpp  src/socket_tcp.cpp src/socket_udp.cpp  src/timer.cpp
ar rcs libZEDd.a *.o; rm *.o
cd ..


#Compile release build
g++  -c -fpermissive  -O3  -s -std=c++17 -DLINUX                                          -Wno-attributes -Wno-format -Wno-conversion-null -Wno-format-extra-args -Wno-unused-result -Wno-deprecated-declarations -Wno-trigraphs       -I"."  -I"src"    -I"external"  `pkg-config --cflags gtk+-3.0`  "src/app.cpp"  "src/app_ajax.cpp"   "src/account.cpp"  "src/age_group.cpp" "src/club.cpp"  "src/dm4.cpp"  "src/database.cpp"   "src/judoka.cpp"   "src/club.cpp" "src/customtable.cpp" src/dmf.cpp "src/md5.cpp"  "src/mat.cpp"   "src/remote_mat.cpp"   "src/match.cpp"  "src/matchtable.cpp"  "src/matchlog.cpp"  "src/timer.cpp"   "src/tournament.cpp"   "src/remote_tournament.cpp"   "src/weightclass.cpp" src/weightclass_generator.cpp  "src/rule_set.cpp" "src/standing_data.cpp" "src/id.cpp" "src/error.cpp"    "src/localizer.cpp"  "src/window_linux.cpp"     "src/HttpServer/HttpServer.cpp" "external/mongoose/mongoose.cpp"     -ldl  -lpthread    `pkg-config --libs gtk+-3.0`     -ldl  -lpthread -lSDL2 -lSDL2_ttf -lfreetype  -lGL  -lpng  "dep/yaml-cpp/libyaml-cpp.a"  -no-pie
ar rcs libjudoboard.a *.o; rm *.o

g++    -fpermissive  -O3  -s  -std=c++17 -DLINUX             -o "bin/judoboard"           -Wno-attributes -Wno-format -Wno-conversion-null -Wno-format-extra-args -Wno-unused-result -Wno-deprecated-declarations -Wno-trigraphs       -I"."   -I"src"   -I"external"  `pkg-config --cflags gtk+-3.0`  "src/main.cpp"                                                                                                                                                                                            "libjudoboard.a"  "dep/yaml-cpp/libyaml-cpp.a"   "ZED/libZED.a"  `pkg-config --libs gtk+-3.0`      -ldl  -lpthread -lSDL2 -lSDL2_ttf   -lfreetype  -lGL  -lEGL -lpng    -no-pie

#Compile debug build
g++  -c -fpermissive  -g      -std=c++17  -D_DEBUG  -DLINUX                               -Wno-attributes -Wno-format -Wno-conversion-null -Wno-format-extra-args -Wno-unused-result -Wno-deprecated-declarations -Wno-trigraphs       -I"."   -I"src"   -I"external"  `pkg-config --cflags gtk+-3.0`  "src/app.cpp"  "src/app_ajax.cpp"   "src/account.cpp"  "src/age_group.cpp" "src/club.cpp"  "src/dm4.cpp"  "src/database.cpp"   "src/judoka.cpp"   "src/club.cpp" "src/customtable.cpp" src/dmf.cpp "src/md5.cpp"   "src/mat.cpp"   "src/remote_mat.cpp"   "src/match.cpp"  "src/matchtable.cpp"  "src/matchlog.cpp"  "src/timer.cpp"   "src/tournament.cpp"  "src/remote_tournament.cpp"   "src/weightclass.cpp"  src/weightclass_generator.cpp  "src/rule_set.cpp" "src/standing_data.cpp" "src/id.cpp" "src/error.cpp"    "src/localizer.cpp"  "src/window_linux.cpp"     "src/HttpServer/HttpServer.cpp" "external/mongoose/mongoose.cpp"      -ldl  -lpthread    `pkg-config --libs gtk+-3.0`   -ldl  -lpthread -lSDL2 -lSDL2_ttf -lfreetype  -lGL  -lpng  "dep/yaml-cpp/libyaml-cpp.a"  -no-pie
ar rcs libjudoboardd.a *.o; rm *.o

g++    -fpermissive -g        -std=c++17  -D_DEBUG  -DLINUX  -o "bin/judoboard_debug"       -Wno-attributes -Wno-format -Wno-conversion-null -Wno-format-extra-args -Wno-unused-result -Wno-deprecated-declarations -Wno-trigraphs       -I"."   -I"src"   -I"external"  `pkg-config --cflags gtk+-3.0`  "src/main.cpp"                                                                                                                                                                                          "libjudoboardd.a" "dep/yaml-cpp/libyaml-cpp.a"   "ZED/libZEDd.a"  `pkg-config --libs gtk+-3.0`     -ldl  -lpthread -lSDL2 -lSDL2_ttf   -lfreetype  -lGL  -lEGL -lpng    -no-pie

#Compile unit tests
g++    -fpermissive  -O3  -s  -std=c++17 -DLINUX           -o "bin/judoboard_test"          -Wno-attributes -Wno-format -Wno-conversion-null -Wno-format-extra-args -Wno-unused-result -Wno-deprecated-declarations -Wno-trigraphs       -I"."   -I"src"   -I"external"  `pkg-config --cflags gtk+-3.0`   tests/account.cpp  tests/app.cpp  "tests/ajax.cpp"   tests/age_group.cpp tests/database.cpp  tests/dm4.cpp  tests/judoka.cpp  tests/club.cpp  tests/id.cpp  tests/mat.cpp  tests/dmf.cpp tests/md5.cpp  tests/remote_mat.cpp   tests/match.cpp  tests/timer.cpp  tests/tournament.cpp                                           "libjudoboard.a"    -ldl  -lpthread   "ZED/libZED.a"   `pkg-config --libs gtk+-3.0`   -lSDL2 -lSDL2_ttf   -lfreetype  -lGL  -lEGL -lpng  "dep/yaml-cpp/libyaml-cpp.a"    -lgtest   -lgtest_main
g++    -fpermissive -g        -std=c++17 -D_DEBUG -DLINUX  -o "bin/judoboard_test_debug"    -Wno-attributes -Wno-format                                                                                                                  -I"."   -I"src"   -I"external"  `pkg-config --cflags gtk+-3.0`   tests/account.cpp  tests/app.cpp  "tests/ajax.cpp"   tests/age_group.cpp tests/database.cpp  tests/dm4.cpp  tests/judoka.cpp  tests/club.cpp  tests/id.cpp  tests/mat.cpp  tests/dmf.cpp tests/md5.cpp  tests/remote_mat.cpp   tests/match.cpp  tests/timer.cpp  tests/tournament.cpp                                           "libjudoboardd.a"   -ldl  -lpthread   "ZED/libZEDd.a"  `pkg-config --libs gtk+-3.0`   -lSDL2 -lSDL2_ttf   -lfreetype  -lGL  -lEGL -lpng  "dep/yaml-cpp/libyaml-cpp.a"    -lgtest   -lgtest_main