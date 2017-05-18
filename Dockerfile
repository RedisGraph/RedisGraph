FROM ubuntu

RUN apt-get -y update && apt-get install -y build-essential
RUN apt-get install -y wget cmake
RUN cd /tmp
RUN wget https://github.com/antirez/redis/archive/4.0-rc2.tar.gz
RUN tar xvzf 4.0-rc2.tar.gz
RUN cd redis-4.0-rc2 && make
RUN cd redis-4.0-rc2 && make install
COPY . /redis-graph
RUN cd /redis-graph && cmake . && make module

EXPOSE 6379

CMD ["/usr/local/bin/redis-server", "--bind", "0.0.0.0", "--loadmodule", "/redis-graph/src/libmodule.so"]