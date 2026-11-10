## SS2026

## Documentation
Both Lab combined.pdf and the Lab combined.md file contain the same documentation of all five labs.

## Getting Started
Zunächst muss das Projekt-Repository geklont und anschließend in Visual Studio Code geöffnet werden:

```bash
git clone https://code.fbi.h-da.de/eingebettete-systeme/arduino-due/ss2026.git
code ss2026
```

[Link zur Anleitung für VSCode](https://code.visualstudio.com/docs/getstarted/getting-started)

### VSCode Tasks 
- **OpenOCD**: Startet automatisch. Muss neu gestartet werden, wenn die Verbindung zur Hardware unterbrochen wurde.
- **Minicom**: Dient zur Kommunikation über die serielle Schnittstelle.

[Link zur Anleitung für VSCode Tasks](https://code.visualstudio.com/docs/debugtest/tasks)

### Debugging
Für das Debugging wird der GNU Debugger (gdb) verwendet.

[Link zur Anleitung für VSCode Debugging](https://code.visualstudio.com/docs/debugtest/debugging)

### Source Controll
Um dein eigenes Repository zu verknüpfen, kannst du das Remote-Repository wie folgt setzen:

```bash
git remote set-url origin <Link zu Ihrem Repository>
```
**Hinweis**: Ersetze `<Link zu Ihrem Repository>` durch die tatsächliche URL deines eigenen Git-Repos.

[Link zur Anleitung für VSCode SourceControl](https://code.visualstudio.com/docs/sourcecontrol/overview)

## Remote-Verbindung zum Auto
Das Passwort wird im entsprechenden Praktikum bereitgestellt (bei beiden Verbindungen identisch).

1. Verbinden Sie den WLAN-Stick per USB mit dem Laborrechner. Ein entsprechender Stick kann bei der verantwortlichen Person für das Praktikum ausgeliehen werden.
2. Mit dem WLAN-Hotspot verbinden (**SSID**: `escarpi5[Autofarbe]` z. B. `escarpi5orange`)
3. Zwei SSH-Verbindungen aufbauen: `ssh escarpi5@10.42.0.1`
4. Auf dem Fahrzeug entsprechende Skripte zum Starten von Minicom und OpenOCD ausführen: `./start_openocd_due.sh` und `./start_minicom.sh`

<details>
<summary>Bilder zum Auto</summary>
![Car](docs/car.webp)
![Car](docs/car2.webp)
</details>

