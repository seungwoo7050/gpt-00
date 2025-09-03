# LogCaster Quick Start Guide

## 🚀 5-Minute Setup

### Prerequisites
- C/C++ compiler (GCC or Clang)
- CMake 3.10+
- Python 3.6+ (for testing)
- netcat (nc) command

### 1. Clone & Build (30 seconds)
```bash
git clone <repository>
cd c-cpp-log-caster
./build.sh
```

### 2. Run Server (10 seconds)
```bash
# Terminal 1 - Run C version
./LogCaster-C/build/logcaster-c -P

# Terminal 2 - Run C++ version (different port)
./LogCaster-CPP/build/logcaster-cpp -P -p 8999
```

### 3. Send Test Logs (20 seconds)
```bash
# Terminal 3 - Send to C version
echo "Hello from C client" | nc localhost 9999
echo "ERROR: Test error message" | nc localhost 9999

# Send to C++ version
echo "Hello from C++ client" | nc localhost 8999
echo "WARNING: Test warning" | nc localhost 8999
```

### 4. Query Logs (20 seconds)
```bash
# Query C version
echo "COUNT" | nc localhost 9998
echo "SEARCH error" | nc localhost 9998

# Query C++ version  
echo "COUNT" | nc localhost 8998
echo "RECENT 5" | nc localhost 8998
```

### 5. Run Tests (optional)
```bash
cd LogCaster-C/tests
python3 test_basic.py

cd ../../LogCaster-CPP/tests
python3 test_basic.py
```

## 📁 What Gets Built?

After running `./build.sh`:
```
LogCaster-C/build/
└── logcaster-c         # C executable

LogCaster-CPP/build/
└── logcaster-cpp       # C++ executable
```

## 🎯 Common Use Cases

### Memory-Only Mode
```bash
./logcaster-c              # No persistence
./logcaster-cpp            # No persistence
```

### With Disk Persistence
```bash
./logcaster-c -P -d /tmp/logs -s 50    # 50MB files
./logcaster-cpp -P -d /tmp/logs -s 100 # 100MB files
```

### Custom Port
```bash
./logcaster-c -p 7777
./logcaster-cpp -p 8888
```

## 🔍 Query Commands

| Command | Example | Description |
|---------|---------|-------------|
| COUNT | `echo "COUNT" \| nc localhost 9998` | Total log count |
| SEARCH | `echo "SEARCH error" \| nc localhost 9998` | Case-insensitive search |
| REGEX | `echo "REGEX ^ERROR:.*failed$" \| nc localhost 9998` | Pattern match |
| FILTER | `echo "FILTER 2024-01-24 10:00:00 2024-01-24 11:00:00" \| nc localhost 9998` | Time range |
| RECENT | `echo "RECENT 50" \| nc localhost 9998` | Last N logs |
| CLEAR | `echo "CLEAR" \| nc localhost 9998` | Delete all logs |

## 🛠️ Troubleshooting

### Build Errors
```bash
# Missing CMake
sudo apt install cmake          # Ubuntu/Debian
brew install cmake              # macOS

# Missing compiler
sudo apt install build-essential # Ubuntu/Debian
xcode-select --install          # macOS
```

### Port Already in Use
```bash
# Find process using port
lsof -i :9999
# Kill it or use different port
./logcaster-c -p 10000
```

### Permission Denied (logs directory)
```bash
# Use accessible directory
./logcaster-c -P -d ~/logs
```

## 📊 Performance Testing
```bash
# Quick performance test
cd LogCaster-C/tests
python3 test_stress.py

# Compare both versions
python3 compare_performance.py
```

## 🎓 Learning Path

1. **Start Simple**: Run without persistence, send/query logs
2. **Enable Persistence**: Add `-P` flag, check `logs/` directory
3. **Test Queries**: Try all query commands
4. **Stress Test**: Run concurrent clients
5. **Compare**: See differences between C and C++ versions

## 📚 Next Steps

- Read [README.md](README.md) for detailed documentation
- Check [DEVELOPMENT_JOURNEY.md](DEVELOPMENT_JOURNEY.md) for implementation details
- Explore version snapshots in `versions/` directories
- Modify and experiment with the code!