import os
import sys
import pickle
import argparse
import torch
import random
import numpy as np
import torch.nn as nn
import torch.nn.functional as F
from tqdm import tqdm
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LinearRegression
from sklearn.metrics import mean_squared_error

use_cuda = 'cuda' if torch.cuda.is_available() else 'cpu'
device = torch.device(use_cuda)
torch.set_num_threads(1)

class PolicyRNN(nn.Module):

    def __init__(self, input_dim, hidden_dim):
        super().__init__()

        self.input_dim = input_dim
        self.hidden_dim = hidden_dim

        self.scaler = StandardScaler()

        self.embed = nn.Sequential(
            nn.Linear(self.input_dim, self.hidden_dim),
            nn.ReLU(),
            nn.Linear(self.hidden_dim, self.hidden_dim)
        )
        self.rnn = nn.GRU(
            input_size=self.hidden_dim,
            hidden_size=self.hidden_dim,
            num_layers=1,
            batch_first=True
        )
        self.target = nn.Sequential(
            nn.Linear(self.hidden_dim, self.hidden_dim),
            nn.ReLU(),
            nn.Linear(self.hidden_dim, 1)
        )

    def forward(self, x, hidden_state):
        rnn_in = self.embed(x)
        rnn_out, hidden_state = self.rnn(rnn_in, hidden_state)
        return self.target(rnn_out), hidden_state

    def do_train(self, features, lengths, scores, args):
        batch_size = args.batch_size
        num_batch = len(features) // batch_size + (1 if len(features) % batch_size else 0)

        x = []
        for i in range(len(lengths)):
            feature, length = features[i], lengths[i]
            for j in range(length):
                x.append(feature[j])
        self.scaler.fit(x)
        optimizer = torch.optim.Adam(self.parameters(), lr=args.lr, weight_decay=args.weight_decay)

        for i in range(args.epoch):
            self.train()
            epoch_loss = 0
            for j in tqdm(list(range(num_batch))):
                feature_batch = np.array(features[j*batch_size:(j+1)*batch_size])
                old_shape = feature_batch.shape
                feature_batch = feature_batch.reshape((-1, old_shape[-1]))
                feature_batch = self.scaler.transform(feature_batch).reshape(old_shape)
                feature_batch = torch.FloatTensor(feature_batch).to(device)
                lengths_batch = lengths[j*batch_size:(j+1)*batch_size]
                score_batch = torch.FloatTensor(scores[j*batch_size:(j+1)*batch_size]).to(device)
                hidden_state = torch.zeros(1, feature_batch.size(0), self.hidden_dim).to(device)
                score_out, _ = self(feature_batch, hidden_state)
                score_out = score_out.squeeze()

                loss, total_length = 0, 0
                for k in range(len(lengths_batch)):
                    length = lengths_batch[k]
                    loss += (score_out[k][:length] - score_batch[k][:length]).pow(2).sum()
                    total_length += length
                loss /= total_length
                epoch_loss += loss.item()

                optimizer.zero_grad()
                loss.backward()
                optimizer.step()
            # print(f'epoch {i}, loss {epoch_loss/num_batch}, score {self.score(features, lengths, scores, args)}')
            print(f'epoch {i}, loss {epoch_loss/num_batch}')
            if args.model_dir:
                os.makedirs(args.model_dir, exist_ok=True)
                self.save(os.path.join(args.model_dir, f'model_{i}.pt'))
                self.save(os.path.join(args.model_dir, f'model.pt'))

    def score(self, features, lengths, scores, args):
        batch_size = args.batch_size
        num_batch = len(features) // batch_size + (1 if len(features) % batch_size else 0)

        self.eval()
        u = 0
        with torch.no_grad():
            for i in range(num_batch):
                feature_batch = np.array(features[i*batch_size:(i+1)*batch_size])
                old_shape = feature_batch.shape
                feature_batch = feature_batch.reshape((-1, old_shape[-1]))
                feature_batch = self.scaler.transform(feature_batch).reshape(old_shape)
                feature_batch = torch.FloatTensor(feature_batch).to(device)
                lengths_batch = lengths[i*batch_size:(i+1)*batch_size]
                score_batch = torch.FloatTensor(scores[i*batch_size:(i+1)*batch_size]).to(device)
                hidden_state = torch.zeros(1, feature_batch.size(0), self.hidden_dim).to(device)
                score_out, _ = self(feature_batch, hidden_state)
                score_out = score_out.squeeze()

                for j in range(len(lengths_batch)):
                    length = lengths_batch[j]
                    u += (score_out[j][:length] - score_batch[j][:length]).pow(2).sum().item()

        num, score_sum, score_sum2 = 0, 0, 0
        for i in range(len(lengths)):
            length = lengths[i]
            num += length
            score_sum += scores[i][:length].sum()
            score_sum2 += (scores[i][:length] ** 2).sum()

        score_mean = score_sum / num
        v = score_sum2 - 2 * score_mean * score_sum + score_mean * score_mean

        return (1 - u/v)

    @staticmethod
    def load(path):
        if use_cuda == 'cuda':
            model_file = torch.load(path)
        else:
            model_file = torch.load(path, map_location='cpu')
        model = PolicyRNN(model_file['input_dim'], model_file['hidden_dim'])
        model.load_state_dict(model_file['state_dict'])
        model.scaler = model_file['scaler']

        return model

    def save(self, path):
        torch.save({
            'input_dim': self.input_dim,
            'hidden_dim': self.hidden_dim,
            'state_dict': self.state_dict(),
            'scaler': self.scaler,
        }, path)

