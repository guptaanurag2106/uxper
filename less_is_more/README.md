# Less Is More: Text Classification

Implementation of text classification using gzip-based compression, inspired by the "Less Is More" paper.

## Context
- [Research Paper: "Less Is More: Parameter-Free Text Classification with Gzip"](https://arxiv.org/abs/2212.09410)
- [AG News Classification Dataset](https://www.kaggle.com/datasets/amananandrai/ag-news-classification-dataset)
- [libdeflate Library](https://github.com/ebiggers/libdeflate)

## Build & Run

```bash
# Build
make

# Run (requires dataset paths)
./main ./AG_Dataset/train.csv ./AG_Dataset/test.csv
```