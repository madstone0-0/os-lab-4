from dataclasses import dataclass
from heapq import heapify, heappop
from typing import Any


@dataclass
class Job:
    """
    Represents a job with an ID, arrival time, and cycle time.
    """

    id: str
    arrivalTime: int
    cycleTime: int

    def __lt__(self, other) -> bool:
        return self.arrivalTime < other.arrivalTime


def makeJobs() -> list[Job]:
    """
    Creates a predefined list of jobs
    """
    return [
        Job("A", 0, 16),
        Job("B", 3, 2),
        Job("C", 5, 11),
        Job("D", 9, 6),
        Job("E", 10, 1),
        Job("F", 12, 9),
        Job("G", 14, 4),
        Job("H", 16, 14),
        Job("I", 17, 1),
        Job("J", 19, 8),
    ]


class JobSpawner:
    """
    Manages job arrivals using a min-heap based on arrival times.
    """

    def __init__(self, jobList: list[Job]) -> None:
        self.jobHeap: list[Job] = jobList
        heapify(self.jobHeap)

    def hasJobs(self) -> bool:
        return len(self.jobHeap) != 0

    def jobArrival(self, tick: int) -> list[Job]:
        """
        Returns a list of jobs that arrive at the given tick.
        """
        arrived = []
        while self.hasJobs() and self.jobHeap[0].arrivalTime <= tick:
            arrived.append(heappop(self.jobHeap))
        return arrived


def printStatusDictAsTable(
    d: dict[Any, Any], keyName="Job ID", valName="Value"
) -> None:
    """
    Prints a dictionary of metrics in a tabular format.
    """
    if not d:
        print("Empty")
        return

    metrics = list(d.keys())
    jobIds = sorted({jid for vals in d.values() for jid in vals.keys()})

    colWidths = {}
    colWidths[keyName] = max(
        len(str(keyName)), max((len(str(j)) for j in jobIds), default=0)
    )
    for m in metrics:
        max_val_len = max((len(str(d[m].get(j, ""))) for j in jobIds), default=0)
        colWidths[m] = max(len(str(m)), max_val_len)

    header = f"{keyName:<{colWidths[keyName]}}"
    for m in metrics:
        header += " | " + f"{m:<{colWidths[m]}}"
    print(header)

    sepLen = sum(colWidths.values()) + 3 * len(metrics)
    print("-" * sepLen)

    for jid in jobIds:
        row = f"{str(jid):<{colWidths[keyName]}}"
        for m in metrics:
            row += " | " + f"{str(d[m].get(jid, '')):<{colWidths[m]}}"
        print(row)


if __name__ == "__main__":
    jobs = makeJobs()
    spawner = JobSpawner(jobs)

    for i in range(21):
        job = spawner.jobArrival(i)
        print(f"t({i}) -> {'No job' if not job else job}")
