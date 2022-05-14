# Supplementary material for *Solving Atomix with pattern databases*

This repository holds the source code, instances and detailed result tables for the following paper:

> **Solving Atomix with pattern databases**<br>
> Alex Gliesch, Marcus Ritt (2016) <br>
> 5th Brazilian Conference on Intelligent Systems (BRACIS) <br>
> https://doi.org/10.1109/BRACIS.2016.022

**Bibtex**

```bibtex
@article{GlieschRitt/2016,
  title        = {Solving Atomix with pattern databases},
  author       = {Gliesch, Alex and Ritt, Marcus},
  booktitle    = {2016 5th Brazilian Conference on Intelligent Systems (BRACIS)},
  pages        = {61--66},
  year         = {2016},
  organization = {IEEE},
  doi          = {10.1109/BRACIS.2016.022}
}
```

Please use the reference above if you use this material in your research.

## Running the code 
1. Run `python Atomix.py -i {instance} -t {timeLimit} -m {memoryLimit}`. 
1. The python script recompiles the code under `src` for every run. Always use it to run the algorithm.
1. To output the optimal solution path, add `--path`. For more options, see `--help`.

To run [Hüffner et al. (2001)](https://doi.org/10.1007/3-540-45422-5_17)'s Atomix code, use the script and instances in `src/hueffner`.
