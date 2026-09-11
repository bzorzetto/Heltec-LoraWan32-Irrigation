# Heltec-LoraWan32-Irrigation
Irrigation for remote gardens

Questo progetto utilizza un dispositivo LoRaWan Heltec V2 abbinato ad un modulo KC868 A8 per pilotare le elettrovalvole di un giardino remoto.
I due dispositivi si scambiano le informazioni tramite uart con un protocollo molto semplice composto da 8 bit dove i primi due bit (MSB)
indicano il tipo di comando i secondi due bit, l'elettrovalvola da aprire e gli ultimi due il tempo di apertura. ES 01 01 0100

