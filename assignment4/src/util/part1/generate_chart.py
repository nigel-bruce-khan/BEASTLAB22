import sys
from argparse import ArgumentParser

try:
    import seaborn as sns
    import matplotlib.pyplot as plt
    import pandas as pd
except ImportError:
    print('Failed to load dependencies. Please ensure that seaborn, matplotlib, and pandas can be loaded.', file=sys.stderr)
    exit(-1)

if __name__ == '__main__':

    parser = ArgumentParser(description='Generate performance charts for ci stage.')
    parser.add_argument('--performance-data', action='append', default=[], required=True)
    parser.add_argument('--reference-data', action='append', default=[], required=True)
    parser.add_argument('--output-file', type=str, required=True)
    parser.add_argument('--hue-column', type=str, default='Threads', help='Seaborn hue column used for data frame.')

    result = parser.parse_args()

    fig = plt.figure(figsize=(8, 3))

    # Read data
    data = pd.DataFrame()
    ref_data = pd.DataFrame()

    for performance_data_file in result.performance_data:
        new_data = pd.read_fwf(performance_data_file, delimiter=' |')
        new_data["File"] = performance_data_file
        data = data.append(new_data, ignore_index=True)
    for reference_data_file in result.reference_data:
        new_data = pd.read_fwf(reference_data_file, delimiter=' |')
        new_data["File"] = reference_data_file
        ref_data = ref_data.append(new_data, ignore_index=True)

    # Measured performance
    axes = plt.subplot(1, 2, 1)

    sns.lineplot(data=data, x="Data size", y="MFLOPS", hue=result.hue_column, marker='o')
    axes.set_xscale('log', base=2)
    axes.set_title('Measured Performance')

    # Reference performance
    axes = plt.subplot(1, 2, 2)

    sns.lineplot(data=ref_data, x="Data size", y="MFLOPS", hue=result.hue_column, marker='o', axes=axes)
    axes.set_xscale('log', base=2)
    axes.set_title('Reference Performance')

    # Max ylim
    max_lim = max([ax.get_ylim()[1] for ax in fig.get_axes()])
    for ax in fig.get_axes():
        ax.set_ylim([0, max_lim])

    fig.tight_layout()

    plt.savefig(result.output_file)
    print(f"Wrote output to {result.output_file}")

