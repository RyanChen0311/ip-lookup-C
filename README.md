# ip-lookup-C

> A high-performance IPv4 routing engine implemented in C, using a **32-level Binary Trie** to perform **Longest Prefix Match (LPM)** — the same algorithm used inside real-world routers and network ASICs.

---

## Overview

IP routing requires finding the *most specific* matching prefix for any destination address.  This project implements that lookup from scratch in standard C, operating on real-world BGP routing table snapshots from [RIPE NCC Route Collectors](https://www.ripe.net/analyse/internet-measurements/routing-information-service-ris).

The engine can resolve **1 million+ prefixes** against large trace files and reports wall-clock timing for every phase of the pipeline.

---

## Algorithm

```
Destination:  192.168.1.1
Binary form:  11000000 10101000 00000001 00000001

             [root]
            /       \
          [0]       [1]   ← bit 0 = 1  →  go right
                   /   \
                 [0]   [1]   ← bit 1 = 1  →  go right
                 ...
          [match: 192.168.0.0/16]   ← deepest matching prefix = LPM result
```

Every bit of the destination address selects a left (0) or right (1) child.  Whenever a node carries a routing table index, it becomes the current *best match*.  The deepest such match at the end of traversal is the forwarding decision.

This mirrors what hardware FIB implementations (TCAM, pipeline ASICs) do conceptually, making this a faithful software model of real IP forwarding.

---

## Project Structure

```
ip-lookup-C/
├── src/
│   ├── main.c        # Entry point, CLI argument handling, pipeline driver
│   ├── trie.c        # Binary trie: insert / search (LPM) / free
│   ├── convert.c     # CIDR prefix & IP trace → 32-bit binary string
│   └── lookup.c      # End-to-end pipeline: table build + forwarding loop
├── include/
│   ├── ip_types.h    # Core structs: TrieNode, PrefixInfo, BinIPWithLen
│   ├── trie.h        # Trie interface
│   ├── convert.h     # Conversion interface
│   └── lookup.h      # Lookup engine interface
├── data/             # Generated binary intermediate files (git-ignored)
├── scripts/
│   └── smoke_test.sh # Automated smoke test (no external data required)
├── docs/             # Doxygen output
├── Makefile
└── Doxyfile
```

---

## Build

**Requirements:** GCC ≥ 9 (or any C11-compatible compiler), GNU Make

```bash
# Release build
make

# Debug build (AddressSanitizer + UBSan enabled)
make debug

# Generate Doxygen documentation
make docs

# Run smoke test
make test
```

The binary is placed at `./iplookup`.

---

## Usage

```
./iplookup <region> <date_tag>
```

| Argument   | Values                                   |
|------------|------------------------------------------|
| `region`   | `rrc00` `rrc01` `rrc03` `rrc04` `rrc05` `Final` |
| `date_tag` | Date string matching your input files, e.g. `20211122` |

### Expected input layout

```
./<region>/prefix_<date_tag>.txt    # CIDR prefix list
./<region>/trace_<date_tag>.txt     # Destination IP trace
```

**Prefix file format** (first line is a skipped header):
```
Network
192.168.0.0/16
10.0.0.0/8
0.0.0.0/0
```

**Trace file format** (first line is a skipped header):
```
Destination
192.168.1.1
10.5.5.5
8.8.8.8
```

### Example

```bash
./iplookup rrc04 20211122
```

Output files are written to `./data/`:
```
data/prefix_bin_rrc04_20211122.txt   # Binary-form routing table
data/trace_bin_rrc04_20211122.txt    # Binary-form trace
data/nexthop_rrc04_20211122.txt      # Resolved next-hop per destination
```

### Sample output

```
=== Pipeline: rrc04 / 20211122 ===
[convert] 847231 prefixes → data/prefix_bin_rrc04_20211122.txt
[convert] 150000 trace entries → data/trace_bin_rrc04_20211122.txt
[timing] alloc nexthop table           1021 ticks  (0.0010 s)
[timing] load nexthop table           18433 ticks  (0.0180 s)
[timing] parse routing table          42187 ticks  (0.0413 s)
[timing] build binary trie           389021 ticks  (0.3809 s)
[timing] LPM lookup (all trace)      211034 ticks  (0.2068 s)
[lookup] total lookups: 150000  |  default-route fallbacks: 3
```

---

## Dataset

The routing data used for development comes from **RIPE NCC RIS** BGP snapshots (November 2021).  Each RRC (Route Collector) represents a different peering point on the global internet, producing routing tables of 800 000 – 900 000 prefixes.

The data files are not included in this repository due to size (~5 GB uncompressed).  They can be downloaded from [https://www.ripe.net/analyse/internet-measurements/routing-information-service-ris](https://www.ripe.net/analyse/internet-measurements/routing-information-service-ris).

---

## Performance Notes

| Phase              | Bottleneck                        | Notes                            |
|--------------------|-----------------------------------|----------------------------------|
| Prefix conversion  | File I/O + `strtok` parsing       | Linear in number of prefixes     |
| Trie construction  | Pointer chasing + malloc          | ~800k insertions, up to 32 levels deep |
| LPM lookup         | Cache miss on trie traversal      | O(32) worst case per lookup      |
| Memory             | ~100 MB for 1M next-hop strings   | Tunable via `MAX_NEXTHOPS`       |

A future optimisation path is to replace the pointer-based trie with a **level-compressed (LC-trie)** or **stride-based** structure to improve cache locality.

---

## Known Limitations

- IPv4 only (32-bit addresses).
- Single-threaded; lookup loop is embarrassingly parallel and could be parallelised with `pthreads`.
- Next-hop table uses fixed `MAX_NEXTHOPS` (1M) allocation; very large tables may require adjustment.

---

## Background

This project was developed as coursework exploring how **IP routing lookups** work at the algorithm level, motivated by the observation that hardware routers (Cisco, Juniper) implement essentially the same binary-trie structure in silicon.  The implementation was validated against real BGP data from five RIPE RRC peering points.

---

## License

MIT — see [LICENSE](LICENSE).
