#!/bin/bash

echo "Testing Kermit configuration loading..."

cd /home/byte/kermit/build

# Run Kermit with the test configuration and capture output
TIMEOUT=2
timeout $TIMEOUT ./kermit -c ../simple_test.conf > output.txt 2>&1 &
KERMIT_PID=$!

# Wait for the process to start or timeout
sleep $TIMEOUT

# Kill the process if it's still running
kill $KERMIT_PID 2>/dev/null
wait $KERMIT_PID 2>/dev/null

# Check the output
echo "=== Kermit Output ==="
cat output.txt

echo ""
echo "=== Checking for first node ==="
if grep -q "31.3.218.22:9001" output.txt; then
    echo "✓ SUCCESS: First node 31.3.218.22:9001 found in configuration!"
else
    echo "✗ FAILED: First node not found in configuration"
fi

# Clean up
rm -f output.txt

echo "Test completed."