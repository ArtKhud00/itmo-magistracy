# Файл с шаблонами и примерам команд

## Установка [ansible](https://docs.ansible.com/ansible/latest/installation_guide/intro_installation.html)

```sh
python3 -m pip install --user ansible
```
Возможно понадобится добавить локаль, в случае появления ошибок при работе с acii символами:

```sh
export LANG=en_US.UTF-8
pip3 install --user ansible
```

## [Команда на конфигурирование установщика nginx и его компиляции](https://nginx.org/ru/docs/configure.html)
```shell
./configure \
    --prefix=/etc/usr/nginx \
    --sbin-path=/etc/usr/nginx/nginx \
    --modules-path=/etc/usr/nginx/modules \
    --conf-path=/etc/usr/nginx/conf/nginx.conf \
    --pid-path=/etc/usr/nginx/nginx.pid \
    --error-log-path=/etc/usr/nginx/logs/error.log \
    --http-log-path=/etc/usr/nginx/logs/access.log \
    --with-pcre && \
make install clean
```

## [Пример шаблона конфигурации nginx](https://nginx.org/ru/docs/beginners_guide.html)

```jinja2

#user  nobody;
worker_processes 1;

#error_log  logs/error.log;
#error_log  logs/error.log  notice;
#error_log  logs/error.log  info;

#pid        logs/nginx.pid;
events {
    worker_connections 1024;
}


http {
    include mime.types;
    default_type application/octet-stream;

    #log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
    #                  '$status $body_bytes_sent "$http_referer" '
    #                  '"$http_user_agent" "$http_x_forwarded_for"';

    #access_log  logs/access.log  main;
    sendfile on;
    #tcp_nopush     on;

    #keepalive_timeout  0;
    keepalive_timeout 65;

    #gzip  on;

    server {
        listen {{ nginx_port }};
        server_name localhost;

        location / {
            root html;
            index index.html index.htm;
        }

        error_page 500 502 503 504 /50x.html;
        location = /50x.html {
            root html;
        }
    }
}

```

## Пример html шаблона статического сайта

```jinja2
<!DOCTYPE html>
<html lang="en">

<head>
    <title>Welcome to nginx-{{ nginx_version }}!</title>
</head>
<style>
    html {
        color-scheme: light dark;
    }

    body {
        width: 35em;
        margin: 0 auto;
        font-family: Tahoma, Verdana, Arial, sans-serif;
    }
</style>
<body>

    <h3>Welcome to nginx: {{ nginx_version }}</h1>


    <ul>
        <li>
            <span><b>Environment</b>: {{ deploy_environment }}</span>
        </li>
        <li>
            <span><b>Current inventory host</b>: {{ inventory_hostname }}</span>
        </li>
        <li>
            <span><b>All services</b>:</span>
            <ul>
                {% for item in all_services -%}
                <li>
                    <span><b>{{ item }}</b></span>
                </li>
                {%- endfor %}
            </ul>
        </li>
    </ul>

</body>

</html>
```
