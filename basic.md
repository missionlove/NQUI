# Build Basics

- qmake 路径：`D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin\qmake.exe`
- qmake bin 目录：`D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin`

## 执行标准

- 每次修改代码后，必须先自行编译检查。
- 若编译报错，必须持续迭代修复，直至没有任何错误后再提交结果。
- 对于 UI 相关需求，不仅要实现功能，还要主动扩展交互细节，以“用户体验优先”为目标进行优化。

## 建议编译命令模板（PowerShell）

- 编译插件 `NFramelessWidget`：
```powershell
$cmd = 'call "C:\Progra~2\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" && cd /d D:\GitHub\NQUI\NQUI\NPlugins\NFramelessWidget && "D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin\qmake.exe" D:\GitHub\NQUI\NQUI\NPlugins\NFramelessWidget\NFramelessWidget.pro && "D:\Qt\Qt5.14.2\Tools\QtCreator\bin\jom.exe"'
cmd /c $cmd
```

- 编译 Demo `Demo1`：
```powershell
$cmd = 'call "C:\Progra~2\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" && cd /d D:\GitHub\NQUI\NQUI\Demos\Demo1 && "D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin\qmake.exe" D:\GitHub\NQUI\NQUI\Demos\Demo1\Demo1.pro && "D:\Qt\Qt5.14.2\Tools\QtCreator\bin\jom.exe"'
cmd /c $cmd
```

- 一键连续编译（先插件后 Demo）：
```powershell
$cmd = 'call "C:\Progra~2\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" && cd /d D:\GitHub\NQUI\NQUI\NPlugins\NFramelessWidget && "D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin\qmake.exe" D:\GitHub\NQUI\NQUI\NPlugins\NFramelessWidget\NFramelessWidget.pro && "D:\Qt\Qt5.14.2\Tools\QtCreator\bin\jom.exe" && cd /d D:\GitHub\NQUI\NQUI\Demos\Demo1 && "D:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin\qmake.exe" D:\GitHub\NQUI\NQUI\Demos\Demo1\Demo1.pro && "D:\Qt\Qt5.14.2\Tools\QtCreator\bin\jom.exe"'
cmd /c $cmd
```
