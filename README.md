# icariusOS

[![Lizenz: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

Minimalistischer Kernel für x86

## 🚀 Features (Top 5)

- **Speicherverwaltung mit Paging (4 MiB Pages)** – inklusive Page Frame Allocator
- **Multitasking mit Task-Switching** – jeder Task hat sein eigenes Seitenverzeichnis
- **Syscalls über `int 0x80`** – sicherer Übergang von Benutzer- zu Kernelmodus
- **Interrupt-Handling** – IDT mit erweiterten Fehleranalysen für Debugging
- **4 KiB Chunk-basierter Heap-Allocator** – Wachstum erfolgt automatisch, wenn zu wenig freier Speicher vorhanden ist

## 🔜 Nächste Schritte

- **Speicherlecks fixen** – Tasks sauber freiräumen 🧹
- **Scheduler bauen** – Round-Robin-Scheduler für mehrere Tasks 🕒
- **Standardbibliothek aufbauen** – POSIX-ähnliche Funktionen ✨
- **Mehr Syscalls** – für bessere Benutzermodus Unterstützung 🛠️

## 🎯 Der Weg ist das Ziel

Kennst du das Gefühl, wenn du etwas baust, das wirklich von Grund auf entsteht? Kein Framework, keine Bibliothek, keine Abkürzungen – nur du, deine Ideen und der Code, der das System kontrolliert. Genau das ist Kernel-Entwicklung für mich.

Hier gibt es keine Sicherheitsnetze – ich (bzw. wir) bin allein verantwortlich dafür, wie Speicher verwaltet wird, wie Tasks kommunizieren und ob der Code das System zum Laufen bringt oder alles abstürzen lässt. Und genau das macht es so faszinierend! Als mein erster Task-Switch funktionierte oder mein Debugger endlich die richtige Speicheradresse ausgab, fühlte sich das an wie ein verdammter Sieg. 🏆

Ja, manchmal könnte ich den Bildschirm aus dem Fenster werfen (👀 looking at you, Page Faults), aber sobald ich das Problem löse – dieses Gefühl ist unbeschreiblich. Kernel-Entwicklung ist mein persönliches Low-Level-Puzzlespiel, das mich ständig herausfordert und wachsen lässt.

Und das Beste? Es gibt kein festes Ziel. Ich baue hier keinen Linux-Konkurrenten – ich erkunde, lerne und genieße jeden Fortschritt.

Das Abenteuer hört nie auf – und genau das macht es so spannend. 🚀

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

git add .
git commit -m "Feature: Kurzbeschreibung der Änderung"

- **Sprache** – Ich akzeptiere nur Commits in Englisch und mit einem klaren Fokus auf das "Warum", nicht nur auf das "Was"
- **Hinweis** – Falls du dir unsicher bist, ob eine Änderung sinnvoll ist, erstelle vorab eine Issue, um sie zu besprechen

5️⃣ Pushen & Pull Request erstellen:

git push origin feature/dein-feature

Danach kannst du auf GitHub einen Pull Request (PR) eröffnen. Ich schaue mir alle PRs an und gebe dir so schnell ich kann Feedback.

📌 Hinweis:

Bitte halte dich an den Code-Stil des Projekts und teste deine Änderungen in QEMU, bevor du den PR erstellst. Falls du Fragen hast, schreib einfach eine Issue oder kommentiere direkt im PR.

Viel Spaß beim Kernel Hacking! 😎🔥

## Abhängigkeiten

Bevor du icariusOS baust, stelle sicher, dass du folgende Abhängigkeiten installiert hast:

- **Ubuntu** – (LTS)-Version von Ubuntu 
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

# Kompilieren

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

# GNU Debugger

```bash
./swap.sh && qemu-system-i386 -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
```

# Verzeichnisstruktur ausgeben

```bash
tree | xclip -selection clipboard
```