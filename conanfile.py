from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake

class libnest2dConan(ConanFile):
    name = "libnest2d"
    version = "4.10.0"
    license = "LGPL-3.0"
    author = "Ultimaker B.V."
    url = "https://github.com/Ultimaker/libnest2d"
    description = "2D irregular bin packaging and nesting library written in modern C++"
    topics = ("conan", "cura", "prusaslicer", "nesting", "c++", "bin packaging")
    settings = "os", "compiler", "build_type", "arch"
    exports = "LICENSE.txt"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "tests": [True, False],
        "header_only": [True, False],
        "geometries": ["clipper", "boost", "eigen"],
        "optimizer": ["nlopt", "optimlib"],
        "threading": ["std", "tbb", "omp", "none"],
        "python_version": "ANY"
    }
    default_options = {
        "shared": True,
        "tests": False,
        "fPIC": True,
        "header_only": False,
        "geometries": "clipper",
        "optimizer": "nlopt",
        "threading": "std",
        "python_version": "3.9"
    }
    scm = {
        "type": "git",
        "subfolder": ".",
        "url": "auto",
        "revision": "auto"
    }

    # TODO: Move lib naming logic to python_requires project
    _ext = None

    @property
    def ext(self):
        if self._ext:
            return self._ext
        self._ext = "lib" if self.settings.os == "Windows" else "a"
        if self.options.shared:
            if self.settings.os == "Windows":
                self._ext = "dll"
            elif self.settings.os == "Macos":
                self._ext = "dylib"
            else:
                self._ext = "so"
        return self._ext

    _lib_name = None

    @property
    def lib_name(self):
        if self._lib_name:
            return self._lib_name
        ext = "d" if self.settings.build_type == "Debug" else ""
        ext += "" if self.settings.os == "Windows" else f".{self.ext}"
        self._lib_name = f"{self.name}_{self.options.geometries}_{self.options.optimizer}{ext}"
        return self._lib_name

    def configure(self):
        if self.options.shared or self.settings.compiler == "Visual Studio":
            del self.options.fPIC
        if self.options.geometries == "clipper":
            self.options["clipper"].shared = self.options.shared
            self.options["boost"].header_only = True
            self.options["boost"].python_version = self.options.python_version
            self.options["boost"].shared = self.options.shared
        if self.options.optimizer == "nlopt":
            self.options["nlopt"].shared = self.options.shared

    def build_requirements(self):
        self.build_requires("cmake/[>=3.16.2]")
        if self.options.tests:
            self.build_requires("catch2/[>=2.13.6]", force_host_context=True)

    def requirements(self):
        if self.options.geometries == "clipper":
            self.requires("clipper/[>=6.4.2]")
            self.requires("boost/1.70.0")
        elif self.options.geometries == "eigen":
            self.requires("eigen/[>=3.3.7]")
        if self.options.optimizer == "nlopt":
            self.requires("nlopt/[>=2.7.0]")

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            tools.check_min_cppstd(self, 17)

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate()

        tc = CMakeToolchain(self)

        # FIXME: This shouldn't be necessary (maybe a bug in Conan????)
        if self.settings.compiler == "Visual Studio":
            tc.blocks["generic_system"].values["generator_platform"] = None
            tc.blocks["generic_system"].values["toolset"] = None

        tc.variables["LIBNEST2D_HEADER_ONLY"] = self.options.header_only
        if self.options.header_only:
            tc.variables["BUILD_SHARED_LIBS"] = False
        else:
            tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.variables["LIBNEST2D_BUILD_UNITTESTS"] = self.options.tests
        tc.variables["LIBNEST2D_GEOMETRIES"] = self.options.geometries
        tc.variables["LIBNEST2D_OPTIMIZER"] = self.options.optimizer
        tc.variables["LIBNEST2D_THREADING"] = self.options.threading
        tc.generate()

    _cmake = None

    def configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        if self.in_local_cache:
            self.cpp_info.libdirs = ["lib"]
        else:
            self.cpp_info.libdirs = [f"cmake-build-{self.settings.build_type}".lower()]
        self.cpp_info.libs = [self.lib_name]
        self.cpp_info.defines.append(f"LIBNEST2D_GEOMETRIES_{self.options.geometries}")
        self.cpp_info.defines.append(f"LIBNEST2D_OPTIMIZERS_{self.options.optimizer}")
        self.cpp_info.defines.append(f"LIBNEST2D_THREADING_{self.options.threading}")
        self.cpp_info.names["cmake_find_package"] = self.name
        self.cpp_info.names["cmake_find_package_multi"] = self.name
        if self.settings.os in ["Linux", "FreeBSD", "Macos"]:
            self.cpp_info.system_libs.append("pthread")
