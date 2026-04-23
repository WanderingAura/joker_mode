Get-ChildItem -Path $args[0] -Recurse -Filter *.c -File |
Where-Object { $_.Name -ne "main.c" -and $_.Name -notlike "*_linux.c" } |
Sort-Object FullName |
ForEach-Object {
    "#include `"$($_.FullName)`""
} > soc_dll_unity_windows.c