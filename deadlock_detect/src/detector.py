'''
Implementation of deadlock detector
Repo: https://github.com/leo-jin-edb/bdr-interview-assignments
'''

import json
import logging
from operator import itemgetter

from .dep_graph import GraphEdge, Graph

log = logging.getLogger(__name__)
log.setLevel(level="DEBUG")


class DeadlockDetector(object):
    '''
    Deadlock detector class
    '''

    def __init__(self, json_job_dep: str = None):
        '''
        Deadlock detector constructor
        '''
        self.dep_graph = Graph()
        if json_job_dep:
            self.add_dep_from_json(json_job_dep)

    def add_dep_from_json(self, json_job_dep: str = None):
        '''
        Add job dependencies
        :param json_job_dep: JSON formatted string of job dependencies
        '''
        nodes = json.loads(json_job_dep)
        # JSON string parsing OK
        # Check nodes of type dict
        if not isinstance(nodes, dict):
            raise ValueError
        # Parse nodes
        for node_name, objects in nodes.items():
            if isinstance(objects, dict):
                for obj_name, dep_list in objects.items():
                    if isinstance(dep_list, list) and len(dep_list) > 1:
                        dst_job = dep_list[0]
                        for i in range(1, len(dep_list)):
                            src_job = dep_list[i]
                            # Add dependency edge
                            edge = GraphEdge(src_job, dst_job, obj_name, node_name)
                            self.dep_graph.edge_add(edge)

    def kill_min_deadlock_jobs(self, cycles: list):
        '''
        Kill min number of jobs to remove cycles
        '''
        while len(cycles) > 0:
            # Calculate job frequency in cycles
            job_freq = {}
            for cycle in cycles:
                for job in cycle:
                    if job in job_freq:
                        job_freq[job] = job_freq[job] + 1
                    else:
                        job_freq[job] = 1
            # Sort jobs in decending frequency order
            sorted_jobs = sorted(job_freq, key=job_freq.get, reverse=True)
            if sorted_jobs:
                job = sorted_jobs[0]
                # Kill max freq job
                print(f"Deleting job {job}")
                self.dep_graph.node_delete(node_id=job)
                # Delete cycles including job
                rem_cycles = []
                for cycle in cycles:
                    if job in cycle:
                        log.debug(f"Removed cycle {cycle}")
                    else:
                        rem_cycles.append(cycle)
                cycles = rem_cycles

    def deadlock_exists(self, remove_deadlock:bool = False):
        '''
        Check if the dependency graph has a deadlock
        :return: True if there is a deadlock, False otherwise
        '''
        deadlock = False

        dep_cycles = self.dep_graph.find_cycles()
        if dep_cycles:
            deadlock = True
            log.warning("Deadlock FOUND")
            if remove_deadlock:
                self.kill_min_deadlock_jobs(cycles=dep_cycles)
                # Check for cycles after deadlock removal
                rem_cycles = self.dep_graph.find_cycles()
                if rem_cycles:
                    log.error("Deadlock removal FAILURE")
                else:
                    log.info("Deadlock removal SUCCESS")
        else:
            log.info("Deadlock NOT FOUND")

        return deadlock
