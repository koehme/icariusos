# icariusOS

[![Lizenz: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

Minimalistischer i686 Kernel

## Abhängigkeiten

Bevor du icariusOS baust, stelle sicher, dass du folgende Abhängigkeiten installiert hast:

- **Ubuntu** – (LTS)-Version von Ubuntu entweder als virtuelle Maschine oder WSL
- **Cross-Compiler** – Um Code zu erzeugen, der freistehend ist und ohne Standardbibliothek auskommt.

```bash
./i686.sh
```

# Starten

Bevor du icariusOS startest, stelle sicher, dass das dazugehörige FAT16-Dateisystem generiert wurde, indem du die folgenden Befehle ausführst:

```bash
./build.sh
./fat16.sh
```

# icariusOS bauen

Um den Kernel neu zu bauen und zu starten, verwende diesen Befehl:

```bash
./build.sh && qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```

# Aktualisieren

Falls du den Kernel aktualisieren möchtest, ersetze die ICARIUS.BIN-Datei im FAT16-Dateisystem mit:

```bash
./swap.sh 
```

Nach der Aktualisierung kannst du icariusOS mit dem neuen Kernel starten:

```bash
qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```

Für die Bequemen - In einem Befehl:

```bash
./swap.sh && qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```

# 🔧 Kernel-Debugging mit QEMU & GDB

Mit QEMU und GDB kannst du den Kernel auf Source-Level debuggen:

```bash
./swap.sh && qemu-system-i386 -m 4G -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
```

# Fehlersuche im Benutzermodus

Auch im Usermode rocken QEMU & GDB – hier kannst du:

```bash
./swap.sh && qemu-system-i386 -m 4G -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
break *0x0
file ./src/user/elf/icarsh.elf
break execute_builtin
```

# Benutzershell 'ICARSH.BIN' bauen

Dieses Skript kompiliert die Benutzer-Shell und integriert sie direkt ins FAT16-Dateisystem, wodurch sie im OS ausführbar wird.

```bash
make icarsh && ./fat16.sh
``` 

# Verzeichnisstruktur ausgeben

Die tree-Ausgabe wird direkt in die Zwischenablage kopiert, was eine schnelle Weiterverarbeitung oder Dokumentation der Projektstruktur ermöglicht.

```bash
tree | xclip -selection clipboard
```

# YOLO-DevOps-Pipeline 

Was soll schon schiefgehen? :D

```bash
git checkout feature && git pull origin feature && git add . && git commit -m "Refactoring" && \
git checkout dev && git pull origin dev && git merge feature && git push origin dev && \
git checkout main && git pull origin main && git merge dev && git push origin main && \
git checkout feature
```

# 🤝 Mitmachen

Falls du Lust hast, an icariusOS mitzuwirken, freue ich mich über jeden Pull Request! Egal, ob du Bugs fixt, Features hinzufügst oder den Code optimierst!

1️⃣ Repo forken:

Klick oben rechts auf Fork, um dein eigenes Repository zu erstellen.

2️⃣ Projekt klonen:

git clone https://github.com/dein-nutzername/icariusOS.git
cd icariusOS

3️⃣ Branch erstellen:

git checkout -b feature/dein-feature

4️⃣ Änderungen machen

git add . &&
git commit -m "Feature: Kurzbeschreibung der Änderung"

5️⃣ Pushen & Pull Request erstellen:

git push origin feature/dein-feature

Danach kannst du auf GitHub einen Pull Request (PR) eröffnen. Ich schaue mir alle PRs an und gebe dir so schnell ich kann Feedback.

## Commit-Richtlinien (aka die liebevollen Rahmenbedingungen)

Sprache:
Du darfst committen, wie du möchtest – Englisch ist empfohlen, weil’s für andere im Projekt (und dein Zukunfts-Ich) am verständlichsten ist.

Inhalt:
Bitte schreib nicht nur WAS du geändert hast, sondern auch WARUM.

Stilfreiheit:
Kein Format-Dogma 

📌 Hinweis:

Teste deine Änderungen in QEMU, bevor du den PR erstellst. Falls du Fragen hast, schreib einfach eine Issue oder kommentiere direkt im PR.

Viel Spaß beim Kernel Hacking! 😎🔥