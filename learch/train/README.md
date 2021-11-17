Training Learch
=============================================================================================================

We provide the 4 trained feedforward network models in [`trained`](trained), which you can directly use for testing new programs (see [`../eval/README.md`](../eval/README.md)).

To train Learch from scratch, we use an iterative learning algorithm (See Section 4 in [the paper](https://files.sri.inf.ethz.ch/website/papers/ccs21-learch.pdf)). At each iteration, we first use [`train_data.sh`](train_data.sh) to generate a supervised dataset. In the example command below, we show how to generate a supervised dataset for the `b2sum` program by running the `random-path` search heuristic for 60 seconds. The results are stored in `~/train_data`. 
```
./train_data.sh b2sum ~/train_data 60 0 "random-path"
```

[`train_data.sh`](train_data.sh) can be parallelized for multiple programs and multiple search strategies. After [`train_data.sh`](train_data.sh) is finished, we run [`collect_scores.py`](collect_scores.py) to merge the generated supervised datasets into single files (you can modify [`../benchmarks/coreutils_train.txt`](../benchmarks/coreutils_train.txt) to contain only `b2sum` for the example trial):
```
python3 collect_scores.py --prog_list ${SOURCE_DIR}/benchmarks/coreutils_train.txt --output_dir ~/train_data --iter 0
```

Then, we call the machine learning algorithm to learn a feedforward neural network model:
```
python3 ../model.py --features ~/train_data/all_features-0.npy --lengths ~/train_data/all_lengths-0.npy --model feedforward --model_dir ~/feedforward
```

After the first iteration, one can generate new supervised dataset with the learned strategy and continue the iterative process to obtain more learned strategies:
```
./train_data.sh b2sum ~/train_data 60 1 "feedforward ~/feedforward/model.pt"
```