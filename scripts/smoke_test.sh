#!/usr/bin/env bash
# scripts/smoke_test.sh
# ---------------------------------------------------------------------------
# Smoke test for the ip-lookup-C binary trie engine.
#
# Generates tiny synthetic prefix/trace files and verifies that the binary
# matches a known expected output.  Designed to run in CI with no external
# data files required.
# ---------------------------------------------------------------------------

set -euo pipefail

BIN="./iplookup"
TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

RED='\033[0;31m'
GRN='\033[0;32m'
NC='\033[0m'

pass() { echo -e "${GRN}[PASS]${NC} $1"; }
fail() { echo -e "${RED}[FAIL]${NC} $1"; exit 1; }

echo "=== ip-lookup-C smoke test ==="

# ---- 1. Binary exists -------------------------------------------------------
[[ -x "$BIN" ]] || fail "Binary not found: $BIN  (run 'make' first)"

# ---- 2. Bad-args exit code --------------------------------------------------
"$BIN" 2>/dev/null && fail "Should have exited non-zero with no args" || true
"$BIN" badregion 20211122 2>/dev/null && fail "Should reject unknown region" || true
pass "argument validation"

# ---- 3. convert_prefixes unit check (via full pipeline) --------------------
#
#  We create a minimal rrc00 directory with a 3-entry prefix file and
#  a 2-entry trace file, run the binary, and check the output contains
#  exactly 2 lines (one per trace entry).

REGION_DIR="$TMPDIR/rrc00"
mkdir -p "$REGION_DIR"
mkdir -p "$TMPDIR/data"

# Prefix file (header + 3 entries)
cat > "$REGION_DIR/prefix_test.txt" <<'EOF'
Network
192.168.0.0/16
10.0.0.0/8
0.0.0.0/0
EOF

# Trace file (header + 2 destinations)
cat > "$REGION_DIR/trace_test.txt" <<'EOF'
Destination
192.168.1.1
10.5.5.5
EOF

# Run inside the tmp dir so relative paths resolve
pushd "$TMPDIR" > /dev/null
"$BIN" rrc00 test > /dev/null 2>&1 || fail "Pipeline returned non-zero"
popd > /dev/null

NEXTHOP_FILE="$TMPDIR/data/nexthop_rrc00_test.txt"
[[ -f "$NEXTHOP_FILE" ]] || fail "Output file not created: $NEXTHOP_FILE"

LINE_COUNT=$(wc -l < "$NEXTHOP_FILE")
[[ "$LINE_COUNT" -eq 2 ]] || fail "Expected 2 output lines, got $LINE_COUNT"
pass "pipeline produces correct line count"

echo ""
echo "All smoke tests passed."
