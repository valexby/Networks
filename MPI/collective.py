#!/usr/bin/env python3
import numpy as np
from argparse import ArgumentParser
from mpi4py import MPI

from constants import MASTER_RANK, X_MATRIX_SIZE, Y_MATRIX_SIZE


def _parse_args():
    parser = ArgumentParser()
    parser.add_argument('-c', '--count', type=int, default=1)
    return parser.parse_args()


def calculate_color(global_process_rank, requested_group_count):
    if requested_group_count > global_process_rank:
        return global_process_rank
    else:
        return np.random.randint(requested_group_count)


def split_into_groups(requested_group_count):
    global_processes_count = MPI.COMM_WORLD.Get_size()
    global_process_rank = MPI.COMM_WORLD.Get_rank()

    if requested_group_count > global_processes_count:
        print("Not enough processors")
        raise SystemExit()

    process_color = calculate_color(global_process_rank, requested_group_count)
    processes_group = MPI.COMM_WORLD.Split(color=process_color, key=global_process_rank)
    processes_group.Set_name(b'%d' % process_color)

    return processes_group


def _generate_matrices():
    dimensions = [np.random.randint(X_MATRIX_SIZE, Y_MATRIX_SIZE) for _ in range(3)]
    first_matrix = np.random.randint(1, 10, size=dimensions[:2], dtype=np.uint)
    second_matrix = np.random.randint(1, 10, size=dimensions[1:], dtype=np.uint)

    return first_matrix, second_matrix


def print_initial_data(group_name, first_matrix, second_matrix):
    print('first_matrix:  {}:\n{}'.format(group_name, first_matrix))
    print('second_matrix: {}:\n{}'.format(group_name, second_matrix))


def print_results(group_name, result_matrix, process_time):
    print('Results {}:\n{}'.format(group_name, result_matrix))
    print('Time: {}'.format(process_time))


def main():
    args = _parse_args()
    requested_group_count = args.count

    process_group = split_into_groups(requested_group_count)
    group_rank = process_group.Get_rank()
    group_processes_count = process_group.Get_size()

    # Basic variable initialization
    first_matrix = None
    second_matrix = None
    matrix_chunks = None

    if group_rank == MASTER_RANK:
        first_matrix, second_matrix = _generate_matrices()
        matrix_chunks = np.array_split(first_matrix, group_processes_count)
        print_initial_data(process_group.name, first_matrix, second_matrix)

    MPI.COMM_WORLD.barrier()
    start_time = MPI.Wtime()

    # Send or receive initial data for matrix multiplication
    first_matrix = process_group.scatter(matrix_chunks)
    second_matrix = process_group.bcast(second_matrix)

    # Send or assembly results.
    result = np.matrix(first_matrix) * np.matrix(second_matrix)
    result = process_group.gather(result.astype(np.uint))

    end_time = MPI.Wtime()
    MPI.COMM_WORLD.barrier()

    if group_rank == MASTER_RANK:
        process_time = end_time - start_time
        result_matrix = np.concatenate(result)
        print_results(process_group.name, result_matrix, process_time)
        
        if result_matrix == first_matrix * second_matrix:
            print("Matrix are equals.")
        else:
            print("Matrix are not equals.")


if __name__ == '__main__':
    main()