class PolicyFeedforward(nn.Module):

    def __init__(self, input_dim, hidden_dim):
        super().__init__()

        self.input_dim = input_dim
        self.hidden_dim = hidden_dim

        self.scaler = StandardScaler()
        self.net = nn.Sequential(
            nn.Linear(self.input_dim, self.hidden_dim),
            nn.ReLU(),
            nn.Linear(self.hidden_dim, self.hidden_dim),
            nn.ReLU(),
            nn.Linear(self.hidden_dim, 1)
        )

    def forward(self, x):
        return self.net(x)

    def do_train(self, x, y, args):
        batch_size = args.batch_size
        num_batch = len(x) // batch_size + (1 if len(x) % batch_size else 0)

        self.scaler.fit(x)
        optimizer = torch.optim.Adam(self.parameters(), lr=args.lr, weight_decay=args.weight_decay)
        loss_func = nn.MSELoss()

        for i in range(args.epoch):
            self.train()
            epoch_loss = 0
            for j in tqdm(list(range(num_batch))):
                x_batch, y_batch = x[j*batch_size:(j+1)*batch_size], y[j*batch_size:(j+1)*batch_size]
                x_batch = self.scaler.transform(x_batch)
                x_batch, y_batch = torch.FloatTensor(x_batch).to(device), torch.FloatTensor(y_batch).to(device).unsqueeze(1)
                y_out = self(x_batch)

                loss = loss_func(y_out, y_batch)
                epoch_loss += loss.item()

                optimizer.zero_grad()
                loss.backward()
                optimizer.step()
            # print(f'epoch {i}, loss {epoch_loss/num_batch}, score {self.score(x, y, args)}')
            print(f'epoch {i}, loss {epoch_loss/num_batch}')
            if args.model_dir:
                os.makedirs(args.model_dir, exist_ok=True)
                self.save(os.path.join(args.model_dir, f'model_{i}.pt'))
                self.save(os.path.join(args.model_dir, f'model.pt'))

    def score(self, x, y, args):
        batch_size = args.batch_size
        num_batch = len(x) // batch_size + (1 if len(x) % batch_size else 0)

        y_tmp = torch.FloatTensor(y)
        u, v = 0, ((y_tmp - y_tmp.mean()) ** 2).sum().item()
        self.eval()
        with torch.no_grad():
            for i in range(num_batch):
                x_batch, y_batch = x[i*batch_size:(i+1)*batch_size], y[i*batch_size:(i+1)*batch_size]
                x_batch = self.scaler.transform(x_batch)
                x_batch, y_batch = torch.FloatTensor(x_batch).to(device), torch.FloatTensor(y_batch).to(device).unsqueeze(1)
                y_out = self(x_batch)
                u += (y_batch - y_out).pow(2).sum().item()

        return (1 - u/v)

    @staticmethod
    def load(path):
        if use_cuda == 'cuda':
            model_file = torch.load(path)
        else:
            model_file = torch.load(path, map_location='cpu')
        model = PolicyFeedforward(model_file['input_dim'], model_file['hidden_dim'])
        model.load_state_dict(model_file['state_dict'])
        model.scaler = model_file['scaler']

        return model

    def save(self, path):
        torch.save({
            'input_dim': self.input_dim,
            'hidden_dim': self.hidden_dim,
            'state_dict': self.state_dict(),
            'scaler': self.scaler,
        }, path)

