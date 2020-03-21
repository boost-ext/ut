from conans import ConanFile, CMake


class BoostUT(ConanFile):
    name = "UT"
    version = "latest"
    url = "https://github.com/boost-experimental/ut"
    license = "Boost"
    description = "[Boost].UT C++17/20 μ(micro)/Unit Testing Framework"
    settings = "os", "compiler", "arch", "build_type"
    exports_sources = "include/*"
    no_copy_source = True

    def package(self):
        self.copy("*.hpp")
