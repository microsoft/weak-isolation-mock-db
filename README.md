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
From the root directory run the build script

On Linux 

```bash
bash scripts/build.sh
```
Linux Note: cpprestdir default path set in kv_store/http_server/CMakeLists.txt, change if cpprestsdk is installed in a different directory.

On Windows (provide the path to vcpkg.cmake as argument)

```powershell
scripts\build.bat \path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
```



## Running Applications

```bash
./build-files/applications/app_name $num_iterations $consistency_level $debug
```

consistency_level = causal or linear

debug parameter is optional

Example commands:

```bash
./build-files/applications/stack_app 1000 causal
./build-files/applications/courseware_app 100000 linear
./build-files/applications/twitter_app 100 causal debug
```

## Team

This work came out of joint research done by teams at Microsoft Research, India and IRIF, France. The team members include (in alphabetical order):

* Ranadeep Biswas
* Constantin Enea
* Diptanshu Kakwani
* Akash Lal
* Jyothi Vedurada



## Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.



## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft  trademarks or logos is subject to and must follow  [Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general). Use of Microsoft trademarks or logos in modified versions of this  project must not cause confusion or imply Microsoft sponsorship. Any use of third-party trademarks or logos are subject to those  third-party's policies.

