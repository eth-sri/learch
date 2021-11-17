import os
import argparse
import numpy as np
from tqdm import tqdm


def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--prog_list', dest='prog_list', type=str, required=True)
    parser.add_argument('--output_dir', dest='output_dir', type=str, required=True)
    parser.add_argument('--iter', dest='iter', type=int, required=True)
    args = parser.parse_args()
    return args


args = get_args()


def main():
    progs = []
    with open(args.prog_list) as f:
        for line in f:
            progs.append(line.strip())

    data, lengths = [], []
    if args.iter > 0:
        data.append(np.load(os.path.join(args.output_dir, f'all_features-{args.iter-1}.npy')))
        lengths.append(np.load(os.path.join(args.output_dir, f'all_lengths-{args.iter-1}.npy')))

    for searcher_dir in sorted(os.listdir(args.output_dir)):
        if not searcher_dir.endswith(f'-{args.iter}'):
            continue
        for prog in sorted(progs):
            prog_data = np.load(os.path.join(args.output_dir, searcher_dir, prog, 'all_features.npy'))
            prog_lengths = np.load(os.path.join(args.output_dir, searcher_dir, prog, 'all_lengths.npy'))
            data.append(prog_data)
            lengths.append(prog_lengths)

    data = np.concatenate(data, axis=0)
    lengths = np.concatenate(lengths, axis=0)

    np.save(os.path.join(args.output_dir, f'all_features-{args.iter}.npy'), data)
    np.save(os.path.join(args.output_dir, f'all_lengths-{args.iter}.npy'), lengths)


if __name__ == '__main__':
    main()