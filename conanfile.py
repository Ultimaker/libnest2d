import os

from os import path

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout
from conan.tools.files import AutoPackager, files, collect_libs, copy, update_conandata
from conan.tools.build import check_min_cppstd
from conan.tools.microsoft import check_min_vs, is_msvc
from conan.tools.scm import Version, Git

required_conan_version = ">=2.7.0"


class Nest2DConan(ConanFile):
    name = "nest2d"
    author = "UltiMaker"
    url = "https://github.com/Ultimaker/libnest2d"
    description = "2D irregular bin packaging and nesting library written in modern C++"
    topics = ("conan", "cura", "prusaslicer", "nesting", "c++", "bin packaging")
    settings = "os", "compiler", "build_type", "arch"
    build_policy = "missing"
    package_type = "library"
    implements = ["auto_header_only"]

    python_requires = "npmpackage/[>=1.0.0]"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "header_only": [True, False],
        "geometries": ["clipper", "boost"],
        "optimizer": ["nlopt", "optimlib"],
        "threading": ["std", "tbb", "omp", "none"],
        "with_js_bindings": [True, False]
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "header_only": False,
        "geometries": "clipper",
        "optimizer": "nlopt",
        "threading": "std",
        "with_js_bindings": False
    }

    def set_version(self):
        if not self.version:
            self.version = self.conan_data["version"]

    def export(self):
        git = Git(self)
        update_conandata(self, {"version": self.version, "commit": git.get_commit()})

    @property
    def _min_cppstd(self):
        return 17

    @property
    def _compilers_minimum_version(self):
        return {
            "gcc": "9",
            "clang": "9",
            "apple-clang": "9",
            "msvc": "192",
            "visual_studio": "14",
        }

    def export_sources(self):
        copy(self, "CMakeLists.txt", self.recipe_folder, self.export_sources_folder)
        copy(self, "*", path.join(self.recipe_folder, "src"), path.join(self.export_sources_folder, "src"))
        copy(self, "*", path.join(self.recipe_folder, "include"), path.join(self.export_sources_folder, "include"))
        copy(self, "*", path.join(self.recipe_folder, "tests"), path.join(self.export_sources_folder, "tests"))
        copy(self, "*", path.join(self.recipe_folder, "tools"), path.join(self.export_sources_folder, "tools"))
        copy(self, "*", path.join(self.recipe_folder, "libnest2d_js"),
             os.path.join(self.export_sources_folder, "libnest2d_js"))

    def layout(self):
        cmake_layout(self)
        self.cpp.build.bin = []
        self.cpp.build.bindirs = []
        self.cpp.package.bindirs = ["bin"]
        self.cpp.package.libs = ["nest2d"]
        if self.settings.os == "Emscripten":
            self.cpp.build.bin = ["libnest2d_js.js"]
            self.cpp.package.bin = ["libnest2d_js.js"]
            self.cpp.build.bindirs += ["libnest2d_js"]

        self.cpp.package.includedirs = ["include"]

    def requirements(self):
        self.requires("spdlog/[>=1.14.1]", transitive_headers=True)
        if self.options.geometries == "clipper":
            self.requires("clipper/6.4.2@ultimaker/stable", transitive_headers=True)
        if self.options.geometries == "boost" or self.options.geometries == "clipper":
            self.requires("boost/1.86.0", transitive_headers=True)
        if self.options.optimizer == "nlopt":
            self.requires("nlopt/2.7.1", transitive_headers=True)
        if self.options.optimizer == "optimlib":
            self.requires("armadillo/12.6.4", transitive_headers=True)
        if self.options.threading == "tbb":
            self.requires("tbb/2020.3", transitive_headers=True)
        if self.options.threading == "omp":
            self.requires("llvm-openmp/17.0.6", transitive_headers=True)

    def validate(self):
        if self.settings.compiler.cppstd:
            check_min_cppstd(self, self._min_cppstd)
        check_min_vs(self, 192)  # TODO: remove in Conan 2.0
        if not is_msvc(self):
            minimum_version = self._compilers_minimum_version.get(str(self.settings.compiler), False)
            if minimum_version and Version(self.settings.compiler.version) < minimum_version:
                raise ConanInvalidConfiguration(
                    f"{self.ref} requires C++{self._min_cppstd}, which your compiler does not support."
                )

    def build_requirements(self):
        self.test_requires("standardprojectsettings/[>=0.2.0]")
        if not self.conf.get("tools.build:skip_test", False, check_type=bool):
            self.test_requires("catch2/[>=3.5.2]")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
        self.options["boost"].header_only = True
        if self.options.geometries == "clipper":
            self.options["clipper"].shared = True if self.options.header_only else self.options.shared
        self.options[str(self.options.optimizer)].shared = True if self.options.header_only else self.options.shared
        if self.options.threading == "tbb":
            self.options["tbb"].shared = True if self.options.header_only else self.options.shared
        if self.options.threading == "omp":
            self.options["llvm-openmp"].shared = True if self.options.header_only else self.options.shared

    def generate(self):
        deps = CMakeDeps(self)
        if not self.conf.get("tools.build:skip_test", False, check_type=bool):
            deps.build_context_activated = ["catch2"]
            deps.build_context_build_modules = ["catch2"]
        deps.generate()

        tc = CMakeToolchain(self)
        tc.variables["HEADER_ONLY"] = self.options.header_only
        if not self.options.header_only:
            tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.variables["ENABLE_TESTING"] = not self.conf.get("tools.build:skip_test", False, check_type=bool)
        tc.variables["GEOMETRIES"] = self.options.geometries
        tc.variables["OPTIMIZER"] = self.options.optimizer
        tc.variables["THREADING"] = self.options.threading
        tc.variables["WITH_JS_BINDINGS"] = self.options.get_safe("with_js_bindings", False)

        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()

    def deploy(self):
        copy(self, "libnest2d_js*", src=os.path.join(self.package_folder, "bin"), dst=self.install_folder)
        copy(self, "*", src=os.path.join(self.package_folder, "bin"), dst=self.install_folder)

    def package(self):
        copy(self, pattern="libnest2d_js*", src=os.path.join(self.build_folder, "libnest2d_js"),
             dst=os.path.join(self.package_folder, "bin"))
        copy(self, f"*.d.ts", src=self.build_folder, dst=os.path.join(self.package_folder, "bin"), keep_path = False)
        copy(self, f"*.js", src=self.build_folder, dst=os.path.join(self.package_folder, "bin"), keep_path = False)
        copy(self, f"*.wasm", src=self.build_folder, dst=os.path.join(self.package_folder, "bin"), keep_path = False)
        packager = AutoPackager(self)
        packager.run()

        # Remove the header files from options not used in this package
        if self.options.geometries != "clipper":
            files.rmdir(self, os.path.join(self.package_folder, "include", "libnest2d", "backends", "clipper"))
        if self.options.optimizer != "nlopt":
            files.rmdir(self, os.path.join(self.package_folder, "include", "libnest2d", "optimizers", "nlopt"))
        if self.options.optimizer != "optimlib":
            files.rmdir(self, os.path.join(self.package_folder, "include", "libnest2d", "optimizers", "optimlib"))

    def package_info(self):
        if not self.options.header_only:
            self.cpp_info.libs = collect_libs(self)
        self.cpp_info.defines.append(f"LIBNEST2D_GEOMETRIES_{self.options.geometries}")
        self.cpp_info.defines.append(f"LIBNEST2D_OPTIMIZERS_{self.options.optimizer}")
        self.cpp_info.defines.append(f"LIBNEST2D_THREADING_{self.options.threading}")
        if self.settings.os in ["Linux", "FreeBSD", "Macos"] and self.options.threading == "std":
            self.cpp_info.system_libs.append("pthread")

        # npm package json for Emscripten builds
        if self.settings.os == "Emscripten" or self.options.get_safe("with_js_bindings", False):
            self.python_requires["npmpackage"].module.conf_package_json(self)
            # Expose the path to the JS/WASM assets for consumers
            js_asset_path = os.path.join(self.package_folder, "bin")
            self.conf_info.define("user.nest2d:js_path", js_asset_path)
