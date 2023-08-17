# Лабораторная работа №1. Автоматическое распараллеливание программ
В рамках данной лабораторной работы необходимо написать программу и скомпилировать ее разными способами:
  - Без использования автоматического распараллеливания с помощью команды:
```
gcc -O3 -Wall -Werror -o lab1-seq lab1.c
```
  - С использованием встроенного в gcc средства автоматического распараллеливания Graphite с помощью следующей команды:
```
gcc -O3 -Wall -Werror -floop-parallelize-all -ftree-parallelize-loops=K lab1.c -o lab1-par-K
```
Где K должно принять как минимум 4 значения: 1, меньше количества ядер, равное количеству физических ядер и больше количества физических ядер.
## Методика проведения эксперимента
Для начала необходимо определить значения N1 и N2. Чтобы это сделать будем запускать программу для различных N (размерность массива). N1 - это значение N, при котором время выполнения превысит 0.01 с. N2 - значение N, при котором время выполнения превысит 5 с.
  - Запускать последовательную программу для значений `N = N1, N1 + ∆, N1 + 2∆, N1 + 3∆, . . . , N2` и записывать значения времени delta_ms(N) в функцию seq(N);
  - Запускать параллельные прогаммы, соответствующие различным значениям K, для значений `N = N1, N1 + ∆, N1 + 2∆, N1 + 3∆, . . . , N2` и записывать значения времени delta_ms в функцию par_K(N);
  - Значение `∆` выбрать так: `∆ = (N2 − N1)/10`.

## Дополнительное задание
Провести аналогичные описанным эксперименты, используя вместо `gcc` компилятор `Intel ICC`. Для автоматического распараллеливания нужно использовать следующую команду:
```
icc -parallel -par-threashold0 -par-num-threads=K -o lab1-icc-par-K lab1.c
```