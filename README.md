# Elaborato-1-IOT 
## 1. File input.cpp 
- aumentato il BOUNCING_TIME da 50ms a 300ms.
- Configurazione Pin: In initInput(), cambiato la modalità dei pin da INPUT a INPUT_PULLUP.
- Tipo di Interrupt: cambiato il trigger dell'interrupt da CHANGE a FALLING. Ora l'interrupt scatta solo quando il pulsante viene premuto
- Nel buttonHandler(), rimosso il digitalRead(). Applicato solo il timer di debounce

## 2. File core.cpp 
- Creazione del sistema wait(): nuova funzione wait(duration, nextState) e una funzione state_wait().
- In set_difficulty(), corretto il bug iniziale. Invece di delay(1000) e changeState(GAME_STATE), ora usiamo wait(1000, STATE_START_ROUND).
- Nuovi Stati: create le funzioni per i nuovi stati (come startNewRound() e state_show_sequence()) per gestire correttamente la visualizzazione della sequenza, la pulizia dello schermo e l'avvio del timer del giocatore.
- Logica playerInput(): modificato playerInput() per lasciare i LED accesi dopo la pressione (invece di accenderli e spegnerli subito) e abbiamo rimosso la chiamata a resetInput(), che prima impediva di inserire più di un numero.
- Logica finalize(): Anche lo stato finale ora usa il sistema wait(10000, INTRO_STATE) per tornare all'inizio in modo non-bloccante.

## 3. File core.h e tos.ino 
- core.h: Abbiamo aggiunto le definizioni per i nuovi stati (es. STATE_WAIT, STATE_START_ROUND, STATE_SHOW_SEQUENCE) e i prototipi per le nuove funzioni (es. wait(), state_wait()).
- tos.ino: Abbiamo aggiornato lo switch nel loop()  per includere i nuovi stati che abbiamo aggiunto, permettendo alla macchina a stati di gestirli.

---

Directory \tos contains all the code for assignement01 for the course "Embedded Systems & Iot"
- tos.ino : is the entry point of the application
- core.cpp : contains the main game logic
- input.cpp : handles button input using interrupts and debouncing
- kernel.cpp : state management procedures