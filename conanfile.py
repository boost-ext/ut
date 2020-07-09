from conans import ConanFile, CMake


class UT(ConanFile):
    name = "UT"
    version = "latest"
    url = "https://github.com/boost-ext/ut"
    license = "Boost"
    description = "[Boost::ext].UT - C++17/20 μ(micro)/Unit Testing Framework"
    settings = "os", "compiler", "arch", "build_type"
    exports_sources = "include/*"
    no_copy_source = True

    def package(self):
        self.copy("*.hpp")
