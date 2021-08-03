#!/usr/bin/env python

import os
import time
import string
import random
import argparse
import tempfile
import subprocess
from multiprocessing.pool import Pool
from collections import namedtuple


ClientRun = namedtuple('ClientRun', ['runtime', 'returncode'])


def run_timed_process(args):
    start_time = time.perf_counter_ns()
    process = subprocess.run(args, stdout=subprocess.DEVNULL)

    end_time = time.perf_counter_ns()
    return ClientRun(
        runtime=(end_time - start_time),
        returncode=process.returncode)


def generate_commands(common, args, chunk_size):
    return list(
        common + args[i:i + chunk_size]
        for i in range(0, len(args), chunk_size))


def main():
    # parse command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-w', '--server-workers',
        type=int,
        default=4,
        help="server worker threads")

    parser.add_argument(
        '-c', '--client-processes',
        type=int,
        default=4,
        help="concurrent client processes")

    parser.add_argument(
        '-k', '--chunk-size',
        type=int,
        default=100,
        help="client command word chunk size")

    parser.add_argument(
        '-i', '--initial-words',
        type=int,
        default=1000000,
        help="initial words to load before running benchmarks")

    parser.add_argument(
        '-r', '--random-words',
        type=int,
        default=50000,
        help="random access words for each command")

    args = parser.parse_args()

    # locate binaries
    build_directory = os.path.join(os.getcwd(), 'build')
    server_binary = os.path.join(build_directory, 'pkg', 'dictdb', 'dictdb')
    client_binary = os.path.join(build_directory, 'pkg', 'dict', 'dict')

    # create a temporary working directory
    with tempfile.TemporaryDirectory(prefix='dictdb-') as temp_directory:
        socket_file = os.path.join(temp_directory, 'socket')

        # start the server
        print(">> Start server")
        server_process = subprocess.Popen([
            server_binary,
            '-s', socket_file,
            '-w', str(args.server_workers)])

        time.sleep(1.0)
        if server_process.returncode:
            print("Failed!")
            return

        # determine common client arguments
        client_args = [client_binary, '-s', socket_file]

        # create a process pool
        with Pool(processes=args.client_processes) as pool:
            # load initial words
            print(">> Generate initial words")
            words = list(
                ''.join(random.choices(string.ascii_letters + string.digits, k=12))
                for i in range(args.initial_words))

            client_initial_inserts = generate_commands(
                client_args + ['insert'],
                words,
                args.chunk_size)

            print(">> Load initial words")
            start_time = time.perf_counter()
            initial_load_runs = pool.map(
                run_timed_process,
                client_initial_inserts,
                chunksize=100)

            elapsed_time = round(time.perf_counter() - start_time, 4)
            print(f"{elapsed_time} seconds")

            # XXX
            print(">> Generating test operations")
            insert_words = list(
                ''.join(random.choices(string.ascii_letters + string.digits, k=16))
                for i in range(args.random_words))

            client_inserts = generate_commands(
                client_args + ['insert'],
                insert_words,
                args.chunk_size)

            search_words = random.sample(list(words), args.random_words)
            client_searches = generate_commands(
                client_args + ['search'],
                search_words,
                args.chunk_size)

            delete_words = random.sample(list(words), args.random_words)
            client_deletes = generate_commands(
                client_args + ['delete'],
                delete_words,
                args.chunk_size)

            client_operations = client_inserts + client_deletes + client_searches
            random.shuffle(client_operations)

            print(">> Running benchmark")
            start_time = time.perf_counter()
            benchmark_runs = pool.map(
                run_timed_process,
                client_operations,
                chunksize=100)

            elapsed_time = round(time.perf_counter() - start_time, 4)
            print(f"{elapsed_time} seconds")

        # stop the server
        print(">> Stop server")
        server_process.terminate()
        server_process.wait()


if __name__ == '__main__':
    main()
