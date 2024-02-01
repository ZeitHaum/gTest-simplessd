# Project Structure

```txt
├── home
    └── simplessd-fullsystem
    └── gTest-simplessd
```

# How to build?
Simply use the following command:
```bash
mkdir build
cd build
cmake .. -DDRAMPOWER_SOURCE_DIR=${HOME}/simplessd-fullsystem/ext/drampower/src;
make
```

If your path of simplessd-fullsystem is different from mine, please modify cmake command to adjust.

# How to run?
```bash
./unit_test
```
