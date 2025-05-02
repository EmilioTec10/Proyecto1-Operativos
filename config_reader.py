import json
from enum import Enum

class Side(Enum):
    LEFT  = 0
    RIGHT = 1

class CarType(Enum):
    NORMAL    = 0
    SPORT     = 1
    EMERGENCY = 2

class Mode(Enum):
    FCFS     = 0
    SJF      = 1
    PRIORITY = 2
    RR       = 3
    REALTIME = 4

def load_config(path):
    with open(path, "r", encoding="utf-8") as f:
        j = json.load(f)
    cfg = {
        "mode": Mode[j["mode"]],
        "street_length": j["street_length"],
        "speed_base": j["speed_base"],
        "sign_interval": j["sign_interval"],
        "W": j["W"],
        "cars": [
            (Side[c["side"].upper()], CarType[c["type"].upper()])
            for c in j["cars"]
        ]
    }
    return cfg
