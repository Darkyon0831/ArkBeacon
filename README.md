# ArkBeacon

A powerful WebSocket server designed for data handling. ArkBeacon allows you to execute custom Python scripts via WebSocket connections and return processed data to clients in real-time.

## Features

- **WebSocket Server** with optional TLS support
- **Python Script Execution** - Run custom scripts dynamically
- **Auto-reload Scripts** - Automatically reload modules during development
- **Argument Passing** - Send parameters from clients to scripts
- **Concurrent Execution** - Thread-safe script execution queue
- **Interactive Commands** - Built-in command-line interface

## Building the Project

### Prerequisites

- CMake 3.24 or higher
- C++20 compatible compiler
- Python 3.x with development headers
- Required libraries (included as submodules):
  - pybind11
  - nlohmann/json
  - tomlplusplus
  - websocketpp

### Build Instructions

```bash
# Clone the repository
git clone <repository-url>
cd ArkBeacon

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# The executable will be in the build directory
./ArkBeacon
```

## Usage

### Basic Usage

Run the server with default settings (port 9002, no TLS):

```bash
./ArkBeacon
```

### Command-Line Arguments

| Argument | Parameters | Description | Default |
|----------|-----------|-------------|---------|
| `ScriptsPath` | `<path>` | Directory containing Python scripts | `./Scripts` |
| `Port` | `<port>` | WebSocket server port | `9002` |
| `UseTLS` | `<cert_path> <key_path>` | Enable TLS with certificate and key files | Disabled |
| `AutoReloadScripts` | None | Automatically reload Python modules on change | Disabled |

#### Examples

```bash
# Use custom port
./ArkBeacon Port 8080

# Enable TLS
./ArkBeacon UseTLS /path/to/cert.pem /path/to/key.pem

# Custom scripts directory with auto-reload
./ArkBeacon ScriptsPath /home/user/my_scripts AutoReloadScripts

# Combine multiple arguments
./ArkBeacon Port 9000 ScriptsPath ./custom_scripts AutoReloadScripts
```

### Interactive Commands

Once the server is running, you can use these commands:

- **`help`** - Display available commands
- **`reload`** - Manually reload all Python modules
- **`exit`** - Stop the server and exit

## Writing Python Scripts

Scripts must be placed in the `Scripts` directory (or the path specified with `ScriptsPath`).

### Basic Script Structure

Every script must have a `Run` function that takes an `out` parameter:

```python
def Run(out):
    # Your script logic here
    out.AddData("key_name", "value_data")
```

### The `out` Object

The `out` object is a `ParameterLoader` instance with the following methods:

#### `AddData(name, value)`
Add data to be returned to the WebSocket client.

- **name** (str): The key/identifier for the data
- **value** (str): The value to return

```python
def Run(out):
    out.AddData("ServerName", "My ARK Server")
    out.AddData("PlayerCount", "10")
    out.AddData("MapName", "TheIsland")
```

#### `GetArg(index)`
Get an argument passed from the client.

- **index** (int): The argument index (0-based)
- **Returns**: The argument value as a string

```python
def Run(out):
    # Get the first argument
    player_id = out.GetArg(0)
    out.AddData("RequestedPlayer", player_id)
```

#### `GetArgCount()`
Get the number of arguments passed from the client.

- **Returns**: Integer count of arguments

```python
def Run(out):
    arg_count = out.GetArgCount()
    
    for i in range(arg_count):
        arg_value = out.GetArg(i)
        out.AddData(f"arg_{i}", arg_value)
```

### Example Scripts

#### Example 1: Simple Information Script

```python
# Scripts/server_info.py
def Run(out):
    out.AddData("ServerName", "Monkey Servers")
    out.AddData("Status", "Online")
    out.AddData("Version", "1.0.0")
```

#### Example 2: Script with Arguments

```python
# Scripts/player_stats.py
def Run(out):
    # Expects player ID as first argument
    if out.GetArgCount() > 0:
        player_id = out.GetArg(0)
        
        # Process player data (example)
        out.AddData("player_id", player_id)
        out.AddData("level", "85")
        out.AddData("tribe", "Alpha Tribe")
    else:
        out.AddData("error", "No player ID provided")
```

#### Example 3: Using Python Modules

```python
# Scripts/system_info.py
import sys
import os
import platform

def Run(out):
    out.AddData("python_version", sys.version)
    out.AddData("platform", platform.system())
    out.AddData("hostname", platform.node())
    out.AddData("current_dir", os.getcwd())
```

## WebSocket Client Protocol

### Connection

Connect to the WebSocket server:

```
ws://localhost:9002
```

Or with TLS:

```
wss://localhost:9002
```

### Message Format

Send JSON messages to execute scripts:

```json
{
  "script_name": "server_info",
  "args": ["optional", "arguments", "here"]
}
```

#### Fields

- **`script_name`** (required): The name of the Python script to run (without `.py` extension)
- **`args`** (optional): Array of string arguments to pass to the script

### Response Format

The server returns JSON with the data added by your script:

```json
{
  "ServerName": "Monkey Servers",
  "Status": "Online",
  "Version": "1.0.0"
}
```

### JavaScript Client Example

```javascript
const ws = new WebSocket('ws://localhost:9002');

ws.onopen = () => {
    console.log('Connected to ArkBeacon');
    
    // Execute a script
    ws.send(JSON.stringify({
        script_name: "server_info"
    }));
};

ws.onmessage = (event) => {
    const data = JSON.parse(event.data);
    console.log('Received:', data);
};

ws.onerror = (error) => {
    console.error('WebSocket error:', error);
};

ws.onclose = () => {
    console.log('Disconnected from ArkBeacon');
};
```

### Python Client Example

```python
import websocket
import json

def on_message(ws, message):
    data = json.loads(message)
    print("Received:", data)

def on_open(ws):
    print("Connected to ArkBeacon")
    
    # Execute a script with arguments
    ws.send(json.dumps({
        "script_name": "player_stats",
        "args": ["player_123"]
    }))

ws = websocket.WebSocketApp(
    "ws://localhost:9002",
    on_message=on_message,
    on_open=on_open
)

ws.run_forever()
```

## Project Structure

```
ArkBeacon/
├── src/
│   ├── ArkBeacon.cpp           # Main entry point
│   ├── backend/                # WebSocket server implementation
│   ├── core/                   # Core components (ParameterLoader, etc.)
│   └── utils/                  # Utility classes
├── external/                   # Third-party libraries
│   ├── pybind11/
│   ├── json/
│   ├── tomlplusplus/
│   └── websocketpp/
├── build/
│   └── Scripts/                # Place your Python scripts here
└── CMakeLists.txt
```

## Development Tips

1. **Auto-reload during development**: Use `AutoReloadScripts` argument to automatically reload your Python scripts without restarting the server

2. **Manual reload**: Use the `reload` command in the interactive console to reload scripts

3. **Debugging scripts**: Add print statements in your Python scripts - they will appear in the server console

4. **Error handling**: Always check `GetArgCount()` before accessing arguments to avoid errors

5. **Return multiple values**: Call `AddData()` multiple times to return multiple key-value pairs

## Troubleshooting

### Server won't start
- Check if the port is already in use
- Verify Python is properly installed with development headers
- Ensure all dependencies are built correctly

### Scripts not executing
- Verify scripts are in the correct directory
- Check that scripts have a `Run(out)` function
- Look for Python errors in the server console
- Try using the `reload` command

### TLS connection issues
- Verify certificate and key file paths are correct
- Ensure certificate is valid and not expired
- Check file permissions on certificate files

## License

[Add your license information here]

## Contributing

[Add contribution guidelines here]
