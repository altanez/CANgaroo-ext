# Prints Python build configuration for qmake as one ';'-separated line:
#   pybind11_include;python_include;python_libdir;python_libname
# Paths are returned as short (8.3) forward-slash paths so qmake/Make
# need no quoting even for locations like "C:\Program Files\...".
import ctypes
import pybind11
import sysconfig


def short_posix(p):
    if not p:
        return p
    buf = ctypes.create_unicode_buffer(260)
    ctypes.windll.kernel32.GetShortPathNameW(p, buf, 260)
    return (buf.value or p).replace("\\", "/")


libdir = sysconfig.get_config_var("LIBDIR") or ""
libname = "python" + sysconfig.get_python_version().replace(".", "")
print(";".join([
    short_posix(pybind11.get_include()),
    short_posix(sysconfig.get_paths()["include"]),
    short_posix(libdir),
    libname,
]))
