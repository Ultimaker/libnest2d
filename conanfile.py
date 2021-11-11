from typing import Optional

from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake
from conan.tools.layout import cmake_layout
from conan.tools.files.packager import AutoPackager
from conans.model.layout import Folders, Infos

required_conan_version = ">=1.42"


class libnest2dConan(ConanFile):
    name = "libnest2d"
    version = "4.11.0"
    license = "LGPL-3.0"
    author = "Ultimaker B.V."
    url = "https://github.com/Ultimaker/libnest2d"
    description = "2D irregular bin packaging and nesting library written in modern C++"
    topics = ("conan", "cura", "prusaslicer", "nesting", "c++", "bin packaging")
    settings = "os", "compiler", "build_type", "arch"
    revision_mode = "scm"
    build_policy = "missing"
    exports = "LICENSE.txt"
    exports_sources = "*", "!cmake-build-*", "!tmp", "test_package"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "tests": [True, False],
        "header_only": [True, False],
        "geometries": ["clipper", "boost", "eigen"],
        "optimizer": ["nlopt", "optimlib"],
        "threading": ["std", "tbb", "omp", "none"]
    }
    default_options = {
        "shared": True,
        "tests": False,
        "fPIC": True,
        "header_only": False,
        "geometries": "clipper",
        "optimizer": "nlopt",
        "threading": "std"
    }
    scm = {
        "type": "git",
        "subfolder": ".",
        "url": "auto",
        "revision": "auto"
    }

    def configure(self):
        if self.options.shared or self.settings.compiler == "Visual Studio":
            del self.options.fPIC
        if self.options.geometries == "clipper":
            self.options["clipper"].shared = self.options.shared
            self.options["boost"].header_only = True
            self.options["boost"].shared = self.options.shared
        if self.options.optimizer == "nlopt":
            self.options["nlopt"].shared = self.options.shared

    def build_requirements(self):
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

    def layout(self):
        cmake_layout(self)
        self.cpp.package.libs = tools.collect_libs(self)
        self.cpp.package.defines = [f"LIBNEST2D_GEOMETRIES_{self.options.geometries}",
                                    f"LIBNEST2D_OPTIMIZERS_{self.options.optimizer}",
                                    f"LIBNEST2D_THREADING_{self.options.threading}"]
        self.cpp.package.system_libs = []
        if self.settings.os in ["Linux", "FreeBSD", "Macos"]:
            self.cpp.package.system_libs.append("pthread")

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

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()

    def package(self):
        packager = AutoPackager(self)
        packager.run()
