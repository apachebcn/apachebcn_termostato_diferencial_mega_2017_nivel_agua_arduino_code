#define MAX_CHIPS 14
#define TOTAL_SHIFT_CHIPS_IN_BOARD 2

#define PIN_EXTIENDE_X_7 2
#define PIN_EXTIENDE_X_6 3
#define PIN_EXTIENDE_X_5 4
#define PIN_EXTIENDE_X_4 5
#define PIN_EXTIENDE_X_3 6
#define PIN_EXTIENDE_X_2 7

#define PIN_LED_ENVIA_DATO 9
#define PIN_ENVIA_DATO_A_TERMOSTATO 11 //(ECHO)
#define PIN_TERMOSTATO_PIDE_DATO 12 //(TRIG)
#define PIN_DEBUG 8
#define PIN_ALIMENTA_CONTACTOS_AGUA 13

#define PRINT_MSG_WAITING_MILISECONDS 5000
#define DELAY_END_MILISECONDS 500
#define DELAY_LED_ENVIA_DATO 50

// Width of pulse to trigger the shift register to read and latch.
#define PULSE_WIDTH_USEC   20

#define PIN_PLOAD_74HC165 A1
#define PIN_CLOCK_ENABLE_74HC165 A0
#define PIN_DATA_74HC165 10
#define PIN_CLOCK_74HC165 A2

byte TOTAL_SHIFT_CHIPS = 2;
byte DATA_WIDTH = TOTAL_SHIFT_CHIPS_IN_BOARD * 8;

bool bytesVal[8 * MAX_CHIPS]; //Total maximo, de 8 bits por 16 chips
bool Debug_pulsado = 0;
unsigned long waiting_msg = 0;

