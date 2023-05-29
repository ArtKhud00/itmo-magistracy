# Разработка клиент - серверного приложения

Необходимо реализовать на языке `C` клиент - серверное взаимодействие, для передачи данных использовать протокол TCP.

Клиент должен принимать два или более аргументов - имя хоста и имена каталогов. Клиент устанавливает TCP соединение с сервером по указанному имени хоста, после чего запрашивает
у сервера содержимое указанных каталогов. Полученный список выводится в стандартный поток вывода.

Сервер должен обеспечивать возможность одновременного обслуживания нескольких клиентов. Протокол взаимодействия с сервером - текстовый. Необходимо предусмотреть возможность взаимодействия
с сервером используя команду `telnet`.

В рамках задания необходимо для каждого соединения создавать новый поток.
