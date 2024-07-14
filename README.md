# Исследование библиотеки [CIMG](https://github.com/GreycLab/CImg) с помощью фаззера [AFL](https://github.com/google/AFL)

## Запуск фаззера

В поддиректории `\example` проекта [CIMG](https://github.com/GreycLab/CImg) находятся программы, использующие методы и функции этой библиотеки. Одна из них была выбрана в качестве `fuzzing target`.

Программа на вход принимает файлы формата `*.tiff`. 


При наличии исходного кода необходимо скомпилировать программу с помощью afl-gcc, так будет добавлени инструментарий фаззера, влияющий на производительность (afl-gcc/g++/clang является как бы заменой gcc/g++/clang).

~~~
[environment vars]/home/ask0later/AFL__/AFL/afl-gcc [флаги]target.c -o target 
~~~

После этого можно запускаем фаззер при помощи:
```
./afl-fuzz -i testcase_dir -o findings_dir /path/to/program [...params...]
```
или
```
./afl-fuzz -i testcase_dir -o findings_dir /path/to/program @@
```

Второй случай нужен для того, если программа принимает аргументы через файл.

В папке с флагом -i находится корпус - корректные входные данные.

Экран состояния фаззера:

![fuzzer](/img_for_README/fuzzer.jpg)

## [Распараллеленный фаззинг](https://github.com/google/AFL/blob/master/docs/parallel_fuzzing.txt)

Также можно запустить несколько процессов для разных или одного target'ов при помощи следующих комманд.

В одном терминале прописываем главный (М) процесс.
```
afl-fuzz -i testcase_dir -o findings_dir -M fuzzer01 [...other stuff...]
```
В других терминалах вторичные:

```
afl-fuzz -i testcase_dir -o sync_dir -S fuzzer02 [...other stuff...]
afl-fuzz -i testcase_dir -o sync_dir -S fuzzer03 [...other stuff...]
```

## [Кастомные мутаторы](https://github.com/AFLplusplus/AFLplusplus/tree/stable/custom_mutators)

Во время выполнения фаззер мутирует входные данные (корпус) для бОльшего покрытия программы. Зная особенности формата файла `*.tiff` можно написать свой ``mutator`` и запустить с ним фаззер.

Файл `mutator.с` с измененными функциями компилируем в динамический разделяемый объектный файл при помощи команды:
```
gcc -shared -Wall -O3 mutator.c -o mutator.so
```
Далее при запуске фаззера в команду необходимо добавить переменную AFL_CUSTOM_MUTATOR_LIBRARY="/full/patg/to/mutator.so". Если необходимо использовать только свой кастомный фаззер, то ещё определим AFL_CUSTOM_MUTATOR_ONLY=1.


Экран состояния с использованием кастомного мутатора и встроеного:

![cust_internal](/img_for_README/cast_plus_internal.jpg)

# Результаты
Если сравнивать результаты тестирования фаззера с импользованием дефолтного встроеного мутатора и его же с кастомным мутатором, то видно, что кол-во `crashes` увеличилось. Эффективность тестирования возросла, это может использоваться в работе.


## Литература
1. AFL: https://github.com/google/AFL.git.
2. AFLplusplus: https://github.com/AFLplusplus/AFLplusplus.git.
3. greybox-фаззинг: https://habr.com/ru/company/bizone/blog/570312/ и https://habr.com/ru/company/bizone/blog/570534/.
4. Detailed TIFF image file formet: https://programmersought.com/article/25991320372/.