from conans import ConanFile, CMake
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake


class LibNest2DTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def build_requirements(self):
        self.build_requires("cmake/[>=3.16.2]")

    def requirements(self):
        self.requires(f"libnest2d/4.11.0@ultimaker/testing")

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()
        tc = CMakeToolchain(self)
        if self.settings.compiler == "Visual Studio":
            tc.blocks["generic_system"].values["generator_platform"] = None
            tc.blocks["generic_system"].values["toolset"] = None
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy("*.dll", str(self.settings.build_type), "lib")
        self.copy("*.dll", str(self.settings.build_type), "lib")
        self.copy("*.dylib", str(self.settings.build_type), "lib")

    def test(self):
        pass # only interested in compiling and linking