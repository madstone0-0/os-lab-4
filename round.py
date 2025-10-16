from job import Job, JobSpawner, makeJobs, printStatusDictAsTable
from collections import defaultdict, deque


class RoundRobin:
    def __init__(self) -> None:
        self.tick = 0
        self.readyQueue: deque[Job] = deque()

        jobs = makeJobs()
        self.spawner = JobSpawner(jobs)
        self.timeQuantum = 4

        self.remainingTime: dict[int, int] = {j.id: j.cycleTime for j in jobs}

        # stats: metric -> job_id -> value
        self.stats: dict[str, dict[int, int]] = {
            "Waiting Time": defaultdict(int),
            "Turnaround Time": defaultdict(int),
        }
        for j in jobs:
            self.stats["Waiting Time"][j.id] = 0
            self.stats["Turnaround Time"][j.id] = 0

    def run(self):
        while self.spawner.hasJobs() or any(t > 0 for t in self.remainingTime.values()):
            # Add arrivals to the ready queue
            arrivals = self.spawner.jobArrival(self.tick)
            for job in arrivals:
                print(f"t({self.tick})\t->\t{job.id}\t+")
                self.readyQueue.append(job)

            # If no jobs are ready, just tick
            if not self.readyQueue:
                self.tick += 1
                continue

            currJob = self.readyQueue.popleft()
            timeSlice = min(self.timeQuantum, self.remainingTime[currJob.id])

            # Execute one tick at a time to handle arrivals and waiting updates
            for _ in range(timeSlice):
                # Update waiting time for all ready and waiting jobs
                for readyJob in self.readyQueue:
                    self.stats["Waiting Time"][readyJob.id] += 1

                # Execute one tick for current job
                self.tick += 1
                self.remainingTime[currJob.id] -= 1

                # Add arrivals that happen during this tick
                newArrivals = self.spawner.jobArrival(self.tick)
                for job in newArrivals:
                    print(f"t({self.tick})\t->\t{job.id}\t+")
                    self.readyQueue.append(job)

                # If current finished, record turnaround
                if self.remainingTime[currJob.id] == 0:
                    self.stats["Turnaround Time"][currJob.id] = (
                        self.tick - currJob.arrivalTime
                    )
                    break

            # If not finished, requeue
            if self.remainingTime[currJob.id] > 0:
                self.readyQueue.append(currJob)
            else:
                print(f"t({self.tick})\t->\t{currJob.id}\t-")

        # Print status table
        print("\nFinal Job Statistics")
        printStatusDictAsTable(self.stats)
        print()

        # Averages and summary
        n = len(self.remainingTime)
        avgWait = sum(self.stats["Waiting Time"].values()) / n
        avgTurn = sum(self.stats["Turnaround Time"].values()) / n
        print("Averages")
        print(f"Average Waiting Time    : {avgWait:.2f}")
        print(f"Average Turnaround Time : {avgTurn:.2f}")


if __name__ == "__main__":
    rr = RoundRobin()
    rr.run()
