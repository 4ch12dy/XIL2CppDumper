# XIL2CppDumper
IL2CppDumper writed in C++

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

- Supports automated IDA script generation (coming soon)

  

### Compile

Default use Clion to compile it to x64 progam. 

For android arm32 libil2cpp.so, It need compile to x86 binary for the same  il2cpp header files.



### Dump files

**dump.cs**

dump C# all types and address



### Credits

- Perfare - [Il2CppDumper](https://github.com/Perfare/Il2CppDumper)

- nevermoe - [unity_metadata_loader](https://github.com/nevermoe)