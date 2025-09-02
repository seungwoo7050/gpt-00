#!/bin/bash
# Build script for LogCaster project - builds both C and C++ versions

set -e  # Exit on error

echo "============================================"
echo "Building LogCaster - C and C++ Versions"
echo "============================================"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Function to build a version
build_version() {
    local version=$1
    local dir=$2
    
    echo -e "\n${GREEN}Building $version...${NC}"
    cd "$dir"
    
    # Clean previous build
    rm -rf build
    mkdir build
    cd build
    
    # Configure and build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ $version built successfully!${NC}"
    else
        echo -e "${RED}✗ $version build failed!${NC}"
        exit 1
    fi
    
    cd ../..
}

# Get the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Build both versions
build_version "LogCaster-C" "LogCaster-C"
build_version "LogCaster-CPP" "LogCaster-CPP"

echo -e "\n${GREEN}============================================${NC}"
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}============================================${NC}"

echo -e "\nExecutables created:"
echo "  - LogCaster-C/build/logcaster-c"
echo "  - LogCaster-CPP/build/logcaster-cpp"

echo -e "\nTo run the servers:"
echo "  ./LogCaster-C/build/logcaster-c -P"
echo "  ./LogCaster-CPP/build/logcaster-cpp -P -p 8999"