from conans import ConanFile, CMake, tools
import os

class Pure_Cpp_Test_Conan (ConanFile) :
    settings = "cppstd", "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def build (self):
        cmake = CMake (self)
        cmake.configure()
        cmake.build()

    def test (self):
        os.chdir ("bin")
        self.run (".%stest" % os.sep)

