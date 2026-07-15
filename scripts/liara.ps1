<#
.SYNOPSIS
  Liara Engine CLI Wrapper for Windows.
#>
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$PythonScript = Join-Path $ScriptDir "liara.py"

if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
    Write-Error "python was not found in your PATH. Please install Python 3."
    exit 1
}

& python $PythonScript $args
exit $LASTEXITCODE