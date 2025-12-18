#!/bin/bash

echo "Testing Kermit with first node at 31.3.218.22:9001..."

# Start Kermit in background
cd build
./kermit -c ../kermit.conf > kermit_output.txt 2>&1 &
KERMIT_PID=$!

# Give it time to start
sleep 2

# Check if it's running and output contains our node
echo "Checking Kermit output..."
grep -q "31.3.218.22:9001" kermit_output.txt
if [ $? -eq 0 ]; then
    echo "✓ SUCCESS: First node 31.3.218.22:9001 found in configuration!"
    echo "Output:"
    cat kermit_output.txt
else
    echo "✗ FAILED: First node not found in configuration"
    echo "Output:"
    cat kermit_output.txt
fi

# Clean up
kill $KERMIT_PID 2>/dev/null
wait $KERMIT_PID 2>/dev/null

rm -f kermit_output.txt

echo "Test completed."