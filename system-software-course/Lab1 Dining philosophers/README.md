# Лабораторная работа №1. Обедающие философы
Решение задачи [1226.The Dining Philosophers](https://leetcode.com/problems/the-dining-philosophers/).

В этой задаче всегда 5 философов. Когда философ хочет кушать, то вызывает метод:
```c++
void wantsToEat(int philosopher,
				function<void()> pickLeftFork,
				function<void()> pickRightFork,
				function<void()> eat,
				function<void()> putLeftFork,
				function<void()> putRightFork);
```
 
 - `philosopher` - число от 0 до 4, «имя» философа
 - `pickLeftFork`, `pickRightFork`, `eat`, `putLeftFork`, `putRightFork` - действия которые он может сделать

Эту задачу можно решать методом атомарного захвата вилок или любым другим методом, который вам больше нравится. Если одновременно могут есть нескоько фиософов,
то они должны это делать