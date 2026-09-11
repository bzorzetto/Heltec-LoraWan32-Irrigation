// ============================================================
// UPLINK
// ============================================================

function decodeUplink(input) {

    var bytes = input.bytes;

    // --------------------------------------------------------
    // Accettiamo solo FPort 10
    // --------------------------------------------------------

    if (input.fPort !== 10) {
        return {
            data: {}
        };
    }

    // --------------------------------------------------------
    // ACK KC868 = 3 byte
    // A5 02 STATO
    // --------------------------------------------------------

    if (bytes.length !== 3) {
        return {
            errors: ["Payload irrigazione non valido"]
        };
    }

    // --------------------------------------------------------
    // Controllo header
    // --------------------------------------------------------

    if (bytes[0] !== 0xA5 || bytes[1] !== 0x02) {
        return {
            errors: ["Header ACK non valido"]
        };
    }

    // --------------------------------------------------------
    // Byte stato
    // --------------------------------------------------------

    var stato = bytes[2];

    return {
        data: {

            stato: stato,

            uscita_1: (stato & 0x01) !== 0,
            uscita_2: (stato & 0x02) !== 0,
            uscita_3: (stato & 0x04) !== 0,
            uscita_4: (stato & 0x08) !== 0,
            uscita_5: (stato & 0x10) !== 0,
            uscita_6: (stato & 0x20) !== 0,
            uscita_7: (stato & 0x40) !== 0,
            uscita_8: (stato & 0x80) !== 0
        }
    };
}


// ============================================================
// DOWNLINK
// ============================================================
//
// Formato comando KC868:
//
// BYTE 0 = COMANDO
// BYTE 1 = USCITA
// BYTE 2 = TEMPO HIGH
// BYTE 3 = TEMPO LOW
//
// Esempio:
//
// {
//     comando: 1,
//     uscita: 4,
//     tempo: 60
// }
//
// produce:
//
// 01 04 00 3C
//
// ============================================================

function encodeDownlink(input) {

    var data = input.data;

    // --------------------------------------------------------
    // Controllo presenza dati
    // --------------------------------------------------------

    if (data === undefined || data === null) {
        return {
            errors: ["Dati downlink mancanti"]
        };
    }

    // --------------------------------------------------------
    // Recupera parametri
    // --------------------------------------------------------

    var comando = data.comando;
    var uscita = data.uscita;
    var tempo = data.tempo;

    // --------------------------------------------------------
    // Controllo comando
    // --------------------------------------------------------

    if (comando !== 1 && comando !== 2) {
        return {
            errors: ["Comando non valido"]
        };
    }

    // --------------------------------------------------------
    // Controllo uscita
    //
    // 1...8 = singola uscita
    // FF    = tutte le uscite
    // --------------------------------------------------------

    if (comando === 1) {

        if (uscita < 1 || uscita > 8) {
            return {
                errors: ["Uscita non valida"]
            };
        }

    } else {

        if (!((uscita >= 1 && uscita <= 8) || uscita === 255)) {
            return {
                errors: ["Uscita non valida"]
            };
        }
    }

    // --------------------------------------------------------
    // Tempo
    // --------------------------------------------------------

    if (tempo === undefined || tempo === null) {
        tempo = 0;
    }

    if (tempo < 0 || tempo > 65535) {
        return {
            errors: ["Tempo fuori intervallo 0..65535 secondi"]
        };
    }

    // --------------------------------------------------------
    // Costruzione frame
    // --------------------------------------------------------

    var tempoHigh = (tempo >> 8) & 0xFF;
    var tempoLow  = tempo & 0xFF;

    return {
        bytes: [
            comando,
            uscita,
            tempoHigh,
            tempoLow
        ],
        fPort: 10
    };
}