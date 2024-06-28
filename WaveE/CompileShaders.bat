@echo off
setlocal

set "current_dir=%~dp0"
set "exe_path=x64\Debug\ShaderCompiler.exe"

start "" %current_dir%%exe_path% %current_dir%Resources\Shaders %current_dir%Resources\Shaders

endlocal