class PolicyLinear:

    def __init__(self):
        self.scaler = StandardScaler()
        self.model = LinearRegression()

    def forward(self, x):
        return self.model.predict(self.scaler.transform(x))

    def do_train(self, x, y, args):
        self.scaler.fit(x)
        self.model.fit(self.scaler.transform(x), y)
        if args.model_dir:
            os.makedirs(args.model_dir, exist_ok=True)
            self.save(os.path.join(args.model_dir, f'model.pt'))

    def score(self, x, y):
        return self.model.score(self.scaler.transform(x), y)

    @staticmethod
    def load(path):
        with open(path, 'rb') as f:
            model = PolicyLinear()
            p = pickle.load(f, encoding='latin1')
            model.model = p['model']
            model.scaler = p['scaler']
        return model

    def save(self, path):
        with open(path, 'wb') as f:
            pickle.dump({'scaler':self.scaler, 'model':self.model}, f)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--features', dest='features', type=str, required=True)
    parser.add_argument('--lengths', dest='lengths', type=str, required=True)
    parser.add_argument('--model', dest='model', choices=['linear', 'feedforward', 'rnn'], type=str, required=True)

    parser.add_argument('--hidden_dim', dest='hidden_dim', type=int, default=64)
    parser.add_argument('--batch_size', dest='batch_size', type=int, default=4096)
    parser.add_argument('--epoch', dest='epoch', type=int, default=50)
    parser.add_argument('--lr', dest='lr', type=float, default=1e-2)
    parser.add_argument('--weight_decay', dest='weight_decay', type=float, default=1e-5)
    parser.add_argument('--seed', dest='seed', type=int, default=1)
    parser.add_argument('--model_dir', dest='model_dir', type=str, default=None)
    args = parser.parse_args()

    torch.manual_seed(args.seed)
    random.seed(args.seed)
    np.random.seed(args.seed)

    features_tmp, features = np.load(args.features), []
    lengths_tmp, lengths = np.load(args.lengths), []
    scores = []

    x, y = [], []
    for i in range(features_tmp.shape[0]):
        for j in range(lengths_tmp[i]):
            x.append(features_tmp[i][j][:-1])
            y.append(features_tmp[i][j][-1])
    tmp = list(zip(x, y))
    random.shuffle(tmp)
    x, y = zip(*tmp)
    mean, std = np.mean(y), np.std(y)

    if args.model in ('linear', 'feedforward'):
        # y = ((np.array(y) - mean) / std).tolist()
        if args.model == 'linear':
            model = PolicyLinear()
            model.do_train(x, y, args)
            print(model.score(x, y))
            print(mean_squared_error(y, model.forward(x)))
        elif args.model == 'feedforward':
            model = PolicyFeedforward(x[0].shape[0], args.hidden_dim).to(device)
            model.do_train(x, y, args)
    elif args.model == 'rnn':
        for i in range(features_tmp.shape[0]):
            features.append(features_tmp[i][:, :-1])
            lengths.append(lengths_tmp[i])
            # scores.append((features_tmp[i][:, -1] - mean) / std)
            scores.append(features_tmp[i][:, -1])
        tmp = list(zip(features, lengths, scores))
        random.shuffle(tmp)
        features, lengths, scores = zip(*tmp)
        model = PolicyRNN(features[0].shape[1], args.hidden_dim).to(device)
        model.do_train(features, lengths, scores, args)

if __name__ == '__main__':
    main()

_model = None
_model_type = None
def init_model(model_type, model_path):
    torch.manual_seed(1)
    random.seed(1)
    np.random.seed(1)

    model_type, model_path = model_type.decode('ascii'), model_path.decode('ascii')
    model_path = os.path.expandvars(os.path.expanduser(model_path))
    global _model_type, _model
    assert(model_type in ('linear', 'feedforward', 'rnn'))
    assert(os.path.exists(model_path))
    _model_type = model_type
    if _model_type == 'linear':
        _model = PolicyLinear.load(model_path)
    elif _model_type == 'feedforward':
        _model = PolicyFeedforward.load(model_path)
        _model.eval()
    elif _model_type == 'rnn':
        _model = PolicyRNN.load(model_path)
        _model.eval()
    else:
        assert(False)

def predict(x, hidden):
    assert(_model is not None)
    assert(_model_type is not None)
    assert(_model_type in ('linear', 'feedforward', 'rnn'))

    if _model_type == 'linear':
        res = _model.forward(x)
        return res.tolist(), None
    elif _model_type == 'feedforward':
        x = _model.scaler.transform(x)
        x = torch.FloatTensor(x)
        res = _model(x).squeeze(1).tolist()
        return res, None
    elif _model_type == 'rnn':
        x = _model.scaler.transform(x)
        x, hidden = torch.FloatTensor(x), torch.FloatTensor(hidden)
        x = x.view(x.shape[0], 1, x.shape[1])
        hidden = hidden.view(1, hidden.shape[0], hidden.shape[1])
        res, hidden_new = _model(x, hidden)
        res = res.view(x.size(0)).tolist()
        hidden_new = hidden_new.view(hidden.size(1), hidden.size(2)).tolist()
        return res, hidden_new

def sample(x):
    prob = F.softmax(torch.FloatTensor(x), dim=0).tolist()
    prob = np.array(prob)
    prob /= prob.sum()
    res = np.random.choice(list(range(len(x))), p=prob).item()
    return res