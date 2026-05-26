#!/usr/bin/env bash
# scripts/fetch_bgp_data.sh
# ---------------------------------------------------------------------------
# Downloads a real BGP routing table from RIPE NCC RIS and converts it into
# the prefix / trace format expected by ip-lookup-C.
#
# Usage:
#   bash scripts/fetch_bgp_data.sh [region] [date_tag]
#
#   region   : rrc00 | rrc01 | rrc03 | rrc04 | rrc05  (default: rrc00)
#   date_tag : any string used as the file suffix     (default: today YYYYMMDD)
#
# Requires: bgpdump   (sudo apt-get install bgpdump)
#           wget
# ---------------------------------------------------------------------------

set -euo pipefail

REGION="${1:-rrc00}"
DATE_TAG="${2:-$(date +%Y%m%d)}"
RIS_URL="https://data.ris.ripe.net/${REGION}/latest-bview.gz"
MRT_FILE="${REGION}/latest-bview.gz"

# ---- dependency check -------------------------------------------------------
for cmd in bgpdump wget; do
    command -v "$cmd" >/dev/null 2>&1 || {
        echo "[error] '$cmd' not found."
        [[ "$cmd" == "bgpdump" ]] && echo "        Install: sudo apt-get install bgpdump"
        [[ "$cmd" == "wget"    ]] && echo "        Install: sudo apt-get install wget"
        exit 1
    }
done

mkdir -p "$REGION"

# ---- download ---------------------------------------------------------------
echo "[1/4] Downloading routing table from RIPE NCC RIS..."
echo "      URL : $RIS_URL"
wget -q --show-progress -O "$MRT_FILE" "$RIS_URL"
echo ""

# ---- convert MRT → raw prefix list -----------------------------------------
# bgpdump -m prints one line per RIB entry in pipe-separated format:
#   TYPE|timestamp|B|peer_ip|peer_asn|PREFIX|as_path|...
# We extract field 6 (the prefix), keep only IPv4, deduplicate.

echo "[2/4] Converting MRT binary to prefix list..."
bgpdump -m "$MRT_FILE" 2>/dev/null \
    | awk -F'|' '$6 ~ /^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\/[0-9]+$/ { print $6 }' \
    | sort -u \
    > "${REGION}/prefixes_raw.txt"

PREFIX_COUNT=$(wc -l < "${REGION}/prefixes_raw.txt")
echo "      Found $PREFIX_COUNT unique IPv4 prefixes."

# ---- build prefix file (program input format) -------------------------------
echo "[3/4] Writing prefix_${DATE_TAG}.txt..."
{ echo "Network"; cat "${REGION}/prefixes_raw.txt"; } \
    > "${REGION}/prefix_${DATE_TAG}.txt"

# ---- build trace file -------------------------------------------------------
# Generate one destination per prefix: use the network address itself.
# This guarantees every destination has a known correct match in the table.
# Take at most 100,000 entries so the lookup phase finishes in reasonable time.

echo "[4/4] Writing trace_${DATE_TAG}.txt (up to 100,000 entries)..."
{
    echo "Destination"
    head -100000 "${REGION}/prefixes_raw.txt" \
        | awk -F'/' '{ print $1 }'
} > "${REGION}/trace_${DATE_TAG}.txt"

TRACE_COUNT=$(( $(wc -l < "${REGION}/trace_${DATE_TAG}.txt") - 1 ))
echo "      Generated $TRACE_COUNT trace entries."

# ---- summary ----------------------------------------------------------------
echo ""
echo "Done.  Files written:"
echo "   ${REGION}/prefix_${DATE_TAG}.txt  ($PREFIX_COUNT prefixes)"
echo "   ${REGION}/trace_${DATE_TAG}.txt   ($TRACE_COUNT destinations)"
echo ""
echo "Next step:"
echo "   make"
echo "   ./iplookup ${REGION} ${DATE_TAG}"
