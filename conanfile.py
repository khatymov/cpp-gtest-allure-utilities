from conan import ConanFile

class MyProjectConan(ConanFile):
    name = "my_project"
    version = "1.0"

    settings = "os", "compiler", "build_type", "arch"

    requires = (
        "gtest/1.14.0",
        "rapidjson/cci.20220822"
    )

    generators = "CMakeDeps", "CMakeToolchain"

    def layout(self):
        self.folders.build = "build"
        self.folders.source = "."