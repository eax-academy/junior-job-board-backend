#!/bin/bash
# Run all tests for the Junior Job Board Backend
# This script runs the server and executes all test suites

set -e  # Exit on error

echo "=================================================="
echo "Junior Job Board Backend - Complete Test Suite"
echo "=================================================="
echo ""

# Check if server is already running
if curl -s http://localhost:8080/jobs > /dev/null 2>&1; then
    echo "✅ Server is already running"
else
    echo "⚠️  Server is not running!"
    echo "Please start the server first:"
    echo "  cd build"
    echo "  JWT_SECRET=my_secret_key_12345 ./server"
    echo ""
    exit 1
fi

echo ""
echo "=================================================="
echo "TEST 1: Full API Coverage"
echo "=================================================="
python3 tests/test_full_coverage.py
echo ""

echo "=================================================="
echo "TEST 2: Edge Cases"
echo "=================================================="
python3 tests/comprehensive_edge_cases.py
echo ""

echo "=================================================="
echo "TEST 3: Security & Performance"
echo "=================================================="
python3 tests/security_performance_tests.py
echo ""

echo "=================================================="
echo "TEST 4: Duplicate Prevention"
echo "=================================================="
python3 tests/test_duplicate_prevention.py
echo ""

echo "=================================================="
echo "✅ ALL TEST SUITES COMPLETED"
echo "=================================================="
echo ""
echo "Summary:"
echo "  - Full Coverage: 23 tests"
echo "  - Edge Cases: 69 tests"  
echo "  - Security & Performance: 44 tests"
echo "  - Duplicate Prevention: 1 test scenario"
echo "  - Total: 137+ tests"
echo ""
echo "Check TESTING_SUMMARY.md for detailed results"
