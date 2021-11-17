import os
import random
import argparse
import pandas as pd
import numpy as np
from mycov import init_cov, get_cov, replay

def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--prog', dest='prog', type=str, required=True)
    parser.add_argument('--src_dir', dest='src_dir', type=str, required=True)
    parser.add_argument('--tests_dir', dest='tests_dir', type=str, required=True)
    parser.add_argument('--replay_script', dest='replay_script', type=str, default=os.path.join(os.environ['SOURCE_DIR'], 'replay.sh'))
    parser.add_argument('--max_len', dest='max_len', type=int, default=30)
    parser.add_argument('--neg_pos_ratio', dest='neg_pos_ratio', type=int, default=40)
    parser.add_argument('--seed', dest='seed', type=int, default=2)
    args = parser.parse_args()
    return args

args = get_args()
random.seed(args.seed)
np.random.seed(args.seed)

def main():
    init_cov(args.src_dir)

    ktests, ktests_normal, ktests_covnew = [], [], []
    for f in os.listdir(args.tests_dir):
        prefix = f[:f.rfind('.')]
        feature_file = f'{prefix}.features.csv'
        if f.endswith('.ktest') and os.path.exists(os.path.join(args.tests_dir, feature_file)):
            ktests.append(f)
            covnew = f'{prefix}.covnew'
            if os.path.exists(os.path.join(args.tests_dir, covnew)):
                ktests_covnew.append(f)
            else:
                ktests_normal.append(f)
    ktests = list(sorted(ktests))

    sample_num = len(ktests_covnew) * args.neg_pos_ratio
    sample_num = sample_num if sample_num < len(ktests_normal) else len(ktests_normal)
    ktests_normal = random.sample(ktests_normal, sample_num)
    ktests_sampled = set(ktests_normal + ktests_covnew)

    improvements, remaining_times, rewards = dict(), dict(), dict()
    cov_prev = 0
    for f in ktests:
        prefix = f[:f.rfind('.')]
        feature_file = f'{prefix}.features.csv'
        covnew = f'{prefix}.covnew'
        features = pd.read_csv(os.path.join(args.tests_dir, feature_file))
        if os.path.exists(os.path.join(args.tests_dir, covnew)):
            replay(args.prog, args.replay_script, os.path.join(args.tests_dir, f))
            line_covered = len(get_cov(args.src_dir))
            cov_after = line_covered
            improvement = cov_after - cov_prev
            cov_prev = cov_after
        else:
            improvement = 0

        remaining_time = 0
        for row in reversed(list(features.itertuples())):
            if row[1] not in improvements:
                improvements[row[1]] = 0
            improvements[row[1]] += improvement
            if row[1] not in remaining_times:
                remaining_times[row[1]] = row.QueryCost
            remaining_times[row[1]] += remaining_time
            remaining_time += row.QueryCost

    for index in improvements:
        reward = (improvements[index] / max(remaining_times[index], 0.025)) ** 0.5
        rewards[index] = reward

    data, lengths = None, np.array([], dtype=np.int64)

    data, lengths = [], []
    for f in ktests:
        prefix = f[:f.rfind('.')]
        feature_file = f'{prefix}.features.csv'
        features = pd.read_csv(os.path.join(args.tests_dir, feature_file), index_col='Index')[:-1]
        features['Improvement'] = features.apply(lambda line: improvements[line.name], axis=1)
        features['RemainingTime'] = features.apply(lambda line: remaining_times[line.name], axis=1)
        features['Reward'] = features.apply(lambda line: rewards[line.name], axis=1)
        features.to_csv(os.path.join(args.tests_dir, feature_file+'.new'))

        if f in ktests_sampled:
            # features = pd.read_csv(os.path.join(args.tests_dir, f'{prefix}.features.csv.new'), index_col='Index')[:-1]
            features = features.drop(['QueryCost', 'QueryCostAcc', 'Improvement', 'RemainingTime'], axis=1)
            features = np.array(features)[:min(features.shape[0], args.max_len), :]
            if features.shape[0] < args.max_len:
                to_concat = np.zeros((args.max_len - features.shape[0], features.shape[1]), dtype=np.float64)
                concatenated = np.concatenate((features, to_concat), axis=0)
            else:
                concatenated = features

            concatenated = concatenated.reshape(1, concatenated.shape[0], concatenated.shape[1])
            data.append(concatenated)
            lengths.append(features.shape[0])

    data = np.concatenate(data, axis=0)
    lengths = np.array(lengths, dtype=np.int64)
    np.save(os.path.join(args.tests_dir, "all_features.npy"), data)
    np.save(os.path.join(args.tests_dir, "all_lengths.npy"), lengths)

    init_cov(args.src_dir)

if __name__ == '__main__':
    main()
