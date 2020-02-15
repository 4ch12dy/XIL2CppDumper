# XIL2CppDumper
> IL2CppDumper writed in C++



这个项目是我理解unity的il2cpp原理时写的一个小玩具，由于IL2CppDumper用C#开发，所以并不支持macOS和linux，il2cpp本身也是C++开发的，为了方便所以我选择了用C++去开发，算是唯一一款跨平台的il2cpp dump工具。目前工具支持il2cpp 24.0/24.1版本，其他版本移植的难度也不大。目前支持Android和iOS 64位可执行文件，32位需自己编译。项目花费了大量时间和精力去开发维护，如果有帮助的话记得点小星星哦，enjoy~（xia0@2019.12.31）

### Usage

```
 __  _____ _     ____   ____             ____                                  
 \ \/ /_ _| |   |___ \ / ___|_ __  _ __ |  _ \ _   _ _ __ ___  _ __   ___ _ __ 
  \  / | || |     __) | |   | '_ \| '_ \| | | | | | | '_ ` _ \| '_ \ / _ \ '__|
  /  \ | || |___ / __/| |___| |_) | |_) | |_| | |_| | | | | | | |_) |  __/ |   
 /_/\_\___|_____|_____|\____| .__/| .__/|____/ \__,_|_| |_| |_| .__/ \___|_|   
                            |_|   |_|                         |_|            



+--------------------------------------------------------------------------------------+
| XIL2CppDumper | a tool of C++ version IL2CppDumper made by xia0@2019                 |
+--------------------------------------------------------------------------------------+
| Info          | version: 0.2 support: iOS[arm64] Android[arm64] il2cpp[24.1/24.0]    |
+--------------------------------------------------------------------------------------+
| Usage         | XIL2CppDumper unity_metadata_file_path il2cpp_so_or_macho_file_path  |
+--------------------------------------------------------------------------------------+
| Blog          | http://4ch12dy.site                                                  |
+--------------------------------------------------------------------------------------+
| Github        | https://github.com/4ch12dy                                           |
+--------------------------------------------------------------------------------------+
| Specail thanks to Perfare's Il2CppDumper:https://github.com/Perfare/Il2CppDumper     |
+--------------------------------------------------------------------------------------+
```

### Features

- Complete DLL restore (except code)

- Supports (ELF) ELF64, MachO64 format

- Supports Unity all version theoretically (test on unity3d(2017-2019))

- Supports automated IDA script generation

  

### Compile

Default use Clion to compile it to x64 progam. 

For android arm32 libil2cpp.so, It need compile to x86 binary for the same  il2cpp header files.



### Dump files

**dump.cs**

dump C# all types and address

**script.py**

ida python script to restore symbol



### Credits

- Perfare - [Il2CppDumper](https://github.com/Perfare/Il2CppDumper)

- nevermoe - [unity_metadata_loader](https://github.com/nevermoe)
