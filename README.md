# UnrealMCP — Unreal Editor MCP Bridge

A TCP bridge plugin that exposes **90+ Unreal Editor automation commands** to AI agents via the [Model Context Protocol (MCP)](https://modelcontextprotocol.io/).

## Features

Control the Unreal Editor from any MCP-compatible AI agent:

- **Actors** — list, spawn, delete, transform, get/set properties
- **Blueprints** — read graphs, add/remove nodes, connect pins, add variables/functions/components/event dispatchers, compile
- **Blueprint Macros** — list, create, read macro graphs
- **Levels** — info, find by class, find in radius, screenshots
- **Landscapes** — create, sculpt, noise, heightmaps, paint layers, foliage
- **Materials** — get/set parameters, create material instances
- **Components** — list, add, remove actor components
- **Widgets (UMG)** — read widget trees, add/remove children, set properties/slots
- **Widget Animations** — list, read, create animations, add tracks/keyframes
- **Data Tables** — list, read, add/remove/edit rows
- **Sequencer** — list/read sequences, add/remove tracks, bindings, playback range
- **Niagara** (optional) — list/read systems, get/set emitter properties and parameters
- **Editor Utilities** — console commands, save, undo/redo, create/duplicate/delete/rename assets, open levels
- **Playtesting** — start/stop PIE, key/mouse input, game state queries, call component functions via reflection

## Installation

### UE5 Plugin

1. Copy the `UnrealMCP` folder into your project's `Plugins/` directory
2. Regenerate project files and build
3. The plugin starts automatically when the editor loads

### Python MCP Server

```bash
cd Python
pip install -e .
```

Configure your MCP client (e.g. Claude Desktop) to use the server:

```json
{
  "mcpServers": {
    "unreal": {
      "command": "python",
      "args": ["-m", "unreal_mcp.main"],
      "env": {
        "UE_PROJECT_PATH": "C:/Path/To/Your/Project"
      }
    }
  }
}
```

## Extending with Custom Commands

Game modules can register custom handlers without modifying the plugin:

```cpp
#include "UnrealMCPModule.h"
#include "MCPTcpServer.h"

void FMyGameModule::StartupModule()
{
    FMCPTcpServer* Server = FUnrealMCPModule::GetServer();
    if (Server)
    {
        Server->RegisterHandler(TEXT("my_custom_command"),
            [](const TSharedPtr<FJsonObject>& Params) -> TSharedPtr<FJsonObject>
            {
                auto Result = MakeShared<FJsonObject>();
                Result->SetBoolField(TEXT("success"), true);
                // ... your logic here
                return Result;
            });
    }
}
```

## Niagara Support

Niagara tools are compiled conditionally. In `UnrealMCP.Build.cs`, set `bEnableNiagara = false` if the Niagara plugin is not available in your project.

## Requirements

- Unreal Engine 5.1+
- Python 3.11+ (for the MCP server)

## License

MIT
