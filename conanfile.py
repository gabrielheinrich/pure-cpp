from conans import ConanFile, CMake, tools

class Pure_Cpp_Conan (ConanFile) :
    name = "pure-cpp"
    version = "0.0.1"
    exports_sources = "*"
    url = "https://github.com/gabrielheinrich/pure-cpp"
    description = "C++ Library for gradually-typed and data-oriented functional programming"
    license = "https://github.com/gabrielheinrich/pure-cpp/blob/master/LICENSE"
    author = "Gabriel Heinrich <gabriel@pure-cpp.org"
    generators = "cmake"
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build(target="check")

    def package (self):
        self.copy ("LICENSE")
        self.copy ("*", src="include/pure", dst="include/pure")
        self.copy ("*", src="include/immer/immer", dst="include/immer")

    def package_info (self):
        self.info.header_only()
