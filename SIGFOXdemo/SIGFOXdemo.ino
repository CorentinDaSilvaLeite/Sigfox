//demo de la platine empilable AKENE

#include <EEPROM.h>
#include "Akene.h"
//#include <SoftwareSerial.h> // la carte Akene utilise les broches 4 et 5 pour sa liaison serial
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

// définition de la trame, sa taille totale ne doit pas dépasser 12 octets
typedef struct{ 
    uint8_t Data0;  
    uint8_t Data1;  
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
    static uint8_t data = 17;
    
    //----- Récupérer le nombre de messages déjà émis
    static uint8_t nbMSG=0;
    nbMSG = EEPROM.read( 41 ); // le nbr de messages envoyés est à l'adresse (arbitraire) 41 dans l'eeprom.

	  //---- construction de la trame ----
    Serial << "\nAcquisition des mesures.\n data=" << data << "\n";
    Trame.Data0 = nbMSG;
    Trame.Data1 = data;
    
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


