/*
#       _\|/_   A ver..., ¿que tenemos por aqui?
#       (O-O)        
# ---oOO-(_)-OOo---------------------------------
 
 
##########################################################
# ****************************************************** #
# *           DOMOTICA PARA PRINCIPIANTES              * #
# *   Estacion barometrica simple para uso educativo   * #
# *            Autor: Eulogio López Cayuela            * #
# *                                                    * #
# *          Versión 1.0    Fecha: 17/04/2016          * #
# ****************************************************** #
##########################################################
*/

/*
    NOTAS SOBRE ESTA VERSION:
      Muestra solo datos de presion y temperatura 
      pues esta basada en un unico sensor, el BMP180
      Muestra datos en LCD 16x2

*/


//------------------------------------------------------
//algunas definiciones personales para mi comodidad al escribir codigo
//------------------------------------------------------
#define AND &&
#define OR ||

//------------------------------------------------------
//Otras definiciones para pines y variables
//------------------------------------------------------
#define LCD_AZUL_ADDR    0x27  // Direccion I2C de nuestro LCD color azul
#define LCD_VERDE_ADDR   0x3F  // Direccion I2C de nuestro LCD color verde
#define PIN_LED 13  //luz de corteria


//------------------------------------------------------
//Definiciones para pines y variables del sensor DHT11
//------------------------------------------------------
#define DHT11_PIN 8 //pin 8 para conectar el sensor DHT11


//------------------------------------------------------
//Importamos las librerias necesarias
//------------------------------------------------------
#include <Wire.h> //libreria para comunicaciones I2C
#include <LiquidCrystal_I2C.h>  // LiquidCrystal library
#include <SFE_BMP180.h>  //libreria para el sensor de presion y temperatura

//------------------------------------------------------
// Creamos las instancia de los objetos:
//------------------------------------------------------

