#xban

Fork from http://sourceforge.net/projects/xban/

###Howto Build 
* Download sourcecode e.g.
```
wget "https://github.com/candux/xban/archive/master.zip"
unzip master.zip
```
* Create and enter build directory
```
mkdir xban-build
cd xban-build
```
* Execute CMake
```
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ../xban-master/
```
* Build code
```
make
```

You can now run the programm
```
tbancontrol/tbancontrol gethwinfo
```
