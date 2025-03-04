# icariusOS

[![Lizenz: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

Minimalistischer i686 Kernel

## 🚀 Features (Top 6)

- **Benutzershell icarsh** – Endlich eine erste Shell für icariusOS! Starte Programme, navigiere demnächst durch das Dateisystem und fühle dich wie ein echter Hacker! :D
- **Usermode-Programme in C** – Dank einer eigenen libc laufen User-Programme jetzt nativ! 🔥
- **Speicherverwaltung mit 4 MiB Paging** – Schnelle Speicherverwaltung mit einem optimierten 4 MiB Page Frame Allocator
- **Syscalls über int 0x80** – Sicherer Übergang zwischen User- und Kernelmodus für flexible Systemaufrufe
- **Interrupt-Handling** - Präzise Fehleranalysen mit der IDT für bessere Debugging-Möglichkeiten
- **Dynamischer Kernel Heap mit 4 KiB Chunks** – Automatisches Wachstum für effiziente Speicherverwaltung im Kernel

## ⚒️ Nächste Schritte

🔜 **Scheduler**  
Ein **Round-Robin-Scheduler** für echtes **Multitasking auf Task- bzw. Thread-Ebene**! Threads werden in einer **globalen Task-Liste nach Prozessen sortiert** eingefügt, um **TLB-Flushes zu minimieren** und schnellere Kontextwechsel zu ermöglichen

🔜 **Standardbibliothek ausbauen**  
Mehr **POSIX-ähnliche Funktionen** für Usermode-Programme, um eine **saubere Schnittstelle** zu bieten 

🔜 **Weitere Syscalls**  
Noch mehr **Systemaufrufe für Usermode-Programme**, um die Funktionalität weiter auszubauen

🔜 **Ordnerstruktur überarbeiten**  
Nur noch **architektur-spezifische Dinge** im `arch/`-Ordner – der Rest wandert in `src/` für eine **bessere Übersichtlichkeit**

## 🎯 Warum icariusOS?

icariusOS ist ein Abenteuer voller Herausforderungen und Erfolgsmomente. Ohne Frameworks oder Bibliotheken erschafft man etwas völlig Eigenes – jede Idee nimmt langsam Form an. Jeder weitere Implementierung bringt neue Fragen und Rätsel: Ein falsch verwalteter Speicher, ein fehlerhafter Moduswechsel – und die Lösung liegt allein in den eigenen Händen. Doch genau das macht es so spannend. Wenn der erste Wechsel in den Benutzermodus klappt oder der Debugger endlich die richtige Adresse ausgibt, ist das Gefühl unbeschreiblich. Es geht mir nicht darum, ein weiterer Linux-Konkurrent zu sein, sondern ums Experimentieren, Lernen und das Feiern jedes kleinen Fortschritts. OS-Dev ist ein nie endendes Projekt – und genau das macht es so unglaublich reizvoll.

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

# Fehlersuche mit GNU Debugger 

Das Skript startet QEMU mit Debugging-Unterstützung und lädt den GNU Debugger (GDB) mit vorkonfigurierten Einstellungen, was die Fehlersuche im Kernel erheblich erleichtert.

```bash
./swap.sh && qemu-system-i386 -m 4G -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
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

# YOLO-DevOps-Pipeline 😎🔥

```bash
git checkout feature/task && git pull origin feature/task && git add . && git commit -m "Refactoring" && \
git checkout dev && git pull origin dev && git merge feature/task && git push origin dev && \
git checkout main && git pull origin main && git merge dev && git push origin main && \
git checkout feature/task
```