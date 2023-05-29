FROM python:3.10-alpine

ARG USER=app
ARG UID=1001
ARG GID=1001

RUN pip install --no-cache-dir Flask==2.2.*

RUN addgroup -g ${GID} -S ${USER} \
   && adduser -u ${UID} -S ${USER} -G ${USER} \
   && mkdir -p /app \
   && chown -R ${USER}:${USER} /app
USER ${USER}

WORKDIR /app


ENV FLASK_APP=server.py \
   FLASK_RUN_HOST="0.0.0.0" \
   FLASK_RUN_PORT="8000" \
   PYTHONUNBUFFERED=1

COPY --chown=$USER:$USER . /app

EXPOSE 8000

CMD ["flask", "run"]