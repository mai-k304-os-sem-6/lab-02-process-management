#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Функции для встроеных команд оболочки
int psh_cd(char **args); // Переход в другую деректорию
int psh_help(char **args); // Информация об оболочке
int psh_exit(char **args); // Выход из оболочки

char *builtin_str[] = { // Список втроеных команд
  "cd", // Переход в другую деректорию
  "help", // Информация об оболочке
  "exit" // Выход из оболочки
};

int (*builtin_func[]) (char **) = { // Указатели на функции встроеных команд
  &psh_cd, // Переход в другую деректорию
  &psh_help, // Информация об оболочке
  &psh_exit // Выход из оболочки
};

int psh_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }; // Возвращение размера команды

int psh_cd(char **args) { // Переход в другую деректорию
  if (args[1] == NULL) { // Если не было аргумента
    fprintf(stderr, "lsh: ожидается аргумент для \"cd\"\n");
  } else { // Если аргумент был
    if (chdir(args[1]) != 0) { // Проверка на директорию
      perror("psh"); // Переход
    }
  }
  return 1;
}

int psh_help(char **args) { // Информация об оболочке
  printf("Пысларь Александр PSH\n");
  printf("МАИ 304к 310 группа 2024г\n");
  printf("Введите имена программ и аргументы и нажмите enter\n");
  printf("В систему встроены следующее команды:\n");

  for (int i = 0; i < psh_num_builtins(); i++) printf("  %s\n", builtin_str[i]); // Вывод встроеных команд

  printf("P.S. отдальная благодарность Титову Ю. П. за столь интелектуальные задания\n");
  return 1;
}

int psh_exit(char **args) { return 0; }; // Функция завершения

int psh_launch(char **args) {
  pid_t pid, wpid; // Идентификатор процесса
  int status; // Статус процесса

  pid = fork(); // Создаём дочерний процесс
  if (pid == 0) { // Дочерний процесс
    if (execvp(args[0], args) == -1) { // Если ошибка в аргументах
      perror("psh"); // Сообщение об ошибки
    }
    exit(EXIT_FAILURE); // Выход из программы
  } else if (pid < 0) { // Ошибка при создании дочернего процесса
    perror("psh"); // Сообщение об ошибки
  } else {
    do { // Родительский процесс
      wpid = waitpid(pid, &status, WUNTRACED); // Ожидания изменения состояния процесса
    } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // Пока процесс не завершился
  }
  return 1;
}

int psh_execute(char **args) { // Запуск команды
  if (args[0] == NULL) { return 1; } // Если команды не было
  for (int i = 0; i < psh_num_builtins(); i++) { // Проход по всем командам оболочки
    if (strcmp(args[0], builtin_str[i]) == 0) { // Если команда являеться встроенной
      return (*builtin_func[i])(args); // Запуск встроенной команды
    }
  }
  return psh_launch(args); // Запуск команды
}

char *psh_read_line(void) { // Считывание строки
  char *line = NULL; // Переменная строки
  ssize_t bufsize = 0; // getline сама выделит память
  getline(&line, &bufsize, stdin); // Считывание строки
  return line; // Возвращение строки
}

#define PSH_TOK_BUFSIZE 64 // Буфер кол-ва аргументов
#define PSH_TOK_DELIM " \t\r\n\a" // Символы для разбиения

char **psh_split_line(char *line) { // Парсинг строки 
  int bufsize = PSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*)); // Массив аргументов
  char *token; // Аргумент
  if (!tokens) { // Получилось ли создать массив аргументов
    fprintf(stderr, "psh: ошибка выделения памяти\n");
    exit(EXIT_FAILURE); // Выход из программы
  }
  token = strtok(line, PSH_TOK_DELIM); // Отделяем первый аргумент
  while (token != NULL) { // Пока аргументы в строке не закончились
    tokens[position] = token; // Перемещаем интекс в массиве аргументов
    position++; // Увеличиваем счётчик индексов
    if (position >= bufsize) { // Пока счётчик индексов не достик буфера
      bufsize += PSH_TOK_BUFSIZE; // Расширяем буфер
      tokens = realloc(tokens, bufsize * sizeof(char*)); // Расширяем массив аргументов
      if (!tokens) { // Если аргументы превысили буфер
        fprintf(stderr, "psh: ошибка выделения памяти\n");
        exit(EXIT_FAILURE); // Выход из программы
      }
    }
    token = strtok(NULL, PSH_TOK_DELIM); // Отделяем следующий аргумент
  }
  tokens[position] = NULL; // Зануляем последний токен
  return tokens; // Возвращаем массив аргументов
}


void psh_loop(void) { // Цикл выполнения команд
  char *line; // Строка команды
  char **args; // Аргументы
  int status; // Мониторинг команды
  do {
    printf("8=> ");
    line = psh_read_line(); // Чтение: считывание команды со стандартных потоков
    args = psh_split_line(line); // Парсинг: распознавание программы и аргументов во входной строке
    status = psh_execute(args); // Исполнение: запуск распознанной команды
    free(line); // Освобождение из памяти строку
    free(args); // Освобождение из памяти аргументы
  } while (status);
}

int main(int argc, char **argv) {
  psh_loop(); // Запуск цикла команд
  return EXIT_SUCCESS; // Выключение / очистка памяти
}

