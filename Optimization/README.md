# Optimization

This folder contains scripts to run the Bayesian Optimization algorithm with the Prime Numbers trend to study the behaviors of different join types amongst other pivots. 

## How to Use 

1. ``conda init`` to start a new conda environment. Your shell should include a ``(base)`` prepended to it to indicate you are making use of the conda environment.
2. Ensure that the inputs are all up to the run. Interpretation:

| Input | Meaning | 
| ----- | ------- |
| ``INPUT_RANGE``      |  A dictionary keyed on the name of the variable you want to optimize with values as tuples with a specified range.       |
| ``OUTPUT_BASE_FOLDER``      |  Where the output json files should be written to. | 
| ``EXPLORATION_EXPLOITATION_PARAMETER``      | If you prefer exploitation, this value should be closer to 0 else, for exploration, this value should be closer to 0.1 |  
| ``SEED``      | Used for reproducibility of results |
| ``TOTAL_ITERATIONS``      | Total number of iterations to run the optimization to run for. | 
| ``INNER_ITERATIONS``      | To get stable results, we take the median of the inner iterations as the result for one outer iteration => this parameter controls how many inner iterations will be run per one outer iteration. |  
| ``BURN_IN_ITERATIONS``      | To aid the algorithm, we supply it with an additional few iterations used to discern the hyperparameters thereby, improving the overall quality of search. | 

3. Setup the experiment you want to run using the example given in the python file: ``pivot_thread_count`` and then construct different various pivots such as ``pivot_thread_count_2``. 

4. Once you have a list of pivots you'd want to examine, append them to a list and make use of the ``OptimizationExperiment`` class to compute the optimization.

## Output

The output is a json file that can be parsed using a Polyglot Notebook to chart the results. An example notebook can be found [here](./ComplexityPivotStudies.ipynb). Run the ``Install.cmd`` script to install the prerequistes of running notebooks.