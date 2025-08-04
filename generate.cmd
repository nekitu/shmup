mkdir build
cd ./build
cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -G "Visual Studio 17 2022" -DOpenGL_GL_PREFERENCE=GLVND ../
cd ..