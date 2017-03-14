#!/usr/bin/env python3

from argparse import ArgumentParser

from mpi4py.MPI import COMM_WORLD
from numpy import matrix, concatenate, array_split, uint
from numpy.random import randint


def send(data, dest, tag):
    if SYNC:
        COMM_WORLD.send(data=data, dest=dest, tag=tag)
    else:
        COMM_WORLD.isend(data=data, dest=dest, tag=tag).wait()


def recv(source, tag):
    if SYNC:
        return COMM_WORLD.recv(source=source, tag=tag)
    return COMM_WORLD.irecv(source=source, tag=tag).wait()


def leader(size):
    dimensions = [randint(1000, 2000) for _ in range(3)]
    a = randint(1, 10, size=dimensions[:2], dtype=uint)
    b = randint(1, 10, size=dimensions[1:], dtype=uint)

    print(a, b, '\n', sep='\n\n')

    chunks = array_split(a, size)
    results = []

    for i, chunk in enumerate(chunks):
        if i == 0:
            results.append((matrix(chunk) * matrix(b)).astype(uint))
        else:
            COMM_WORLD.send(chunk, dest=i, tag=2)
            if 0 not in chunk.shape:
                COMM_WORLD.send(b, dest=i, tag=3)

    for rank in range(1, size):
        if 0 not in chunks[rank].shape:
            results.append(COMM_WORLD.recv(source=rank, tag=4))

    print(concatenate(results))


def follower():
    chunk = COMM_WORLD.recv(source=0, tag=2)
    if 0 not in chunk.shape:
        b = COMM_WORLD.recv(source=0, tag=3)
        c = (matrix(chunk) * matrix(b)).astype(uint)

        COMM_WORLD.send(c, dest=0, tag=4)


def main():
    rank = COMM_WORLD.Get_rank()
    size = COMM_WORLD.Get_size()
    if rank == 0:
        leader(size)
    else:
        follower()

if  __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('-a', '--async', action='store_false')
    args = parser.parse_args()

    SYNC = args.async

    main()