void read_shift_regs()
{
    byte real_bit_pos;
    byte chip_num;

    limpiar_bytesVal();

    digitalWrite(PIN_CLOCK_ENABLE_74HC165, HIGH);
    digitalWrite(PIN_PLOAD_74HC165, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(PIN_PLOAD_74HC165, HIGH);
    digitalWrite(PIN_CLOCK_ENABLE_74HC165, LOW);

    // Loop to read each bit value from the serial out line of the SN74HC165N.
    for(byte chip_num = 0; chip_num < TOTAL_SHIFT_CHIPS; chip_num++)
    {
        for (byte bit_pos = 8; bit_pos > 0; bit_pos--)
        {
            real_bit_pos = (chip_num * 8) + (bit_pos - 1);
            bytesVal[real_bit_pos] = digitalRead(PIN_DATA_74HC165);
            shift_bit_165();
        }
    }
}

void shift_bit_165()
{
    digitalWrite(PIN_CLOCK_74HC165, HIGH);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(PIN_CLOCK_74HC165, LOW);
}

void limpiar_bytesVal()
{
    byte real_bit_pos;
    for(byte real_bit_pos = 0; real_bit_pos < DATA_WIDTH + 1; real_bit_pos++)
    {
        bytesVal[real_bit_pos] = 0;
    }
 }

void mostrar_bytesVal()
{
    byte real_bit_pos;
    for(byte i = 0; i < TOTAL_SHIFT_CHIPS; i++)
    {
        Serial.print("Chip ");Serial.print(i+1);Serial.print(" -> ");
        for (byte bit_pos = 0; bit_pos < 8; bit_pos++)
        {
            real_bit_pos = (i*8)+(bit_pos);
            Serial.print(bytesVal[real_bit_pos]);
        }
        Serial.println();
    }
}

byte total_data_chips()
{
    pinMode(PIN_EXTIENDE_X_2, INPUT_PULLUP);  
    pinMode(PIN_EXTIENDE_X_3, INPUT_PULLUP);  
    pinMode(PIN_EXTIENDE_X_4, INPUT_PULLUP);  
    pinMode(PIN_EXTIENDE_X_5, INPUT_PULLUP);  
    pinMode(PIN_EXTIENDE_X_6, INPUT_PULLUP);  
    pinMode(PIN_EXTIENDE_X_7, INPUT_PULLUP);  

    if (!digitalRead(PIN_EXTIENDE_X_2)) TOTAL_SHIFT_CHIPS = TOTAL_SHIFT_CHIPS_IN_BOARD * 2;
    if (!digitalRead(PIN_EXTIENDE_X_3)) TOTAL_SHIFT_CHIPS = TOTAL_SHIFT_CHIPS_IN_BOARD * 3;
    if (!digitalRead(PIN_EXTIENDE_X_4)) TOTAL_SHIFT_CHIPS = TOTAL_SHIFT_CHIPS_IN_BOARD * 4;
    if (!digitalRead(PIN_EXTIENDE_X_5)) TOTAL_SHIFT_CHIPS = TOTAL_SHIFT_CHIPS_IN_BOARD * 5;
    if (!digitalRead(PIN_EXTIENDE_X_6)) TOTAL_SHIFT_CHIPS = TOTAL_SHIFT_CHIPS_IN_BOARD * 6;
    if (!digitalRead(PIN_EXTIENDE_X_7)) TOTAL_SHIFT_CHIPS = TOTAL_SHIFT_CHIPS_IN_BOARD * 7;
    
    pinMode(PIN_EXTIENDE_X_2, 0);  
    pinMode(PIN_EXTIENDE_X_3, 0);  
    pinMode(PIN_EXTIENDE_X_4, 0);  
    pinMode(PIN_EXTIENDE_X_5, 0);  
    pinMode(PIN_EXTIENDE_X_6, 0);  
    pinMode(PIN_EXTIENDE_X_7, 0);  
    return TOTAL_SHIFT_CHIPS * 8 - 1;
}

void msg_string(String text, bool forced = 0)
{
    if (!Debug_pulsado && !forced) return;
    double timer = millis();
    timer = timer / 1000;
    Serial.print(timer);
    Serial.print(F(" segundos - "));
    Serial.println(text);
}

void setup()
{
    String texto;
    pinMode(PIN_PLOAD_74HC165, OUTPUT);
    pinMode(PIN_CLOCK_ENABLE_74HC165, OUTPUT);
    pinMode(PIN_CLOCK_74HC165, OUTPUT);
    pinMode(PIN_DATA_74HC165, INPUT);
    digitalWrite(PIN_CLOCK_74HC165, LOW);
    digitalWrite(PIN_PLOAD_74HC165, HIGH);

    pinMode(PIN_TERMOSTATO_PIDE_DATO, INPUT);  

    pinMode(PIN_ALIMENTA_CONTACTOS_AGUA, OUTPUT);  
    digitalWrite(PIN_ALIMENTA_CONTACTOS_AGUA, LOW);

    pinMode(PIN_LED_ENVIA_DATO, OUTPUT);  
    pinMode(PIN_ENVIA_DATO_A_TERMOSTATO, OUTPUT);  

    pinMode(PIN_DEBUG, INPUT_PULLUP);  

    Serial.begin(9600);

    DATA_WIDTH = total_data_chips();

    msg_string(F("Iniciando.."), 1);

    texto = F("Total de chips seleccionados: ");
    texto += TOTAL_SHIFT_CHIPS;
    msg_string(texto, 1);

    texto = F("Longitud del array de bits: ");
    texto += (DATA_WIDTH + 1);
    msg_string(texto, 1);

    msg_string(F("Para usar placas esclavas con mas chips, conectar las placas y usar los jumpers para aumentar el ancho de bits"), 1);
    msg_string(F("LISTO PARA RECIBIR EL DISPARO DE TRIGGER O DEL JUMPER 'DEBUG'"), 1);
    Serial.println();

    waiting_msg = millis();
}

void loop()
{
    bool termostato_trigger = digitalRead(PIN_TERMOSTATO_PIDE_DATO);

    Debug_pulsado = !digitalRead(PIN_DEBUG);

    if (Debug_pulsado) 
    {
        msg_string(F("Pulsado Debug (forzando lectura de cables y envio de datos)"));
    }

    if (!termostato_trigger && !Debug_pulsado) return;
    
    msg_string(F("Aplicando tension en cable COM"));

    digitalWrite(PIN_ALIMENTA_CONTACTOS_AGUA, HIGH);
    read_shift_regs();
    digitalWrite(PIN_ALIMENTA_CONTACTOS_AGUA, LOW);

    msg_string(F("Desconectando tension en cable COM"));

    if (Debug_pulsado) mostrar_bytesVal();

    //Calculo del total de cables mojados y el último cable mojado
    byte sum_bytes = 0;
    byte last_byte = 0;
    for(byte i = 0; i < DATA_WIDTH + 1; i++)
    {
        if (bytesVal[i] == 1) 
        {
            last_byte = i+1;
            sum_bytes++;
        }
    }

    if (Debug_pulsado)
    {
        String texto = F("ULTIMO CONTACTO MOJADO: ");
        texto += last_byte;

        texto += F(" (Total mojados = ");
        texto += sum_bytes;
        texto += F(")");

        msg_string(texto);
    }

    send_data_to_echo(last_byte);

    if (Debug_pulsado)
    {
        Serial.println();
    }

    delay(DELAY_LED_ENVIA_DATO);
    digitalWrite(PIN_LED_ENVIA_DATO, LOW);
    delay(DELAY_END_MILISECONDS-DELAY_LED_ENVIA_DATO);
}

void send_data_to_echo(byte value)
{
    unsigned int freq = value * 100;
    if (!freq) freq = 10;

    if (Debug_pulsado) 
    {
        Serial.print(F("ENVIANDO A ECHO VALOR "));
        Serial.println(freq);
    }

    digitalWrite(PIN_LED_ENVIA_DATO, HIGH);
    for(byte loop=0; loop < 20; loop++)
    {
        pulseOut(PIN_ENVIA_DATO_A_TERMOSTATO, freq, HIGH);
    }
}

void pulseOut(byte pinNumber, int pulseWidth, bool state) 
{
    if (state == HIGH) 
    {
        digitalWrite(pinNumber, HIGH);
        delayMicroseconds(pulseWidth);
        digitalWrite(pinNumber, LOW);
        delayMicroseconds(pulseWidth);
    } 
    else {
        digitalWrite(pinNumber, LOW);
        delayMicroseconds(pulseWidth);
        digitalWrite(pinNumber, HIGH);
        delayMicroseconds(pulseWidth);
    }
}		
