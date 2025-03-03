# icariusOS

[![Lizenz: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

Minimalistischer Kernel für x86

## 🚀 Features (Top 6)

Usermode-Programme in C – Dank einer eigenen libc laufen User-Programme jetzt nativ! 🔥
Speicherverwaltung mit 4 MiB Paging – Schnelle Speicherverwaltung mit einem optimierten 4 MiB Page Frame Allocator
Echtes Multitasking mit Task-Switching – Jeder Prozess bekommt ein eigenes Page Directory für maximale Isolation
Syscalls über int 0x80 – Sicherer Übergang zwischen User- und Kernelmodus für flexible Systemaufrufe
Erweitertes Interrupt-Handling – Präzise Fehleranalysen mit der IDT für bessere Debugging-Möglichkeiten
Dynamischer Heap mit 4 KiB Chunks – Automatisches Wachstum für effiziente Speicherverwaltung im Kernel

## 🔜 Nächste Schritte

Eigener Scheduler – Ein Round-Robin-Scheduler für echtes Multitasking! 🕒
Standardbibliothek ausbauen – Mehr POSIX-ähnliche Funktionen für Usermode-Programme ✨
Weitere Syscalls – Noch mehr Funktionen für Usermode-Programme und besseren Kernel-Support! 🛠️
Ordnerstruktur überarbeiten – Nur noch architektur-spezifische Dinge im arch/ Ordner, der Rest wandert in src/ für mehr Übersichtlichkeit 📂

## 🎯 Der Weg ist das Ziel

Kennst du das Gefühl, wenn du wirklich von Null etwas erschaffst? Das fühlt sich richtig echt an. Ohne Frameworks, keine Bibliotheken und vor allem keine fertigen Lösungen, die ins System hineingepfuscht werden. Nur ich, meine Ideen im Kopf und die komplette Kontrolle über das System, das Stück für Stück entsteht.

Hier gibt’s nichts, was dich vor Fehlern bewahrt – wenn der Speicher falsch verwaltet wird oder der Wechsel in den Benutzermodus nicht sauber läuft, liegt es an mir, das zu fixen. Und genau das macht es so verdammt spannend! Als mein erster Wechsel in den Benutzermodus funktioniert hat oder mein Debugger endlich die richtige Speicheradresse ausgespuckt hat, fühlte sich das verdammt gut an. Genau das macht Kernel-Entwicklung für mich so verdammt reizvoll.

Klar, manchmal will ich den Monitor aus dem Fenster werfen – "Och nö, schon wieder ein Seitenfehler? MAN!" 👀 Aber wenn ich den Bug dann finde – dieses Gefühl ist einfach unersetzbar. Kernel-Entwicklung ist eine krasse Herausforderung, an der man ständig wachsen kann, wenn man es zulässt.

Und das Beste daran? Es gibt kein Endziel. Ich baue hier auch keinen Linux-Konkurrenten – ich experimentiere, lerne und genieße einfach jeden noch so kleinen Fortschritt.

Das Abenteuer geht immer weiter – und genau das macht es so cool. 🚀

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

Das Skript startet QEMU mit Debugging-Unterstützung und lädt den GNU Debugger (GDB) mit vorkonfigurierten Einstellungen, was die Fehlersuche im Kernel erheblich erleichtert.

```bash
./swap.sh && qemu-system-i386 -m 4G -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
```

# Benutzershell 'ICARSH.BIN'

Dieses Skript kompiliert die Benutzer-Shell und integriert sie direkt ins FAT16-Dateisystem, wodurch sie im OS ausführbar wird.

```bash
make icarsh && ./fat16.sh
``` 

# Verzeichnisstruktur ausgeben

Die tree-Ausgabe wird direkt in die Zwischenablage kopiert, was eine schnelle Weiterverarbeitung oder Dokumentation der Projektstruktur ermöglicht.

```bash
tree | xclip -selection clipboard
```