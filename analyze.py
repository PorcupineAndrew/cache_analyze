#! /usr/bin/env python3
#-*- coding: utf-8 -*-
# **********************************************************************
# * Description   : analyze for result
# * Last change   : 21:10:00 2020-04-05
# * Author        : Yihao Chen
# * Email         : chenyiha17@mails.tsinghua.edu.cn
# * License       : www.opensource.org/licenses/bsd-license.php
# **********************************************************************

import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path
from glob import glob

script_dir = Path(os.path.dirname(os.path.abspath(__file__)))
result_dir = script_dir / "result"
trace_dir = script_dir / "trace"

# result analyze
if os.path.exists(result_dir / "summary.csv"):
    df = pd.read_csv(result_dir / "summary.csv")
else:
    result_files = list(map(Path, glob(str(result_dir / "*.output"))))

    def get_count(path):
        df = pd.read_csv(path, engine="c", dtype=str, names=["state"])
        return np.sum(df.state == "Miss"), np.sum(df.state == "Hit")

    records = map(lambda a, b: a + list(b), map(lambda x: str(x.stem).split("_"), result_files), map(get_count, result_files))
    df = pd.DataFrame(list(records), columns=["trace", "block", "way", "algorithm", "assignment", "write", "miss", "hit"])
    df.block = df.block.astype(int)
    df.way.loc[df.way == "FA"] = float("inf")
    df.way.loc[df.way == "DM"] = 0.
    df.way = df.way.astype(float)
    df.assignment = df.assignment.astype(int)
    df.write = df.write.astype(int)
    df.to_csv(result_dir / "summary.csv", index=False)

list_block = [3, 5, 6]
list_way = [0., 2., 3., float('inf')]
list_algo = ["lru", "rand", "tree"]
list_assignment = [0, 1]
list_write = [0, 1]

def bar_on_metrc(ax, df, comb, metric):
    data = df.loc[np.logical_and.reduce([df[k] == v for k,v in comb.items()])]
    data["miss_ratio"] = data.miss / (data.miss + data.hit)
    data.sort_values(by=list(data.columns), inplace=True)
    print(data)
    metric_values = np.unique(data[metric].values.tolist(), axis=0)

    ax.set_title(f"Miss ratio on '{'&'.join(metric)}'\n{comb}")
    ax.grid(True)
    ax.set_xlabel("trace")
    ax.set_ylabel("miss ratio")

    bar_width = 0.2
    for idx, v in enumerate(metric_values):
        m = list(zip(metric, v))
        d = data.loc[np.logical_and.reduce([data[i] == j for i,j in m])].sort_values(by=["trace"])
        X = np.arange(len(d.trace)) - ((len(metric_values)-1)/2-idx)*bar_width
        ax.bar(X, d.miss_ratio, label=f"{'&'.join([f'{i}={j}' for i,j in m])}", width=bar_width, linewidth=0, alpha=0.5)

    ax.set_xticks(np.arange(len(d.trace)))
    ax.set_xticklabels(d.trace)
    ax.legend()

n_row, n_col, ax_idx = 3, 3, 1
fig = plt.figure(figsize=(n_col*5, n_row*5))

basic_comb = {
    "algorithm": "lru",
    "assignment": 1,
    "write": 1,
}
for comb in map(lambda x: dict(block=x, **basic_comb), list_block):
    bar_on_metrc(fig.add_subplot(n_row, n_col, ax_idx), df, comb, ["way"])
    ax_idx += 1

comb = {
    "block": 3,
    "way": 3.,
    "assignment": 1,
    "write": 1,
}
bar_on_metrc(fig.add_subplot(n_row, n_col, ax_idx), df, comb, ["algorithm"])
ax_idx += 1

comb = {
    "block": 3,
    "way": 3.,
    "algorithm": "lru",
}
bar_on_metrc(fig.add_subplot(n_row, n_col, ax_idx), df, comb, ["assignment", "write"])
ax_idx += 1

# trace analyze
traces = np.sort(np.unique(df.trace.values.tolist()))
del df

def locality_on_addr(ax, traces):
    addr_norm, xtick = 128, 16
    ax.set_xlabel(f"address distribution on mod-{addr_norm}")
    for tr in traces:
        addr = pd.read_csv(trace_dir / f"{tr}.trace", engine='python', sep="\s+", names=["act", "addr"]).addr.apply(lambda x: int(x[2:], 16)).values
        addr, counts = np.unique(addr % addr_norm, return_counts=True)
        addr = addr / addr_norm
        counts = np.log10(counts) / 10
        r = counts
        theta = 2 * np.pi * addr
        ax.plot(theta, r, label=tr, alpha=0.5, linewidth=1)
    ax.set_xticks(2 * np.pi / xtick * np.arange(xtick))
    ax.set_xticklabels([f"{int(i*addr_norm/xtick)}" for i in np.arange(xtick)])
    ax.set_yticks([])
    ax.legend()

locality_on_addr(fig.add_subplot(n_row, n_col, ax_idx, projection='polar'), traces)
ax_idx += 1

def locality_on_traces(ax, traces):
    window_size, sample_size = 100, 1000
    ax.set_xlabel("trace")
    ax.set_ylabel("locality")
    ax.set_title(f"Locality when window_size={window_size}&sample_size={sample_size}")
    for tr in traces:
        addr = pd.read_csv(trace_dir / f"{tr}.trace", engine='python', sep="\s+", names=["act", "addr"]).addr.apply(lambda x: int(x[2:], 16)).values
        count = dict(zip(*np.unique(addr[:window_size], return_counts=True)))
        locality = [len(count)]
        for i, a_push in enumerate(addr[window_size:]):
            a_pop = addr[i]
            count[a_pop] -= 1
            if a_push in count:
                count[a_push] += 1
            else: count[a_push] = 1
            if count[a_pop] == 0:
                count.pop(a_pop)
            locality.append(len(count))
        locality = 1 - np.convolve(locality, np.ones(window_size)/window_size, "valid") / window_size
        locality = [locality[int(i*len(locality)/sample_size):int((i+1)*len(locality)/sample_size)].mean() for i in range(sample_size)]
        ax.plot(np.arange(len(locality)), locality, alpha=0.5, linewidth=1, label=tr)
    ax.legend()

locality_on_traces(fig.add_subplot(n_row, 1, n_row), traces)

fig.tight_layout(rect=[0, 0.03, 1, 0.95])
fig.savefig(result_dir / f"analyze.png")
