import sys
import itertools

import pandas as pd

def main():
    in_paths = [sys.argv[1], sys.argv[2]]
    frames = [pd.read_json(path) for path in in_paths]
    data = pd.concat(frames)
    # TODO: better visualisation
    print(data.sort_values('dataset'))

if __name__ == '__main__':
    main()
