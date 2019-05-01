Import('env')
from os.path import join, realpath, islink, abspath
from os import symlink, unlink

# Initialize
hal_symbol = abspath(join("src", "hal"))
unlink(hal_symbol)

for item in env.get("CPPDEFINES", []):
    # Setup HAL
    if isinstance(item, tuple) and item[0] == "HAL":
        src = realpath(join("src", "hal_" + item[1].lower()))
        symlink(src, hal_symbol)
        break

# Default HAL=posix
if not islink(hal_symbol):
    src = realpath(join("src", "hal_posix"))
    symlink(src, hal_symbol)
