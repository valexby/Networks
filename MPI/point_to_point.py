#!/usr/bin/env python3

import numpy as np
from mpi4py import MPI
from argparse import ArgumentParser

MASTER_RANK = 0

FIRST_MATRIX_TAG = 2
SECOND_MATRIX_TAG = 3
RESULT_TAG = 4

X_MATRIX_SIZE = 1000
Y_MATRIX_SIZE = 2000


def _parse_args():
    parser = ArgumentParser()
    parser.add_argument('-a', '--async', action='store_false')
    return parser.parse_args()


def send(data, destination_process_rank, tag, is_sync=True):
    if is_sync:
        MPI.COMM_WORLD.send(data=data, dest=destination_process_rank, tag=tag)
    else:
        MPI.COMM_WORLD.isend(data=data, dest=destination_process_rank, tag=tag).wait()


def receive(source_process_rank, tag, is_sync=True):
    if is_sync:
        return MPI.COMM_WORLD.recv(source=source_process_rank, tag=tag)
    else:
        return MPI.COMM_WORLD.irecv(source=source_process_rank, tag=tag).wait()


def process_master_task(first_matrix, second_matrix, processes_count, is_async_mode):
    matrix_chunks = np.array_split(first_matrix, processes_count)

    results = []
    for rank_index, chunk in enumerate(matrix_chunks):
        if rank_index == MASTER_RANK:
            results.append((np.matrix(chunk) * np.matrix(second_matrix)).astype(np.uint))
        else:
            MPI.COMM_WORLD.send(chunk, dest=rank_index, tag=FIRST_MATRIX_TAG)
            if len(chunk):
                MPI.COMM_WORLD.send(second_matrix, dest=rank_index, tag=SECOND_MATRIX_TAG)

    for rank_index in range(1, processes_count):
        if len(matrix_chunks[rank_index]):
            results.append(MPI.COMM_WORLD.recv(source=rank_index, tag=RESULT_TAG))

    return np.concatenate(results)


def process_follower_task(is_async_mode):
    matrix_chunk = MPI.COMM_WORLD.recv(source=MASTER_RANK, tag=FIRST_MATRIX_TAG)

    if len(matrix_chunk):
        second_matrix = MPI.COMM_WORLD.recv(source=MASTER_RANK, tag=SECOND_MATRIX_TAG)
        result = (np.matrix(matrix_chunk) * np.matrix(second_matrix)).astype(np.uint)

        MPI.COMM_WORLD.send(result, dest=MASTER_RANK, tag=RESULT_TAG)


def _generate_matrices():
    dimensions = [np.random.randint(X_MATRIX_SIZE, Y_MATRIX_SIZE) for _ in range(3)]
    first_matrix = np.random.randint(1, 10, size=dimensions[:2], dtype=np.uint)
    second_matrix = np.random.randint(1, 10, size=dimensions[1:], dtype=np.uint)

    return first_matrix, second_matrix


def main():
    args = _parse_args()
    process_rank = MPI.COMM_WORLD.Get_rank()
    processes_count = MPI.COMM_WORLD.Get_size()

    if process_rank == MASTER_RANK:
        first_matrix, second_matrix = _generate_matrices()
        result_matrix = process_master_task(first_matrix, second_matrix, processes_count, args.async)

        if result_matrix == first_matrix * second_matrix:
            print("Matrix are equals.")
        else:
            print("Matrix are not equals.")
    else:
        process_follower_task(args.async)


if __name__ == '__main__':
    main()
