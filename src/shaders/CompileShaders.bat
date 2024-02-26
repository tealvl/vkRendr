:: Этот скрипт компилирует файлы .frag и .vert в SPIR-V формат с помощью glslc 
:: (glslc должен быть в переменных окружения)

@echo off

for %%i in (*.frag *.vert) do (
    glslc %%i -o %%~ni.spv
)


