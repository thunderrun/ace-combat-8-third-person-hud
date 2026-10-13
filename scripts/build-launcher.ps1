param([switch]$Test, [string]$Output = '')
$ErrorActionPreference = 'Stop'
$taskRoot = Split-Path -Parent $PSScriptRoot
$source = Join-Path $taskRoot 'src\launcher.cpp'
$taskOutput = if ([string]::IsNullOrWhiteSpace($Output)) { Join-Path $taskRoot 'build\AC8NoIntroHUD.exe' } else { [IO.Path]::GetFullPath($Output) }
$buildDirectory = Join-Path $taskRoot 'build'
$devCommand = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat'
if (-not (Test-Path -LiteralPath $devCommand)) { throw 'Visual Studio 2022 C++ Build Tools are required.' }
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $taskOutput), $buildDirectory | Out-Null
$compileCommand = 'call "{0}" -arch=x64 -host_arch=x64 >nul && cl /nologo /std:c++17 /EHsc /W4 /O2 /MT /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS /Fe:"{1}" /Fo:"{2}\launcher.obj" "{3}" /link /DYNAMICBASE /NXCOMPAT /HIGHENTROPYVA /SUBSYSTEM:CONSOLE bcrypt.lib' -f $devCommand, $taskOutput, $buildDirectory, $source
& $env:ComSpec /d /s /c $compileCommand
if ($LASTEXITCODE -ne 0) { throw "Launcher compilation failed with exit code $LASTEXITCODE." }
if ($Test) {
    & $taskOutput --self-test --output (Join-Path $taskRoot 'verification\self-test')
    if ($LASTEXITCODE -ne 0) { throw 'Launcher cross-process self-test failed.' }
}
Get-Item -LiteralPath $taskOutput | Select-Object FullName, Length
Get-FileHash -LiteralPath $taskOutput -Algorithm SHA256
