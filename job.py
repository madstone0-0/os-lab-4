from dataclasses import dataclass


@dataclass
class Job:
    id: int
    arrivalTime: int
    cycleTime: int
