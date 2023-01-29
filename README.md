# ncway

## [server](/server)

## [client](/client)

# Prerequsite

```bash
$ sudo apt install libwayland-dev
$ sudo apt install libegl1-mesa-dev
$ sudo apt install cmake
$ sudo apt install g++10
$ sudo apt install build-essential
```

 If you are interested in the Ubuntu packaging,
 you should install the debhelper package.

```bash
$ sudo apt install debhelper
```

# Docker

 I prepared a dockerfile to build the docker image for building the package
 If you want to build a new image, use the Dockerfile
```bash
$ docker build . -t registry.nicesj.com/ncway:latest
$ docker run -it -v $PWD:/root registry.nicesj.com/ncway:latest /bin/bash
```

 You can pull the docker image of course
```bash
$ docker pull registry.nicesj.com/ncway:latest
$ docker run -it -v $PWD:/root registry.nicesj.com/ncway:latest /bin/bash
```

# Debian packaging (Ubuntu package generation)
```bash
$ dpkg-buildpackage -uc -us -b
```

# References

 * [Small Wayland Compositor](https://github.com/michaelforney/swc)
 * [An introduction to Wayland](https://drewdevault.com/2017/06/10/Introduction-to-Wayland.html)
