# Weak Isolation Mock DB

An in-memory mock storage system for systematic testing of storage-backed applications under various isolation levels. WeakIsolationMockDB generates weaker behaviors - subject to the chosen isolation level, which occur rarely in real-world databases. It allows application developers to easily test their applications under various corner cases. WeakIsolationMockDB currently supports key-value interface with multiple isolation levels.



## Dependencies

### Cpprestsdk

https://github.com/microsoft/cpprestsdk

The core library doesn't require this dependency, however applications and HTTP-server modules require cpprestsdk.

**How to install**

With [vcpkg](https://github.com/Microsoft/vcpkg) on Windows

```powershell
PS> vcpkg install cpprestsdk cpprestsdk:x64-windows
```

With [apt-get](https://launchpad.net/ubuntu/+source/casablanca/2.8.0-2build2) on Debian/Ubuntu

```bash
$ sudo apt-get install libcpprest-dev
```



## Build

On Linux

```bash
bash scripts/build.sh
```

On Windows

```powershell
scripts\build.bat
```



## Running Applications

```bash
./applications/kv_store_apps $log_file $num_iterations $consistency_level $test_type $test_cases
```

consistency_level=causal or linear

test_type=fixed or random (For fixed use test_cases=1)

Example commands:

```bash
./applications/kv_store_apps app.log 100000 causal random 50
./applications/kv_store_apps app.log 100000 linear random 50
./applications/kv_store_apps app.log 100000 causal fixed 1
./applications/kv_store_apps app.log 1000 linear fixed 1
```



By default it runs the stack application, to change application modify applications/CmakeLists.txt and build again.



## Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.



## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft  trademarks or logos is subject to and must follow  [Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general). Use of Microsoft trademarks or logos in modified versions of this  project must not cause confusion or imply Microsoft sponsorship. Any use of third-party trademarks or logos are subject to those  third-party's policies.

