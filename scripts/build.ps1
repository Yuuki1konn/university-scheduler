<#
.SYNOPSIS
    一键构建「教务排课系统」的两个入口
.DESCRIPTION
    -Target tui : 用 MinGW g++ 编译 TUI 版，产物 output/scheduler.exe
    -Target qt  : 用 CMake + MinGW 编译 Qt 版，产物 build_qt/scheduler_qt.exe
    -Target all : 两个都编译（默认）
    -Clean      : 先删除构建产物再编译
.EXAMPLE
    pwsh -File scripts/build.ps1 -Target tui
#>
[CmdletBinding()]
param(
    [ValidateSet('tui', 'qt', 'all')]
    [string]$Target = 'all',
    [switch]$Clean
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot          # 仓库根目录
$tuiExe = Join-Path $root 'output/scheduler.exe'
$qtBuild = Join-Path $root 'build_qt'
$qtExe = Join-Path $qtBuild 'scheduler_qt.exe'

# 工具链位置：优先 PATH，其次默认安装路径
function Resolve-Tool([string]$name, [string[]]$fallbacks) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    foreach ($p in $fallbacks) { if (Test-Path $p) { return $p } }
    throw "找不到 $name，请先安装并加入 PATH"
}

$gxx = Resolve-Tool 'g++' @('C:\Mingw64\mingw64\bin\g++.exe')
$cmake = Resolve-Tool 'cmake' @('C:\Program Files\CMake\bin\cmake.exe')

if ($Clean) {
    Write-Host '[clean] 删除旧产物' -ForegroundColor DarkGray
    Remove-Item -Force -ErrorAction SilentlyContinue $tuiExe
    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue (Join-Path $root 'build_qt')
}

function Build-Tui {
    Write-Host '=== 构建 TUI 版 ===' -ForegroundColor Cyan
    $sources = @(
        'src/main.cpp'
        'src/course_io.cpp'
        'src/course_edit.cpp'
        'src/scheduler.cpp'
        'src/db_odbc.cpp'
    ) | ForEach-Object { Join-Path $root $_ }

    $outDir = Split-Path $tuiExe
    if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }

    $compileArgs = @('-std=c++17', '-Wall', '-Wextra', '-O2', '-g',
                     '-I', (Join-Path $root 'include')) + $sources +
                   @('-lodbc32', '-o', $tuiExe)

    & $gxx @compileArgs
    if ($LASTEXITCODE -ne 0) { throw "TUI 编译失败 (g++ exit $LASTEXITCODE)" }
    Write-Host "  [ok] $tuiExe" -ForegroundColor Green
}

function Build-Qt {
    Write-Host '=== 构建 Qt 版 ===' -ForegroundColor Cyan

    $qtDir = $env:QT_DIR
    if (-not $qtDir) {
        $candidate = Get-ChildItem 'C:\Qt' -Directory -ErrorAction SilentlyContinue |
            Sort-Object Name -Descending |
            ForEach-Object { Join-Path $_.FullName 'mingw_64' } |
            Where-Object { Test-Path $_ } |
            Select-Object -First 1
        $qtDir = $candidate
    }
    if (-not $qtDir) { throw '找不到 Qt6 (mingw_64)，请设置环境变量 QT_DIR，例如 C:\Qt\6.10.2\mingw_64' }

    if (-not (Test-Path (Join-Path $qtBuild 'CMakeCache.txt'))) {
        Write-Host "  [configure] Qt = $qtDir" -ForegroundColor DarkGray
        & $cmake -S (Join-Path $root 'qt_ui') -B $qtBuild -G 'MinGW Makefiles' `
            "-DCMAKE_PREFIX_PATH=$qtDir" "-DCMAKE_BUILD_TYPE=Release"
        if ($LASTEXITCODE -ne 0) { throw "CMake 配置失败 (exit $LASTEXITCODE)" }
    }

    & $cmake --build $qtBuild --parallel
    if ($LASTEXITCODE -ne 0) { throw "Qt 编译失败 (exit $LASTEXITCODE)" }
    Write-Host "  [ok] $qtExe" -ForegroundColor Green

    # 首次构建需要把 Qt 运行时 DLL 拷到 exe 旁边，否则双击起不来
    if (-not (Test-Path (Join-Path $qtBuild 'platforms\qwindows.dll'))) {
        $windeployqt = Join-Path $qtDir 'bin\windeployqt.exe'
        if (Test-Path $windeployqt) {
            Write-Host '  [deploy] windeployqt 拷贝 Qt 运行时 DLL' -ForegroundColor DarkGray
            & $windeployqt --no-translations --no-system-d3d-compiler --no-opengl-sw $qtExe
        }
    }
}

switch ($Target) {
    'tui' { Build-Tui }
    'qt'  { Build-Qt }
    'all' { Build-Tui; Build-Qt }
}

Write-Host ''
Write-Host '构建完成。' -ForegroundColor Green
Write-Host '  TUI : 在仓库根目录执行 .\output\scheduler.exe（必须用根目录做工作目录）'
Write-Host '  GUI : 双击 build_qt\scheduler_qt.exe，或用 scripts\run_gui.ps1'
