# Kubernetes_Homework

## Задачи

1. В качестве web-сервера для простоты можно воспользоваться Python: “python -m http.server 8000”. 
    1. Добавьте эту команду в инструкцию CMD в Dockerfile.
2. Создать Dockerfile на основе “python:3.10-alpine”, в котором.
    1. Создать каталог “/app” и назначить его как WORKDIR.
    2. Добавить в него файл, содержащий текст “Hello world”.
    3. Обеспечить запуск web-сервера от имени пользователя с “uid 1001”.
3. Собрать Docker image с tag “1.0.0”.
4. Запустить Docker container и проверить, что web-приложение работает.
5. Выложить image на Docker Hub.
6. Создать Kubernetes Deployment manifest, запускающий container из созданного image.
    1. Имя Deployment должно быть “web”.
    2. Количество запущенных реплик должно равняться двум.
    3. Добавить использование Probes.
7. Установить manifest в кластер Kubernetes.
8. Обеспечить доступ к web-приложению внутри кластера и проверить его работу
    1. Воспользоваться командой kubectl port-forward: “kubectl port-forward --address 0.0.0.0 deployment/web 8080:8000”.
    2. Перейти по адресу http://127.0.0.1:8080/hello.html.

## Структура файлов проекта:

```shell
$ tree -I 'venv'
.
├── README.md
├── deployment.yaml
├── docker-compose.yml
└── web-server
    ├── Dockerfile
    ├── server.py
    └── templates
        └── hello.html
```

## Шаг 1

