<#
.SYNOPSIS
    启动 Qt 版排课系统（工作目录固定为仓库根目录，保证 output/ 等相对路径可用）
#>
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $root 'build_qt\scheduler_qt.exe'

if (-not (Test-Path $exe)) {
    throw "找不到 $exe，请先运行 scripts\build.ps1 -Target qt"
}

Push-Location $root
try {
    & $exe
} finally {
    Pop-Location
}
