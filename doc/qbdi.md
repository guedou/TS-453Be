# Install in a Docker image

```
sudo sysctl kernel.yama.ptrace_scope=0

docker pull ubuntu:18.10

docker run --name qbdi guedou/ubuntu

cat Dockerfile 
FROM ubuntu:18.10
CMD tail -f /dev/null
docker build . -t guedou/ubuntu:18.10

docker run --name qbdi --privileged --detach guedou/ubuntu:18.10

docker exec -it qbdi bash
apt update ; apt install wget vim python-pip npm cmake
# install qbdi & frida

docker commit qbdi guedou/qbdi

LD_PRELOAD=/usr/lib/libpyqbdi.so PYQBDI_TOOL=./pyqbdi_trace.py ./panqlk temperature > qbdi_temperature.txt
```

# use

```
docker run --rm -it -v $PWD:/qnap --cap-add=SYS_RAWIO guedou/qbdi  bash
LD_PRELOAD=/usr/lib/libpyqbdi.so PYQBDI_TOOL=./pyqbdi_trace.py ./panqlk fan
```