В качестве фреймворка для сервера использован [Python Flask](https://flask.palletsprojects.com/). Код `web-server/server.py`:

```python
from flask import Flask, render_template

app = Flask(__name__,template_folder='templates')


@app.route('/')
@app.route('/hello.html')
def hello_world():
    """Hello world function

    Returns template:
        hello.html
    """
    return render_template('hello.html')

```

hello.html:

```html
<!DOCTYPE html>
<html lang="en">
  <head>
    <title>
      Hello World!
    </title>
  </head>
  <body>
    Hello World!
  </body>
</html>
```

## Шаг 2

В файле `web-server/Dockerfile` описана сборка и описан запуск сервера:

```Dockerfile
# Based image
FROM python:3.10-alpine

# Variables required for enviroment creation
ARG USER=app
ARG UID=1001
ARG GID=1001

# Framework installation
RUN pip install --no-cache-dir Flask==2.2.*

# Creating OS user and home directory
RUN addgroup -g ${GID} -S ${USER} \
   && adduser -u ${UID} -S ${USER} -G ${USER} \
   && mkdir -p /app \
   && chown -R ${USER}:${USER} /app
USER ${USER}

# Entering home dir /app
WORKDIR /app


# Enviroment variables required for launching web-application
ENV FLASK_APP=server.py \
   FLASK_RUN_HOST="0.0.0.0" \
   FLASK_RUN_PORT="8000" \
   PYTHONUNBUFFERED=1

# Copying application code to home directory
COPY --chown=$USER:$USER . /app

# Publishing the port that the application is listening on
EXPOSE 8000

# Application launch command
CMD ["flask", "run"]
```

## Шаг 3

Используя приведённую команду ниже создаём образ - Docker image:

```shell
sudo docker build -t artkhud20/web-server:1.0.0 --network host -t artkhud20/web-server:latest web-server 
```

Список image:

```shell
$ sudo docker image ls
REPOSITORY                    TAG       IMAGE ID       CREATED             SIZE
artkhud20/web-server          1.0.0     fc7ad8271574   32 minutes ago   60.7MB
artkhud20/web-server          latest    e269a7140bd1   32 minutes ago   60.7MB
```

Теперь для удобства создадим файл `docker-compose.yml`, где опишем конфигурацию сборки и запуска приложений.

```yml
# Версия API Docker compose
version: "3"

# Раздел, в котором описываются приложения (сервисы).
services:

  # Раздел для описания приложения 'server'.
  server:

    # Имя image tag
    image: artkhud20/web-server:1.0.0
 
    # Параметры сборки Docker image.
    build: 
      # Путь к Dockerfile,
      context: web-server/
      # Использовать host-сеть при сборке,
      network: host

    # Перенаправление портов из Docker container на host-машину.
    ports:
      - 8000:8000

    # Имя user, используемого в image,
    user: "1001"

    # Используемый тип сети при запуске container.
    network_mode: host

    # Проверка готовности приложения к работе. Параметр "--spider" означает: не загружать url, 
    # а только проверить его наличие.
    healthcheck:
        test: wget --no-verbose --tries=1 --spider http://localhost:8000 || exit 1
        interval: 5s
        timeout: 5s
        retries: 5
```

Соберем Docker images при помощи docker compose.

```shell
$ sudo docker compose build
[+] Building 1.3s (10/10) FINISHED                                                                                                                                                                                                                                                                         
 => [internal] load build definition from Dockerfile                                                                                                                                                                                                                                                  0.0s
 => => transferring dockerfile: 493B                                                                                                                                                                                                                                                                  0.0s
 => [internal] load .dockerignore                                                                                                                                                                                                                                                                     0.0s
 => => transferring context: 2B                                                                                                                                                                                                                                                                       0.0s
 => [internal] load metadata for docker.io/library/python:3.10-alpine                                                                                                                                                                                                                                 1.1s
 => [internal] load build context                                                                                                                                                                                                                                                                     0.0s
 => => transferring context: 591B                                                                                                                                                                                                                                                                     0.0s
 => [1/5] FROM docker.io/library/python:3.10-alpine@sha256:def82962a6ee048e54b5bec2fcdfd4aada4a907277ba6b0300f18c836d27f095                                                                                                                                                                           0.0s
 => CACHED [2/5] RUN pip install --no-cache-dir Flask==2.2.*                                                                                                                                                                                                                                          0.0s
 => CACHED [3/5] RUN addgroup -g 1001 -S app    && adduser -u 1001 -S app -G app    && mkdir -p /app    && chown -R app:app /app                                                                                                                                                                      0.0s
 => CACHED [4/5] WORKDIR /app                                                                                                                                                                                                                                                                         0.0s
 => [5/5] COPY --chown=app:app . /app                                                                                                                                                                                                                                                                 0.1s
 => exporting to image                                                                                                                                                                                                                                                                                0.1s
 => => exporting layers                                                                                                                                                                                                                                                                               0.0s
 => => writing image sha256:fc7ad827157490f2b57af977e53d8fc87ce2d3a824ee656640fd4adf51fabdfa                                                                                                                                                                                                          0.0s
 => => naming to docker.io/artkhud20/web-server:1.0.0 
 ```
## Шаг 4

Запустим веб-приложение при помощи docker compose.

```shell
$ docker compose up
[+] Running 2/0
✔ Container devops1-server-1                                        Created                                                                                                                                                                                                                         0.1s 
 ! server Published ports are discarded when using host network mode                                                                                                                                                                                                                                  0.0s 
Attaching to devops1-server-1
devops1-server-1  |  * Serving Flask app 'server.py'
devops1-server-1  |  * Debug mode: off
devops1-server-1  | WARNING: This is a development server. Do not use it in a production deployment. Use a production WSGI server instead.
devops1-server-1  |  * Running on all addresses (0.0.0.0)
devops1-server-1  |  * Running on http://127.0.0.1:8000
devops1-server-1  |  * Running on http://10.0.2.15:8000
devops1-server-1  | Press CTRL+C to quit
devops1-server-1  | 127.0.0.1 - - [14/May/2023 17:06:46] "GET / HTTP/1.1" 200 -
```

В отдельном терминале проверим веб-приложение.

```shell
$ curl http://127.0.0.1:8000/hello.html
<!DOCTYPE html>
<html lang="en">
  <head>
    <title>
      Hello World!
    </title>
  </head>
  <body>
    Hello World!
  </body>
</html>
```

Веб-приложение работает корректно.

## Шаг 5

Выложим на dockerhub.
Получим Access Token в `https://hub.docker.com/settings/security`

Авторизуемся в Docker registry. В качестве пароля введем Access Token.

```shell
$ sudo docker login -u artkhud20

Password:
Login Succeeded
```

Отправим Docker image в Registry Docker Hub.

```shell
$ sudo docker push artkhud20/web-server:1.0.0
```

## Шаг 6

Создадим Kubernetes Deployment manifest, запускающий container из созданного image. Кол-во реплик равно 2.

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: web
  labels:
    app: server
spec: 
  replicas: 2
  selector:
    matchLabels:
      app: server
  template:
    metadata:
      labels:
        app: server
    spec:
      containers:
        - name: web-server
          image: artkhud20/web-server:1.0.0
          ports:
            - containerPort: 8000
```

Добавим Proba.

```yml
          livenessProbe:
            httpGet:
              path: /hello.html
              port: 8000
            initialDelaySeconds: 3
            periodSeconds: 3
```

## Шаг 7

Установим manifest в кластер Kubernetes.

```shell
$ kubectl apply --filename deployment.yaml --namespace default
deployment.apps/web unchanged
$ kubectl get pods --namespace default
NAME                  READY   STATUS    RESTARTS   AGE
web-6bc9747666-ctm29   1/1     Running   0          6m16s
web-6bc9747666-d67bz   1/1     Running   0          6m1s
```

## Шаг 8 

Обеспечим доступ к web-приложению внутри кластера и проверим его работу.

```shell
$ kubectl port-forward --address 0.0.0.0 deployment/web 8080:8000
Forwarding from 0.0.0.0:8080 -> 8000
Handling connection for 8080
```

```shell
$ curl http://127.0.0.1:8080/hello.html
<!DOCTYPE html>
<html lang="en">
  <head>
    <title>
      Hello World!
    </title>
  </head>
  <body>
    Hello World!
  </body>
</html>
```

Выведем информацию об развертывании.

```shell
$ kubectl describe deployment web
Name:                   web
Namespace:              default
CreationTimestamp:      Sun, 14 May 2023 20:04:22 +0300
Labels:                 app=server
Annotations:            deployment.kubernetes.io/revision: 1
Selector:               app=server
Replicas:               2 desired | 2 updated | 2 total | 2 available | 0 unavailable
StrategyType:           RollingUpdate
MinReadySeconds:        0
RollingUpdateStrategy:  25% max unavailable, 25% max surge
Pod Template:
  Labels:  app=server
  Containers:
   web-server:
    Image:        artkhud20/web-server:1.0.0
    Port:         8000/TCP
    Host Port:    0/TCP
    Liveness:     http-get http://:8000/hello.html delay=3s timeout=1s period=3s #success=1 #failure=3
    Environment:  <none>
    Mounts:       <none>
  Volumes:        <none>
Conditions:
  Type           Status  Reason
  ----           ------  ------
  Available      True    MinimumReplicasAvailable
  Progressing    True    NewReplicaSetAvailable
OldReplicaSets:  <none>
NewReplicaSet:   web-6bc9747666 (2/2 replicas created)
Events:          <none>
```