//Creamos el objeto 'lcd' como una instancia del tipo "LiquidCrystal_I2C"
//                             addr, en,rw,rs,d4,d5,d6,d7,bl, blpol
LiquidCrystal_I2C lcd(LCD_AZUL_ADDR,  2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 


//Creamos el objeto sensor BMP180
SFE_BMP180 sensorBMP180;   //Creamos el objeto 'sensorBMP180' como una instancia del tipo "SFE_BMP180"

//#define ALTITUD 407.0  // Altitud de "Sorbas" en metros
//#define ALTITUD 375.0  // Altitud de "Colegio NS de Gador (Berja)" en metros

#define ALTITUD 407.0    // Altitud de "Sorbas" en metros
float altitud_estimada;  //guarda el calculo de altitud estimada en funcion de la presion Sin utilidad practica
float Temperatura = 0;   //variable para la temperatura leida del BMP180
float PresionRelativaCotaCero = 0;
float PresionABS = 0;    //variable para la presion leida del BMP180



//#####################################################################################################

//*******************************************************
//         FUNCION DE CONFIGURACION
//*******************************************************

void setup()
{
  Serial.begin(9600);

  if (sensorBMP180.begin()){
    Serial.println("Sensor BMP180 ok");
  }
  else
  {
    // Si no se encuentra el sensor o este no responde se da un mensaje de error.
    Serial.println("Fallo del sensor BMP180\n\n");
  }

  lcd.begin (16,2);    //Inicializar lcd 
  //Mensaje inicial para demostrar que el LCD esta bien conectado y comunicado
  lcd.clear();         //Reset del display 
  lcd.setBacklight(true);     //Activamos la retroiluminacion
  lcd.setCursor(0, 0);
  //lcd.print("0123456789ABCDEF");
  lcd.print(" - BAROMETRO -  ");
  lcd.setCursor(0, 1);
  //lcd.print("0123456789ABCDEF");
  lcd.print(" Educativo v1.0 ");
  delay(3500);
  lcd.clear();         //Reset del display

  lcd.setCursor(0, 0);
  //lcd.print("0123456789ABCDEF");
  lcd.print("Tempe: ");  
  lcd.setCursor(14, 0);
  lcd.print("C");
  lcd.setCursor(0, 1);
  //lcd.print("0123456789ABCDEF");
  lcd.print("P rel:        mb");
  delay(100);

  //lcd.noBacklight();   //desactivamos la retroiluminacion
  
  //apagamos el led 'On Board'
  pinMode(13, OUTPUT); //PIN13 como salida
  digitalWrite(13, LOW); //apagar PIN13

  mostarDatosLCD_v3();
}

//#####################################################################################################

//*******************************************************
//            BUCLE PRINCIPAL DEL PROGRAMA
//*******************************************************

void loop()
{                
  unsigned long tiempoActual;
  tiempoActual = millis();
  if(tiempoActual%2500 < 250){
    mostarDatosLCD_v3();
    delay(250);
  }
}



//#####################################################################################################

// ***********************************************************************************************
//  BAROMETRO, usando sensor BMP180
// ***********************************************************************************************
void leerDatosSensorBMP180()
{
  char estado;
  double T,P,p0,a;

  /* Primero se debe hacer una lectura de la temepratura para poder hacer una medida de presion.
  Se inicia el proceso de lectura de la temperatura.
  Si se realiza sin errores, se devuelve un numero de (ms) de espera, si no, la funcion devuelve 0.
  */
  
  estado = sensorBMP180.startTemperature();
  if (estado != 0)
  {
    // pausa para que se complete la medicion en el sensor.
    delay(estado);

    // Obtencion de la medida de temperatura que se almacena en T:
    // Si la lectura el correcta la funcion devuelve 1, si se producen errores, devuelve 0.

    estado = sensorBMP180.getTemperature(T);
    if (estado != 0)
    {
      Temperatura = T;  //Asignacion a variable global
      
      /* Se inicia el proceso de lectura de la presion.
         El parametro para la resolucion de muestreo varia de 0 a 3 (a mas resolucion, mayor tiempo necesario).
         Si se realiza sin errores, se devuelve un numero de (ms) de espera, si no, la funcion devuelve 0.
      */

      estado = sensorBMP180.startPressure(3);
      if (estado != 0)
      { 
        delay(estado); // pausa para que se complete la medicion en el sensor.
        
        // Obtencion de la medida de Presion que se almacena en P:
        // Si la lectura el correcta la funcion devuelve 1, si se producen errores, devuelve 0.
        estado = sensorBMP180.getPressure(P,T);

        if (estado != 0)
        {
          PresionABS = P - 4;  //correccion para este sensor a variable global

          /* 
          El sensor devuelve presion absoluta. Para compensar el efecto de la altitud
          usamos la funcion interna de la libreria del sensor llamada: 'sealevel'
          P = presion absoluta en (mb) y ALTITUD = la altitud del punto en que estomos (m).
          Resultado: p0 = presion compensada a niveldel mar en (mb)
          */

          p0 = sensorBMP180.sealevel(PresionABS,ALTITUD); // 407 metros (SORBAS)
          PresionRelativaCotaCero= p0;  //Asignacion a variable global
        }
        else Serial.println("error obteniendo la presion\n");
      }
      else Serial.println("error obteniendo la presion\n");
    }
    else Serial.println("error obteniendo la temperatura\n");
  }
  else Serial.println("error obteniendo la temperatura\n");
}



//*******************************************************
//  FUNCION PARA MOSTAR TEMPERATURA/HUMEDAD POR  LCD
//*******************************************************
void mostarDatosLCD_v3()
{
  leerDatosSensorBMP180(); //captura los datos del sensor BMP180 (Temperatura, PresionABS)


  //lcd.clear();         //Reset del display 
  lcd.setBacklight(true);     //Activamos la retroiluminacion
  //lcd.print("0123456789ABCDEF");
  lcd.setCursor(9, 0);
  lcd.print(Temperatura,1);
  
  lcd.setCursor(7, 1);
  if (PresionRelativaCotaCero <1000){
    lcd.print("P rel:        mb");
    lcd.setCursor(8, 1);
  }
    
  lcd.print(PresionRelativaCotaCero,1); // PresionRelativaCotaCero
  //lcd.print(PresionABS,1); //PresionABS
}



//*******************************************************
//                    FIN DE PROGRAMA
//*******************************************************
