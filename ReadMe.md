# Project Structure

```txt
├── home
    └── simplessd-fullsystem
    └── gTest-simplessd
```

If your path of simplessd-fullsystem, please change to CmakeLists to adjust.
The Code Need to be changed is seems like below:

```CMakeLists
add_compile_options(-DDRAMPOWER_SOURCE_DIR=${HOME}/simplessd-fullsystem/ext/drampower/src)
```

# How to build?
Simply use the following command:
```bash
mkdir build
cd build
cmake ..
make
```

# How to run?
```bash
./unit_test
```
