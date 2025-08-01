#!/usr/bin/env python3
"""
STL Viewer MCP Server

MCP server that provides tools for displaying 3D models using the STL Viewer application.
"""

import asyncio
import logging
import subprocess
from pathlib import Path
from typing import Any

import mcp.server.stdio
from mcp import types
from mcp.server import NotificationOptions, Server
from mcp.server.models import InitializationOptions


# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("stl-viewer-mcp")

# Initialize the MCP server
server = Server("stl-viewer-mcp")

# Paths for STL viewer
PROJECT_ROOT = Path(__file__).parent.parent.parent
STL_VIEWER_PATH = PROJECT_ROOT / "build" / "Release" / "stl_viewer.exe"


@server.list_tools()
async def handle_list_tools() -> list[types.Tool]:
    """List available tools."""
    return [
        types.Tool(
            name="display_3d_model",
            description="Display a 3D model file using the STL Viewer application (fixed 800x600 window)",
            inputSchema={
                "type": "object",
                "properties": {
                    "file_path": {
                        "type": "string",
                        "description": "Path to the 3D model file (STL, OBJ, etc.)",
                    }
                },
                "required": ["file_path"],
            },
        )
    ]


@server.call_tool()
async def handle_call_tool(
    name: str, arguments: dict[str, Any] | None
) -> list[types.TextContent]:
    """Handle tool calls."""
    if name != "display_3d_model":
        raise ValueError(f"Unknown tool: {name}")

    if not arguments:
        raise ValueError("Missing arguments")

    file_path = arguments.get("file_path")
    if not file_path:
        raise ValueError("file_path is required")

    # Validate file exists
    model_file = Path(file_path)
    if not model_file.exists():
        return [
            types.TextContent(type="text", text=f"Error: File not found: {file_path}")
        ]

    # Check if STL viewer executable exists
    if not STL_VIEWER_PATH.exists():
        return [
            types.TextContent(
                type="text",
                text=f"Error: STL Viewer executable not found at: {STL_VIEWER_PATH}\nPlease build the project first.",
            )
        ]

    try:
        # Run the STL viewer
        cmd = [str(STL_VIEWER_PATH), str(model_file)]
        logger.info(f"Running command: {' '.join(cmd)}")

        # Use subprocess.Popen with DETACHED_PROCESS for independent execution
        # Set working directory to project root so shaders/ can be found
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            cwd=str(PROJECT_ROOT),
            creationflags=subprocess.DETACHED_PROCESS
            | subprocess.CREATE_NEW_PROCESS_GROUP,
        )

        # Give the process a moment to start
        await asyncio.sleep(0.5)

        # Check if process started successfully
        if process.poll() is not None:
            # Process has already terminated, check for errors
            stdout, stderr = process.communicate()
            error_msg = f"STL Viewer exited immediately. "
            if stderr:
                error_msg += f"Error: {stderr}"
            if stdout:
                error_msg += f"Output: {stdout}"
            return [types.TextContent(type="text", text=error_msg)]

        return [
            types.TextContent(
                type="text",
                text=f"STLビューアーを起動しました: {model_file.name}\n\n利用可能な操作:\n- マウスホイール: ズームイン/アウト\n- ESCキー: ビューアー終了\n\n注意: モデルは静的表示です（回転や移動操作は未実装）",
            )
        ]

    except Exception as e:
        logger.error(f"Error launching STL viewer: {e}")
        return [
            types.TextContent(type="text", text=f"Error launching STL viewer: {str(e)}")
        ]


async def main():
    """Main entry point for the MCP server."""
    async with mcp.server.stdio.stdio_server() as (read_stream, write_stream):
        await server.run(
            read_stream,
            write_stream,
            InitializationOptions(
                server_name="stl-viewer-mcp",
                server_version="1.0.0",
                capabilities=server.get_capabilities(
                    notification_options=NotificationOptions(),
                    experimental_capabilities={},
                ),
            ),
        )


if __name__ == "__main__":
    asyncio.run(main())
