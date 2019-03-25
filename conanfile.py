from conans import ConanFile, CMake, tools

class Pure_Cpp_Conan (ConanFile) :
    name = "pure-cpp"
    version = "0.0.1"
    generators = "cmake"
    exports_sources = "*"
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build(target="check")

    def package (self):
        self.copy ("*", src="include/pure", dst="include/pure")
        self.copy ("*", src="include/immer/immer", dst="include/immer")

    def package_info (self):
        self.info.header_only()
