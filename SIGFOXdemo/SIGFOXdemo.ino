//demo de la platine empilable AKENE

#include <math.h>

#include <EEPROM.h>
#include "Akene.h"
//#include <SoftwareSerial.h> // la carte Akene utilise les broches 4 et 5 pour sa liaison serial
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }


const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int pinTempSensor = A0;     // Grove - Temperature Sensor connect to A0


/* Mesure la température interne de l'ATmega 
float getInternalTemp(void){
  ADMUX = 0xC8;                // Sélectionne le port analogique 8 + référence interne 1v1
  ADCSRA |= (1 << ADEN);       // Active le convertisseur analogique -> numérique
  ADCSRA |= (1 << ADSC);       // Lance une conversion analogique -> numérique
  while(ADCSRA & (1 << ADSC)); // Attend la fin de la conversion
  uint16_t raw = ADCL | (ADCH << 8); // Récupère le résultat de la conversion
  return (raw - 324.31 ) / 1.22;     // Calcul la température
}*/


double GetTemp(void)
{
  unsigned int wADC;
  double t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celsius.
  return (t);
}


// définition de la trame, sa taille totale ne doit pas dépasser 12 octets
typedef struct{ 
    uint8_t Data0;  
    //uint8_t Data1;
    float Data1; 
    float Data2;
} Trame_t;

void setup() {
	  pinMode(13, OUTPUT);  // la LED
	  Serial.begin(9600);
	  delay(3000); // attendre 3 secondes que le modem s'initialise
	  Akene.begin();
}//END setup()

void loop(){
    //---- Déclaration des variables ----
    Trame_t Trame;
    static uint8_t data = 26;
    
    //----- Récupérer le nombre de messages déjà émis
    static uint8_t nbMSG=0;
    nbMSG = EEPROM.read( 41 ); // le nbr de messages envoyés est à l'adresse (arbitraire) 41 dans l'eeprom.

	  //---- construction de la trame ----

    //---- tmperature externe
    int a = analogRead(pinTempSensor);

    float R = 1023.0/a-1.0;
    R = R0*R;

    float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet

   // Serial << "temperature = " << temperature << ".\n";

    //delay(100);


    //---- tmperature interne
    /* Lit la température du capteur interne de l'ATmega */
    float temp = GetTemp();


    Serial << "\nAcquisition des mesures.\n temperature externe = " << temperature << "\n";
    Serial << "Temperature interne = " << temp << "\n";
    Trame.Data0 = nbMSG;
    Trame.Data1 = temperature;
    Trame.Data2 = temp;
    
	  //---- envoi des données vers le cloud ----  
    while( !Akene.isReady() ){// On attend que le modem soit prêt
        Serial << "Le modem n'est pas prêt\n";
        PORTB ^= 0x20; // LED = bit 5 du port B
        delay(100);
    }//END while( !Akene.isReady() )
    Akene.send(&Trame,sizeof(Trame));
    Serial << "La trame " << nbMSG << " est partie.\n";
    
    //---- Mise à jour du nombre de messages envoyés
    nbMSG++;
    EEPROM.write(41,nbMSG);

	  //---- on ne doit pas envoyer plus de 140 messages par jour ----
    Serial << "Attente de 5 minutes. ";
    for( byte i=0; i<50; i++){ delay(6000); Serial << "." ; }
    
}//END loop()

