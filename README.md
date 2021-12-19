# dll-hot-reload

Small utility DLL that loads and reloads a given DLL when it's updated on disk.
This can be useful when developing and debugging DLLs which are meant to be
injected (*e.g.*, game mods, in-process debugging tools).

**Features**  
* Configuration via INI file
* Can use manual mapping (with Blackbone)

## How to Build

```
$ cmake -B build
$ cmake --build build --config Release -- -maxcpucount
```


## How to Use

Create a `dll_loader.ini` file in the working directory of the host application
the DLL will be loaded into.  
An example INI file can be found in the `examples` subfolder.