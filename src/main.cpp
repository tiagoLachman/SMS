#include <Arduino.h>
#include <SoftwareSerial.h>

#include <dados.h> //Dados pessoais dos números e nomes cadastrados.

/*
Dados definidos na biblioteca "dados.h"
const char* constTextos[];
const char* constNumeros[];
const char* constNomes[];
*/

#define tempo_aguardar 1800 //tempo, em segundos, de aguardo para verificar se as máquinas já estão operando


//Pinos de comunicação com o SIM800L
#define SIM800_TX_PIN 7
#define SIM800_RX_PIN 8

const int pinVetor[]={
  A0, //FALTA DE ENERGIA
  A1, //CHILLER 1 FALHA GERAL
  A2, //CHILLER 2 FALHA GERAL
  A3, //CHILLER 3 FALHA GERAL
  A4, //BOMBA HIDRAULICA CHILLER 1
  A5, //BOMBA HIDRAULICA CHILLER 2
  10, //BOMBA HIDRAULICA CHILLER 3
  11, //BOMBA DA TORRE DE ARREFECIMENTO 1 PARADO
  2,  //BOMBA DA TORRE DE ARREFECIMENTO 2 PARADO
  3,  //BOMBA DA TORRE DE ARREFECIMENTO 3 PARADO
  4,  //FALHA VENTILADOR DA TORRE 1 PARADO DESARME
  5,  //FALHA VENTILADOR DA TORRE 2 PARADO DESARME
  6   //FALHA VENTILADOR DA TORRE 3 PARADO DESARME
};

const int lenPinVetor = sizeof(pinVetor)/sizeof(pinVetor[0]);
int vetorAlt[lenPinVetor];

const int lenNumeros = sizeof(constNumeros)/sizeof(constNumeros[0]);
const int lenNomes = sizeof(constNomes)/sizeof(constNomes[0]);

//Software serial para a comunicação com o SIM800L
SoftwareSerial serialSIM800(SIM800_TX_PIN, SIM800_RX_PIN);

void mandarMSG(const char* nome, const char* numero , const char* mensagem, SoftwareSerial* moduloSMS);
void mandarMensagens(
  const char** nomes,
  const char** textos,
  const char** numeros,
  int lenNumeros,
  int indice,
  SoftwareSerial* moduloSMS
);

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Inciando Setup...");
  if(lenNumeros != lenNomes){
    while(1){
      Serial.println("Quantidade de números cadastrados não é igual a qnt de nomes");
      delay(5000);
    }
  }

  for(int i=0;i<lenPinVetor;i++)
    pinMode(pinVetor[i], INPUT);
  delay(1000);
  //Being serial communication witj Arduino and SIM800
  serialSIM800.begin(9600);
  delay(1000);
  for(int i=0;i<lenPinVetor;i++)
    vetorAlt[i] = 0;
  Serial.println("Setup Completo!");
  delay(1000);
}

void loop(){
  int i=0;
  int cont=0;
  Serial.println("Aguardando falhas...");
  
  while(cont < tempo_aguardar){
    for(i=0;i<lenPinVetor;i++){
      if(digitalRead(pinVetor[i])==1 && vetorAlt[i] == 0){
        if(vetorAlt[0] == 1){
          break;
        } else if(digitalRead(pinVetor[0])==1 && vetorAlt[0] == 0){
          mandarMensagens(constNomes, constTextos, constNumeros, lenNumeros, 0, &serialSIM800);
          vetorAlt[0] = 1;
        } else{
          mandarMensagens(constNomes, constTextos, constNumeros, lenNumeros, i, &serialSIM800);
          vetorAlt[i] = 1;
        }
      }
    }
    delay(1000);
    cont++;
  }

  for(i=0;i<lenPinVetor;i++){
    if(digitalRead(pinVetor[i]) < vetorAlt[i]){
      vetorAlt[i] = 0;
    } else{
      vetorAlt[i] = digitalRead(pinVetor[i]);
    }
    //Sugestão
    //vetorAlt[i] = digitalRead(pinVetor[i]) < vetorAlt[i] ? 0 : 1;
  }
}

void mandarMensagens(
  const char** nomes,
  const char** textos,
  const char** numeros,
  int lenNumeros,
  int indice,
  SoftwareSerial* moduloSMS
){
  for(int i=0;i<lenNumeros;i++){
    mandarMSG(nomes[i],numeros[i],textos[indice], moduloSMS);
  }
}

void mandarMSG(const char* nome, const char* numero , const char* mensagem, SoftwareSerial* moduloSMS){
  moduloSMS->write("AT+CMGF=1\r\n");
  delay(1000);

  String tempString = "AT+CMGS=\""+String(numero)+"\"\r\n";
  char tempChar[200];
  tempString.toCharArray(tempChar,200);
  //Manda um comando para se conectar com o número via SMS
  Serial.println("Enviando...");
  Serial.println(tempChar);
  moduloSMS->write(tempChar);
  delay(1000);

  //Manda o conteúdo do SMS
  tempString = String(nome)+", "+String(mensagem);
  tempString.toCharArray(tempChar, 200);
  Serial.println(tempChar);
  moduloSMS->write(tempChar);
  delay(1000);

  //Manda Ctrl+Z / ESC para dizer que a mensagem SMS terminou
  moduloSMS->write((char)26);
  delay(5000);
}
