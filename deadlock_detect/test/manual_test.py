#! /usr/bin/python3

'''
Deadlock detector: Manual tests
'''

import logging
import os
import sys

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from src.detector import DeadlockDetector
from test.conftest import GRAPHS

logging.basicConfig(level="DEBUG")


if __name__ == '__main__':
    gindex = 0
    for dlock, gstr in GRAPHS:
        print(f"Graph {gindex}:")
        detector = DeadlockDetector(json_job_dep=gstr)
        deadlock = detector.deadlock_exists(remove_deadlock=True)
        if deadlock:
            print(f"Graph {gindex}: DEADLOCK FOUND")
        else:
            print(f"Graph {gindex}: NO DEADLOCK")
        assert deadlock == dlock, f"Deadlock state {deadlock} does not match given state {dlock}"
        gindex += 1
        print()
