# Sample IPC

## Сборка проекта
1. Создать папку для сборки проекта
```
mkdir build
```
2. Перейти в папку `build`
```
cd build
```
3. Сконфигурировать сборку
```
cmake ..
```
4. Собрать исполняемые файлы
```
make
```

## Shared memory via signal

1. Запуск
```
./signal/shm_signal -c COUNT_OF_MESSAGE -s SIZE_OF_MESSAGE
```
`COUNT_OF_MESSAGE` - целое число больше нуля, отвечает за количество сообщений.
`SIZE_OF_MESSAGE`  - целое число больше нуля, отвечает за размер сообщения сообщения.

## Пример
```
./signal/shm_signal -c 200 -s 200
MAIN[2781]: pid c:0, s:2782
MAIN[2781]: pid c:2783, s:2782
WRITER[2783]: START_TIME: s: 20395,  n:568939575
READER[2782]: START_TIME: s: 20395,  n:568899307
WRITER[2783]: END_TIME: s: 20395,  n:584931399
READER[2782]: END_TIME: s: 20395,  n:584942415
WRITER[2783]: DURATION: sec: 0, nanosec: 15991824
READER[2782]: DURATION: sec: 0, nanosec: 16043108
READER[2782]: Message per seconds: 12466.412109
WRITER[2783]: Message per seconds: 12506.390625
```

## Shared memory via eventfd

1. Запуск
```
./event/shm_event -c COUNT_OF_MESSAGE -s SIZE_OF_MESSAGE
```
`COUNT_OF_MESSAGE` - целое число больше нуля, отвечает за количество сообщений.
`SIZE_OF_MESSAGE`  - целое число больше нуля, отвечает за размер сообщения сообщения.

## Пример
```
./event/shm_event -c 200 -s 200
READER[2785]: Start reader
WRITER[2786]: Start writer
WRITER[2786]: START_TIME: s: 20566,  n:985684440
READER[2785]: START_TIME: s: 20566,  n:985400903
WRITER[2786]: END_TIME: s: 20566,  n:78683
WRITER[2786]: DURATION: sec: 0, nanosec: 14394243
READER[2785]: END_TIME: s: 20566,  n:84083
WRITER[2786]: Message per seconds: 13894.443359
READER[2785]: DURATION: sec: 0, nanosec: 14683180
READER[2785]: Message per seconds: 13621.028320
```

## Domain datagram socket
1. Запуск
```
./event/shm_event -c COUNT_OF_MESSAGE -s SIZE_OF_MESSAGE
```
`COUNT_OF_MESSAGE` - целое число больше нуля, отвечает за количество сообщений.
`SIZE_OF_MESSAGE`  - целое число больше нуля, отвечает за размер сообщения сообщения.

## Пример
```
./socket/socket -c 200 -s 200
WRITER[2789]: START_TIME: s: 20643,  n:543479975
READER[2788]: START_TIME: s: 20643,  n:543863538
WRITER[2789]: END_TIME: s: 20643,  n:544044710
READER[2788]: END_TIME: s: 20643,  n:544060081
WRITER[2789]: DURATION: sec: 0, nanosec: 564735
READER[2788]: DURATION: sec: 0, nanosec: 196543
WRITER[2789]: Message per seconds: 354148.406250
READER[2788]: Message per seconds: 1017589.062500
```