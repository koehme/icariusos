# icariusOS

[![Lizenz: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

Minimalistischer i686 Kernel

## 🎯 Warum icariusOS?

icariusOS ist für mich ein Abenteuer – voller Fragen, Rätsel und Erfolgsmomente.
Hier entsteht alles von Grund auf selbst. Jeder Schritt, jede Entscheidung formt das System weiter – und genau das macht es so spannend.

Ich baue icariusOS nicht, um irgendein Linux-Konkurrent zu werden.
Ich baue es, weil ich lernen will. Weil ich’s spannend finde. Weil ich’s kann. Und ich feiere jeden kleinen Fortschritt:
- 🟢 Der erste Benutzerprozess.
- 📦 Das erste korrekt gemappte Page Directory.
- 🧵 Der erste Task-Switch.

Das ist ein verdammt gutes Gefühl.

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

# FAT16 Partition  

Hier sind die relevanten Werte des FAT16-Dateisystems `./fat16.sh`, das in ICARIUS verwendet wird:

- **Bootsektor** → Enthält wichtige Parameter (z. B. Reserved Sectors, FAT Size, Root Entry Count).  
- **FAT-Tabelle** → Zeigt an, welche Cluster zusammengehören (Verkettung von Dateien).  
- **Root Directory** → Speichert Dateinamen und Startcluster.  
- **Datenbereich** → Hier liegen die eigentlichen Dateiinhalte.  

✅ **Sektoren, Cluster, Offsets sind immer nach fester Logik aufgebaut.**  
✅ **Die FAT ist einfach eine "Verkettungstabelle", die sagt, welcher Cluster nach welchem kommt.**  
✅ **Wenn man das Partitionsoffset beachtet, kann man jede Datei direkt auslesen!**  

## 📌 Bootsektor-Daten  

| Parameter           | Wert          | Erklärung |
|---------------------|--------------|-----------------------------------------|
| **Jump Code**      | `0xeb 0x3c 0x90` | Bootsektor-Sprungbefehl für den Code |
| **OEM Name**       | `mkfs.fat`      | Name des Erstellers des Dateisystems |
| **Bytes Per Sec**  | `512`          | Bytes pro Sektor (Standard: 512) |
| **Sec Per Cluster**| `16`           | Anzahl der Sektoren pro Cluster |
| **Reserved Sectors** | `1`          | Reservierte Sektoren für Bootsektor/FAT-Header |
| **Number of FATs** | `2`            | Anzahl der FAT-Kopien (meistens 2) |
| **Root Entry Count** | `512`        | Maximale Anzahl an Einträgen im Root-Verzeichnis |
| **Total Sectors16** | `0`          | Falls 0 → Wert steht in *Total Sectors32* |
| **Media Type**     | `0xf8`         | Medientyp (0xF8 = Festplatte) |
| **FAT Size 16**    | `256`          | Anzahl der Sektoren pro FAT-Tabelle |
| **Sectors Per Trk** | `63`         | Anzahl der Sektoren pro Spur (Track) |
| **Number of Heads** | `32`         | Anzahl der Leseköpfe (z.B. für CHS-Adressierung) |
| **Hidden Sectors** | `0`           | Versteckte Sektoren vor der Partition |
| **Total Sectors32** | `1.046.493`  | Gesamtanzahl der Sektoren der Partition |

| Parameter         | Wert (Hex)  | Wert (Dezimal) | Bedeutung |
|------------------|------------|---------------|-----------|
| **Sektorgröße**  | `0x200`     | `512`         | Standardgröße eines Sektors |
| **Gesamtsektoren** | `0x100000` | `1.048.576` | Gesamtanzahl an Sektoren im Dateisystem |
| **Kapazität**    | `0x80000`   | `524288 KiB`  | Größe der Partition |
| **Root Dir Offset** | `0x42000`  | `270336`     | Offset zum Root Directory innerhalb der Partition |
| **Root Dir Abs** | `0x140000`  | `1.310.720`   | Absoluter Offset des Root Directories auf der Festplatte |
| **Root Dir Größe** | `0x4000`  | `16.384 Bytes` | Größe des Root Directory Bereichs (512 Einträge * 32 Bytes) |
| **FAT Offset**  | `0x2000`    | `8192`        | Offset zur ersten FAT-Tabelle innerhalb der Partition |
| **FAT Absolut** | `0x102000`  | `1.056.768`   | Absoluter Offset zur ersten FAT-Tabelle auf der Festplatte |
| **FAT Größe** | `0x20000`    | `131.072 Bytes` | Größe der FAT-Tabelle in Bytes |
| **Root Dir Sek** | `0x20`     | `32`          | Anzahl der belegten Sektoren für das Root Directory |
| **1 Daten-Sektor** | `0x230`   | `560`         | Sektor, in dem der erste Datencluster beginnt |
| **Daten-Sektoren** | `0xFF5D`  | `1.045.933`   | Anzahl der nutzbaren Daten-Sektoren |

## 📌 FAT16-Speicherlayout  

| Offset   | Größe   | Sektoren | Berechnung (512B) | Name |
|----------|--------|----------|-------------------|-----------------------------------------|
| `0x00000` | `0x2000`  | `16` | `16 * 512` | Reserved Sectors (inkl. Bootsektor) |
| `0x02000` | `0x20000` | `256` | `256 * 512` | 1st FAT |
| `0x22000` | `0x20000` | `256` | `256 * 512` | 2nd FAT |
| `0x42000` | `0x4000`  | `32` | `512 * 32` | Root Directory Area |
| `0x46000` | `0x4000`  | `32` | `2 * 8192` | Data Area (Cluster 0: Boot Code, Cluster 1) |
| `0x4A000` |||| Data Area (Files & Subdirectories) |

**Gesamt: 560 Sektoren bis zum Datenbereich** 🚀

## 📌 Weitere Offsets  

| Bezeichnung | Hex-Wert | Dezimalwert | Bedeutung |
|------------|---------|------------|------------|
| **FAT-Bereich Offset** | `0x2000` | `8192` | Offset zur FAT (relativ zur Partition) |
| **FAT-Bereich Absolut** | `0x102000` | `1056768` | Absoluter Speicherort der FAT auf der Festplatte |
| **Root-Dir Offset** | `0x42000` | `270336` | Offset zum Root Directory (relativ zur Partition) |
| **Root-Dir Absolut** | `0x142000` | `1310720` | Absoluter Speicherort des Root Directories |
| **Erster Daten-Sektor** | `0x230` | `560` | Startsektor des ersten Datenclusters |
| **Daten-Sektoren** | `0xFF5A` | `65370` | Gesamte Anzahl der nutzbaren Sektoren für Daten |
| **Gesamtanzahl Cluster** | `0xFF5A` | `65370` | Anzahl der Cluster im Dateisystem |

## 📌 Beispiel: Cluster 770  

Die **Adresse eines Clusters** im Datenbereich berechnet sich wie folgt:

Partition 0x100000

Datenbereich Offset = 560 × 512 = 0x46000
0x46000 ist der Offset des ersten nutzbaren Clusters innerhalb der Partition.

Cluster Offset = (Cluster - 2) * Bytes per Cluster
(770−2)×8192=768×8192=0x600000
0x600000 ist der relative Offset des Clusters im Datenbereich.

Cluster 770 Absoluter Offset = Partition Offset + Datenbereich Offset + Cluster Offset
                              = 0x100000 + 0x46000 + 0x600000
                              = 0x742000
                              
# FAT16 Dateisystem prüfen

Nachdem eine Datei erfolgreich im FAT16-Dateisystem `./fat16.sh` angelegt wurde, kann mit `hexdump` überprüft werden, ob der Eintrag tatsächlich im Root Directory geschrieben wurde.

```bash
hexdump -C -s 0x742060 -n 512 ICARIUS.img
```