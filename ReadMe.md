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

# Args
In order to reduce the size of the test samples, I modified the configuration file. The new file name is unit_test.cfg and is also the default configuration file used. If you want to use other configuration files, please modify the main function of test.cc yourself.

Notice that the arg --testconfig is already used, you can just change the path of config files and the corresponding parameters in SetUpTestCases function.
