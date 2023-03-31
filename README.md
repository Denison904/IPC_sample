# Shared memory via signal

1. Компиляция
```
gcc main.c shared_memory.c -lrt -o sh_signal
```
2. Запуск
```
./sh_signal -c COUNT_OF_MESSAGE -s SIZE_OF_MESSAGE
```
`COUNT_OF_MESSAGE` - целое число больше нуля, отвечает за количество сообщений.
`SIZE_OF_MESSAGE`  - целое число больше нуля, отвечает за размер сообщения сообщения.

## Пример
```
./sh_signal -c 200 -s 200
MAIN[453]: pid c:0, s:454
MAIN[453]: pid c:455, s:454
READER[454]: TIME: sec: 0, nanosec: 18709344
READER[454]: LAST MESSAGE: "message #199"
```