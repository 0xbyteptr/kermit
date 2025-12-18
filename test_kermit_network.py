#!/usr/bin/env python3

import socket
import subprocess
import time
import sys
import signal
import os

def test_kermit_network():
    print("Testing Kermit network functionality...")
    
    # Start Kermit in background
    print("Starting Kermit router...")
    kermit_process = subprocess.Popen(
        ["./kermit", "-c", "../kermit.conf"],
        cwd="./build",
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    
    # Give Kermit time to start
    time.sleep(2)
    
    try:
        # Test TCP connection
        print("Connecting to Kermit router on port 9055...")
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('localhost', 9055))
        print("✓ Successfully connected to Kermit router")
        
        # Send test message
        test_message = b"Hello Kermit from Python test!"
        s.send(test_message)
        print(f"✓ Sent {len(test_message)} bytes to Kermit router")
        
        # Give time for processing
        time.sleep(1)
        
        s.close()
        print("✓ Connection closed gracefully")
        
        print("\nKermit network test: PASSED")
        return True
        
    except Exception as e:
        print(f"✗ Network test failed: {e}")
        return False
        
    finally:
        # Stop Kermit process
        print("Stopping Kermit router...")
        kermit_process.terminate()
        try:
            kermit_process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            kermit_process.kill()
            kermit_process.wait()
        
        print("Kermit router stopped")

if __name__ == "__main__":
    success = test_kermit_network()
    sys.exit(0 if success else 1)