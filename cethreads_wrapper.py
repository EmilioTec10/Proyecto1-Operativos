import ctypes
import os
from config_reader import Mode, Side, CarType

# carga la librería compartida (ajusta la ruta si estás en otro folder)
_lib = ctypes.CDLL(os.path.abspath("build/libcethreads.so"))

# prototipos
_lib.simulator_init.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int]
_lib.simulator_init.restype  = None

_lib.simulator_add_car.argtypes = [ctypes.c_int, ctypes.c_int]
_lib.simulator_add_car.restype  = ctypes.c_int

_lib.simulator_start.argtypes = []
_lib.simulator_start.restype  = None

_lib.simulator_cleanup.argtypes = []
_lib.simulator_cleanup.restype  = None

def run_simulation(cfg):
    # inicializar
    _lib.simulator_init(cfg["mode"].value,
                        cfg["street_length"],
                        cfg["speed_base"],
                        cfg["sign_interval"],
                        cfg["W"])
    # añadir coches
    for side, ctype in cfg["cars"]:
        res = _lib.simulator_add_car(side.value, ctype.value)
        if res < 0:
            raise RuntimeError("No se pudo agregar el coche %d" % res)

    # arrancar
    _lib.simulator_start()

    # cleanup
    _lib.simulator_cleanup()
