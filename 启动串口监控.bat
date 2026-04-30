@echo off
chcp 65001 >nul
powershell -Command "[Console]::OutputEncoding = [System.Text.Encoding]::UTF8; [Console]::InputEncoding = [System.Text.Encoding]::UTF8; pio device monitor | ForEach-Object { Write-Host $_; $_ | Out-File -FilePath '串口日志.txt' -Encoding UTF8 -Append }"