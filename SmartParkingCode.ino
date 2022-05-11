#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <Servo.h>

#define CAPACITY_MAX 9
#define B_IN  0
#define B_OUT 1
#define SOUND_VELOCITY 0.034

U8G2_SH1107_SEEED_128X128_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

//capteur
class Capteur
{
  const int trigPin;
  const int echoPin;
  long duration;
  float distanceCm;
  public:
  Capteur(int tPin,int ePin) : trigPin(tPin), echoPin(ePin){
    pinMode(tPin, OUTPUT);
    pinMode(ePin, INPUT);
  }
  bool getStatus(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calcul de la distance
  distanceCm = duration * SOUND_VELOCITY/2;
    if (distanceCm <= 10.0){
      return true;
    }else{
      return false;
    }
  }
};
//Redéfinition d'opérateur
// class Servo {
//    Servo & operator ++(int){
//       myServo->write(180);
//    return(*this);}
//    Servo & operator --(int){
//       myServo->write(0);
//    return(*this);}
//}
//
//barriere
class Barriere
{
protected:
  int *nbrPlacesLibres;
  Capteur *capteur;
public:
  virtual bool isThereAvehicule() = 0;
  bool servirVehicule = false;
  Servo *myServo;
  virtual void tick(void) = 0;
};
class BarriereIn : public Barriere
{
public:
  BarriereIn(int *nbr){
    capteur = new Capteur(12,14);
    myServo = new Servo;
    myServo->attach(D3);
    nbrPlacesLibres = nbr;
  }
 virtual bool isThereAvehicule(){
        return capteur->getStatus();
    };
 virtual void tick(void){
    isThereAvehicule();
    if(isThereAvehicule() == true){
        if((*nbrPlacesLibres) > 0){
            if(servirVehicule == false){
                servirVehicule = true;
                myServo->write(180);
                (*nbrPlacesLibres)--; 
            }
        } 
    } else {
      servirVehicule = false;
      myServo->write(0);
    }
 }
};
class BarriereOut : public Barriere
{
public:
  BarriereOut(int *nbr){
    //  Il faut trouver les bonnes pin pour connecter le deuxieme capteur
    capteur = new Capteur(D4,D7);
    myServo = new Servo;
    myServo->attach(15);
    nbrPlacesLibres = nbr;
  }
 virtual bool isThereAvehicule(){
        return capteur->getStatus();
    };
 virtual void tick(void){
    isThereAvehicule();
    if(isThereAvehicule() == true){
        if((*nbrPlacesLibres) < CAPACITY_MAX){
            if(servirVehicule == false){
                servirVehicule = true;
                myServo->write(180);
                (*nbrPlacesLibres)++; 
                //afficheur->print(*nbrPlacesLibres, "Soyez les", "bienvenues", "");
            }
        } //else{
           // if(servirVehicule == false)
                //afficheur->print(0, "Merci de", "revenir", "plus tard"); 
       // }
    } else {
      servirVehicule = false;
      myServo->write(0);
      //afficheur->update();
    }
 }
};
//forward déclaration
class Parking;
class Afficheur
{
  public:
  Parking *parking;
  Afficheur(Parking *p){
    parking = p;
  };
public:
// fonction qui sert à afficher à l'écran 
  void print(char nbrPlacesLibres, char *str0, char* str1, char *str2, char *str3){
    char buffer [16];
    do {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        buffer[0] = ' ';
        buffer[1] = ' ';
        buffer[2] = ' ';
        buffer[3] = ' ';
        buffer[4] = ' ';
        buffer[5] = '0' + nbrPlacesLibres;
        buffer[6] = ' ';
        buffer[7] = 'p';
        buffer[8] = 'l';
        buffer[9] = 'a';
        buffer[10] = 'c';
        buffer[11] = 'e';
        buffer[12] = 's';
        buffer[13] = 0;
        u8g2.drawStr(20,40,buffer);
        u8g2.drawStr(20,30,str0);
        u8g2.drawStr(20,60,str0);
        u8g2.drawStr(20,75,str1);
        u8g2.drawStr(20,90,str2);
        u8g2.drawStr(20,105,str3);
    } while ( u8g2.nextPage() );
  }
  void update();
};
class Vehicules
{

};
class Voiture : public Vehicules
{

};
class Camion : public Vehicules
{

};
//declaration parking
class Parking
{
  public: Parking();
private:
  //bool toggle = false;
  Vehicules *vehicules[CAPACITY_MAX];
  
public:
    void tick();

public:
  int nbrPlacesLibres;
  Barriere *barriere[2];
  Afficheur *afficheur;
};

//implementation parking en dehors de la class Parking
Parking::Parking(){
    //Utilisation d'exceptions
    //if (CAPACITY_MAX<0) throw 1;
    nbrPlacesLibres = CAPACITY_MAX;
    barriere[B_IN] = new BarriereIn(&nbrPlacesLibres);
    barriere[B_OUT] = new BarriereOut(&nbrPlacesLibres);
    afficheur = new Afficheur(this);
}
void Parking::tick(){
  //Pour éliminer les interferences
   //if(toggle == true){
     // barriere[B_IN]->tick();
      //toggle = false;
   //}else{
    //  barriere[B_OUT]->tick();
     // toggle = true;
  // }
  barriere[B_IN]->tick();
  barriere[B_OUT]->tick();
   if((barriere[B_IN]->servirVehicule == true) or (barriere[B_OUT]->servirVehicule == true)){
      afficheur->print(nbrPlacesLibres,"****************", "Soyez les", "bienvenues", "");
   }else{
      if(nbrPlacesLibres > 0){
          afficheur->print(nbrPlacesLibres,"****************","Soyez les", "bienvenues", "");
      }else{
          afficheur->print(0,"****************","Merci de", "revenir", "plus tard"); 
      }
   }
}

Parking *parking;
void setup() {
  // put your setup code here, to run once:
  u8g2.begin();
  u8g2.clear();
  parking = new Parking;
}
void Afficheur::update(){
    if(parking->nbrPlacesLibres > 0){
        print(parking->nbrPlacesLibres,"****************", "Soyez les", "bienvenues", ""); 
    } else{
      if((parking->barriere[0])->servirVehicule == false){
        print(0,"****************", "Merci de", "revenir", "plus tard"); 
      } else {
        print(parking->nbrPlacesLibres,"****************", "Soyez les", "bienvenues", ""); 
      }
    }
}
void loop() {
  // put your main code here, to run repeatedly:
    u8g2.firstPage();
    //try{
    parking->tick();
    //}
    //catch(int erreur){
    //if(erreur == 1){
    //   afficheur->print("Erreur capacité max"); 
    //}else{
    //   afficheur->print("Exception inconnue"); 
    //}
    //}
}
