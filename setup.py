from setuptools import setup
from setuptools.command.build_py import build_py
import subprocess

class BuildMake(build_py):
    def run(self, *args, **kwargs):
        subprocess.check_call(('make', 'lib_cabaliser.so'), cwd="c_lib")        
        subprocess.check_call(('cp', 'c_lib/lib_cabaliser.so', 'src/cabaliser/lib_cabaliser_bin.so'))

        return super().run(*args, **kwargs)
setup(
    name='cabaliser',
    packages=['cabaliser'],
    package_dir={'':'src'},
    package_data={"cabaliser":["lib_cabaliser_bin.so"]},
    include_package_data=True,
    cmdclass={'build_py': BuildMake}
)
