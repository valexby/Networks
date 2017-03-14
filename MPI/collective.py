#!/usr/bin/env python3

from argparse import ArgumentParser
from mpi4py.MPI import COMM_WORLD, Wtime
from numpy import matrix, concatenate, array_split, uint
from numpy.random import randint


def split_into_groups(count):
    world_size = COMM_WORLD.Get_size()
    if count > world_size:
        quit("Not enough processors")
    world_rank = COMM_WORLD.Get_rank()
    if count > world_rank:
        color = world_rank
    else:
        color = randint(count)
    comm = COMM_WORLD.Split(color=color, key=world_rank)
    comm.Set_name(b'%d' % color)
    return comm


def main(count):
    comm = split_into_groups(count)
    rank = comm.Get_rank()
    size = comm.Get_size()
    if rank == 0:
        dimensions = [randint(1000, 2000) for _ in range(3)]
        a = randint(1, 10, size=dimensions[:2], dtype=uint)
        b = randint(1, 10, size=dimensions[1:], dtype=uint)
        divided_a = array_split(a, size)
        print('a{name}:\n{a}\nb{name}:\n{b}'.format(name=comm.name, a=a, b=b))
    else:
        a = None
        b = None
        divided_a = None
    COMM_WORLD.barrier()
    start = Wtime()
    a = comm.scatter(divided_a)
    b = comm.bcast(b)
    c = matrix(a) * matrix(b)
    c = comm.gather(c.astype(uint))
    end = Wtime()
    COMM_WORLD.barrier()
    if rank == 0:
        print('c{name}:\n{c}\ntime: {time}'.format(
            name=comm.name, c=concatenate(c), time=end - start))

if  __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('-c', '--count', type=int, default=1)
    args = parser.parse_args()

    main(args.count)
