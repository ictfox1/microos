# MicroOS

Учебная 16-битная операционная система в реальном режиме x86 с загрузчиком
на NASM и ядром на C (freestanding).

## Возможности

- Загрузчик на ассемблере (NASM, real mode)
- Ядро на C (`-m16`, без стандартной библиотеки)
- Эхо-шелл с историей команд (стрелки вверх/вниз)
- Встроенная файловая система в памяти
- Утилита `cut` для обработки текста
- Команды: `help`, `ls`, `cat`, `create`, `write`, `del`, `format`, `info`,
  `echo`, `clear`, `reboot`, `shutdown`, `history`, `version`, `cut`

## Требования

- `nasm`
- `clang` (с поддержкой `-m16`)
- `ld` (binutils с `elf_i386`)
- `qemu-system-i386`
- `make`

Установка на Debian/Ubuntu:

```bash
sudo apt install nasm clang lld binutils qemu-system-x86 make
