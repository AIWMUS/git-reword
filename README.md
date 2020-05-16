# git-reword

git-reword -- простая утилита, позволяющая быстро и просто менять сообщения в старых коммитах (при этом всё равно присутствует ряд ограничений, 
которые следует иметь в виду).

## Идея

В обычном git самым простым способом сделать аналогичное является использование команды "git rebase --interactive", которая откатывает коммиты, 
а потом применяет их с новыми условиями, которые задаёт пользователь. Одним из этих условий может быть изменение сообщений коммитов 
(команда "reword" при применении коммита). 
Так как эта команда очень универсальна, 
она не очень подходит для такой специализированной задачи, как изменение сообщений коммитов. Одна из точных причин в том, 
что команда откатывает коммиты полностью, вместе данными, которые могут быть большими, 
в результате это может занять очень много времени только для того, чтобы изменить сообщения коммитов. 
Оказывается, что git хранит метаинформацию и сами данные репозитория отдельно, поэтому коммиты можно изменить никак не касаясь данных, 
что позволит выполнять изменения сообщений гораздо быстрее, чем это делает "git rebase --interactive".

## Быстрый старт

Существует три версии:
- *py_git*        (**Python : git-cli**)                    -- написана на Python3 и использует специальные команды обычного git;
- *py_git-reabse* (**Python : "git rebase --interactive"**) -- написана на Python3 и использует обычный "git rebase --interactive";
- *cpp_libgit2*   (**C/C++ : libgit2**)                     -- написана на C++14 и использует libgit2.

Для использования доступны все версии. Каждая работает независимо от остальных и расположена в собственной папке. Для работы *py_\** 
нужна так же папка *py_utility*.

Для каждой версии в соответствующем каталоге есть исполняемый файл "git-reword", к которому можно обращаться. 
Для удобства в корне репозитория расположена ссылка "git-reword", к которой можно обращаться. 
Она может ссылаться на любую одну конкретную версию (по умолчанию ссылается на версию *cpp_libgit2*).

## Ограничения и различия [обязательно к прочтению]

Этот раздел предстоит серьёзно дорабатывать (TODO).

Режим "one":
- all -- нужно находится в вершине ветки, в которой находится редактируемый коммит, (то есть HEAD должен находится в этой ветки).

Режим "list":
- all             -- в списке все коммиты должны находится на одном прямом участке одной ветки (такой участок на "git log --graph" 
выглядит как `*--*--*--*--...`) (то есть нельзя, чтобы в списке были коммиты из разных веток или коммиты из разных ответвлений внутри 
одной ветки (на самом деле, некоторые из этих случаев по-разному учтены в разных версия, но проект ещё будет дорабатываться, а пока лучше 
избегать применения git-reword на сложных участках (TODO) ) ), при этом нужно находится в вершине этой ветки (то есть HEAD должен находится в этой ветки);
- *py_git*        -- в конце списка должен быть самый родительский коммит (самый ранний) среди всех коммитов в списке;
- *py_git-reabse* -- все коммиты в списке должны быть упорядочены по отношениям, при этом сначала идут самые потомки (самые последние);
- *cpp_libgit2*   -- в конце списка должен быть самый родительский коммит (самый ранний) среди всех коммитов в списке.

Общие различия:
- *py_git*        -- рекомендуется к использованию, потому что тестировалась наибольшее количество времени;
- *py_git-rebase* -- не рекомендуется к использования, потому что очень неаккуратно написана, так как была создана для тестирования;
- *cpp_libgit2*   -- работает быстрее остальных версий (используется по умолчанию).

Есть и другие различия (TODO).

## Как работает

[Тут](howitworks.md).

## Сборка и Установка

#### *py_git*

Не требует сборки.

#### *py_git-reabse*

Не требует сборки.

#### *cpp_libgit2*

Сборка и запуск:

```
cd cpp_libgit2
./br
```

Если увидели "usage", то программа успешно собралась и работает (находится по пути *cpp_libgti2/build/git-reword*).

### Использование

Помощь (rev -- ревизия (идентификатор) коммита, new_message -- новое сообщение (комментарий) для коммита):

```
$ git-reword --help
Usage: git-reword one (-d | -m) <rev> <new_message>
       git-reword list (-d | -m) (-s | -c | -n) <path_to_list-file>
    
Commands "one" & "list":
    
    -d  (not yet implemented)
        Delete merge-commits on the edited path.
    
    -m
        Save merge-commits on the edited path.

Command "list":
    
    -s (not yet implemented)
        Automatically format the list because it does not satisfy some conditions.
    
    -c (not yet implemented)
        Check that the list satisfies the conditions.
    
    -n
        The list fully meets all the requirements, the user knows what he is doing.

    list-file:

        format:
            <rev_1> <new_message_1>
            <rev_2> <new_message_2>
            ...
            <rev_n> <new_message_n>
        
        The list must satisfy certain conditions, which depend on the version.
```
