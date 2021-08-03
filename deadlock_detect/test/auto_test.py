'''
Deadlock detector: Auto tests
'''

import logging
import os
import pytest
import sys

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from src.detector import DeadlockDetector
from test.conftest import GRAPHS

logging.basicConfig(level="DEBUG")

@pytest.mark.parametrize("dependency_graph", GRAPHS)
def test_deadlock_detection(dependency_graph):
    '''
    Test Deadlock detection
    '''
    dlock, gstr = dependency_graph
    detector = DeadlockDetector(json_job_dep=gstr)
    deadlock = detector.deadlock_exists(remove_deadlock=False)
    # Check detected deadlock state matches provided state
    assert deadlock == dlock, f"Deadlock state {deadlock} does not match given state {dlock}"


@pytest.mark.parametrize("dependency_graph", GRAPHS)
def test_deadlock_removal(dependency_graph):
    '''
    Test Deadlock removal
    '''
    dlock, gstr = dependency_graph
    detector = DeadlockDetector(json_job_dep=gstr)
    deadlock = detector.deadlock_exists(remove_deadlock=True)
    # Check detected deadlock state matches provided state
    assert deadlock == dlock, f"Deadlock state {deadlock} does not match given state {dlock}"
    # Check that the deadlock has been removed
    deadlock = detector.deadlock_exists(remove_deadlock=False)
    assert not deadlock, f"System should be deadlock free at this point"
