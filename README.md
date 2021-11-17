Learch: a Learning-based Strategies for Path Exploration in Symbolic Execution
=============================================================================================================

Learch is a learning-based state selection strategy for symbolic execution. It can achieve significantly more coverage and detects more security violations than existing manual heuristics. Learch is instantiated on [KLEE](http://klee.github.io/). The directory `klee` contains our modified KLEE code (from [this commit](https://github.com/klee/klee/tree/95ce1601c380341ef3b1043644c66be754e345c0)) and `learch` contains the Learch code. Learch is developed at [SRI Lab, Department of Computer Science, ETH Zurich](https://www.sri.inf.ethz.ch/) as part of the [Machine Learning for Programming](https://www.sri.inf.ethz.ch/research/plml) project. For more details, please refer to [Learch CCS'21 paper](https://files.sri.inf.ethz.ch/website/papers/ccs21-learch.pdf).

## Setup
We provide a [docker file](Dockerfile), which we recommend to start with. To set Learch up locally, one can follow the instructions in the [docker file](Dockerfile). To build and run:
```
$ docker build -t learch .
$ docker run -it learch
```

## Usages
The following README files explains how to use Learch:
* Obtaining the benchmarks used in the paper: [`benchmarks/README.md`](benchmarks/README.md).
* Training Learch: [`train/README.md`](train/README.md).
* Using Learch to test new programs: [`eval/README.md`](eval/README.md).

## Citing Learch
```
@inproceedings{10.1145/3460120.3484813,
  author = {He, Jingxuan and Sivanrupan, Gishor and Tsankov, Petar and Vechev, Martin},
  title = {Learning to Explore Paths for Symbolic Execution},
  year = {2021},
  isbn = {9781450384544},
  publisher = {Association for Computing Machinery},
  address = {New York, NY, USA},
  url = {https://doi.org/10.1145/3460120.3484813},
  doi = {10.1145/3460120.3484813},
  booktitle = {Proceedings of the 2021 ACM SIGSAC Conference on Computer and Communications Security},
  pages = {2526–2540},
  numpages = {15},
  keywords = {fuzzing, symbolic execution, machine learning, program testing},
  location = {Virtual Event, Republic of Korea},
  series = {CCS '21}
}
```

## Authors
* [Jingxuan He](https://www.sri.inf.ethz.ch/people/jingxuan)
* Gishor Sivanrupan
* [Petar Tsankov](https://www.sri.inf.ethz.ch/people/petar)
* [Martin Vechev](https://www.sri.inf.ethz.ch/people/martin)

## License
The KLEE code uses [KLEE Release License](klee/LICENSE.txt) and the Leach code uses [Apache License 2.0](learch/LICENSE).