# UnrealMCP — AI Agent Automation for Unreal Editor

A code plugin that exposes **90+ Unreal Editor automation commands** to AI agents via the [Model Context Protocol (MCP)](https://modelcontextprotocol.io/). Works with Claude Code, Cursor, Windsurf, Continue, Cline, and any MCP-compatible client.

## How It Works

```
AI Agent  <-->  Python MCP Server  <-->  TCP  <-->  UnrealMCP Plugin  <-->  Unreal Editor
```

The plugin runs a lightweight TCP server inside the editor. A companion Python MCP server bridges the MCP protocol to the plugin. AI agents connect and issue commands — the server handles the rest.

## 90+ Tools Across 15 Categories

| Category | Tools |
|---|---|
| **Actors** | List, spawn, delete, transform, get/set any property |
| **Blueprints** | Read graphs, add/remove nodes, connect pins, set defaults, add variables/functions/components/dispatchers, compile |
| **Blueprint Macros** | List, create, read macro graphs |
| **Levels** | Level info, find actors by class or radius, screenshots |
| **Landscapes** | Create, sculpt (brush/noise), import heightmaps, paint layers, place/clear foliage |
| **Materials** | Get/set parameters, create material instances |
| **Components** | List, add, remove actor components |
| **Widgets (UMG)** | Read widget trees, add/remove children, set properties/slots |
| **Widget Animations** | List, read, create animations, add tracks/keyframes |
| **Data Tables** | List, read, add/remove/edit rows |
| **Sequencer** | List/read sequences, add/remove tracks, bindings, playback range |
| **Niagara** (optional) | List/read systems, emitter properties, parameters |
| **Assets** | Create, duplicate, delete, rename, import, save |
| **Editor Utilities** | Console commands, save levels, undo/redo, open levels |
| **Playtesting** | Start/stop PIE, keyboard/mouse input, game state, component function calls via reflection |

## Installation

### 1. UE5 Plugin

Copy the `UnrealMCP` folder into your project's `Plugins/` directory, regenerate project files, and build. The plugin starts automatically when the editor loads.

### 2. Python MCP Server

```bash
cd Plugins/UnrealMCP/Python
pip install -e .
```

### 3. Configure Your AI Agent

Add the MCP server to your AI client's configuration (example for Claude Code / Claude Desktop):

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

- Unreal Engine 5.5 or later (tested on 5.7)
- Python 3.11+ (for the MCP server)

## Support

- Documentation: https://gist.github.com/Victorvsky/0fa4e1aa6039cd2ec6e09ba2d1e37477

## License

Copyright (c) 2026 victorvksy. All rights reserved.
Licensed under the [Fab Standard License (EULA)](https://www.fab.com/eula).
