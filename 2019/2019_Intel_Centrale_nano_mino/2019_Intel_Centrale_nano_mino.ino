/* Code de l'Arduino gérant l'intelligence centrale du Minotaure
  Son but est de récolter toutes les informations des capteurs et de prendre des décisions en conséquence
  @author: Dany VALADO (2018) Lucien PRUVOT Paul THONNEY
  DATE: 30.06.19
  REMERCIEMENTS: Merci à Maxime SCHARWATH, Joan MAILLARD et Léonard BESSEAU pour leur aide
*/

#include <Wire.h> //I2C
//toutes les adresses I2C

#define ADDR_INTELLIGENCE_CENTRALE 0x0 // adresse de l'intelligence centrale, arduino nano sur le PCB Bluetooth
#define ADDR_TRACKING 0x10 //16 Arduino Nano se trouvant sur le PCB ADDR_TRACKING (ici c'est l'angle du servo qui est transmit)
#define ADDR_CONTACT 0x11 //17  PCB HMI, Nano se trouvant à gauche lorsqu'on regarde le U depuis sa base. Il gère les plaque de contact et les LED
#define ADDR_SOUND  0x12 //18
#define ADDR_WHEEL 0x13 //19 PCB Puissance, "Arduino 2" Nano
#define ADDR_EYES 0x14 //20

#define IS_MINOTAURE true

#define LOGS_DELAY 2000

//DEFINE ROBOT

#define MAX_LIFE 6
#define CONTACT_DEFAULT_MODE 1 //Valueur par defaut => 0: Tracking, 1: Rainbow, 2:AnimShield, 3: BlinkAll(RED)

//BUTTONS
#define BUFFER_SIZE 9
#define JOYSTICK_MARGIN 0.02f

//MANETTE STATES
#define CONNECTED 255

#define EASY 0
#define MEDIUM 1
#define HARD 2

#define MAX_SPEED 0
#define COOLDOWN 1

bool ButtonA(bool flanc = false);
bool ButtonB(bool flanc = false);
bool ButtonX(bool flanc = false);
bool ButtonY(bool flanc = false);
bool ButtonWEST(bool flanc = false);
bool ButtonEAST(bool flanc = false);
bool ButtonNORTH(bool flanc = false);
bool ButtonSOUTH(bool flanc = false);
bool ButtonLB(bool flanc = false);
bool ButtonRB(bool flanc = false);
bool ButtonLS(bool flanc = false);
bool ButtonRS(bool flanc = false);
bool ButtonSTART(bool flanc = false);
bool ButtonSELECT(bool flanc = false);
bool flancsMontants[14];

bool stateModules[5];

byte modules[5] =  {ADDR_TRACKING, ADDR_CONTACT, ADDR_SOUND, ADDR_WHEEL, ADDR_EYES};

byte dataBuffer[BUFFER_SIZE];
byte controllerOutput = 0;
byte controllerBuzzer = 0; //0-15
byte controllerVibrator = 0; //0-15

byte memOutput = 0;

typedef enum State { // On définit les états possible de la machine
  Automatique,
  Manuel,
  PauseGenerale,
  MenuSelection,
  MenuGO,
  Disconnected,
  Difficulty,
} State;

State setState(State state, int menuPos = -1);
State savedMode = State::Manuel;
State currentState = State::MenuSelection; // On démarre sur le menu de sélection
State previousState; // Ancien état

unsigned long timeStartAt;
unsigned long lastContactAt = 0;
unsigned long askResponseAt = 0;
unsigned long lastPingAt = 0;
unsigned long nbRequest;
unsigned long nbTransmission;
unsigned long lastAmbiantAt = 0;
long loopTime;

bool isStartedState = false;

bool waitingResponse = false;

bool sendEyes(int id, int data = -1);
bool sendSound(int id, int data = -1);
bool sendContact(int id, int data = -1, int duration = -1);

int stateMenuPos = 0; // Position du curseur

// maxSpeed[%] - hurtCooldown[ms]
int difficulty[][2] = {
  {30, 4000},//easy
  {40, 2500},//medium
  {50, 1000}//hard
};
int currentDifficulty = EASY; // On défini la difficulté ici (le menu de selection n'est pas implémenté)

int getDifficulty(int id) {
  return difficulty[currentDifficulty][id];
}

//AUTOMATIQUE
long lastUpdateHead = 0;
int headAngle = 0;
byte targetDistance = 0;
bool isFindTarget = false;

//ROBOT
byte currentLife = MAX_LIFE;
int hurtCooldown = 0;
int currentMotorValue[2];
//END ROBOT
unsigned long lastLogAt;

void setup() {
  Wire.begin(ADDR_INTELLIGENCE_CENTRALE);
  Serial.begin(115200);

  //Serial.println("Setup completed");
  delay(1000);
  pingModules();
  setupRobot(); // on setup le robot une première fois
  sendSound(0, 0);// Starting sound
}


/*
   @func void loop Est la boucle centrale du code c'est elle qui appelle les fonctions principales
   @param null
   @return void
*/
void loop() {
  timeStartAt = millis();
  communicationController(); // On commence par communiquer les dernières infos avec la manette
  pingModules(); // On ping les modules pour voir lesquels répondent
  loopAmbiant(); // On execute le loopAmbiant pour jouer des sons et des animations sur les yeux
  if (millis() > askResponseAt + 25) {
    askResponseAt = 0;
    waitingResponse = false;
  }
  switch (currentState) {
    case State::Automatique: { // Mode automatique du robot
        loopAutomatique();
        break;
      }
    case State::Manuel: { // Mode manuel du robot
        loopManuel();
        break;
      }
    case State::PauseGenerale: { // mode menu de pause
        loopPauseGenerale();
        break;
      }
    case State::MenuSelection: { // Mode menu principal
        loopMenuSelection();
        break;
      }
    case State::MenuGO: { // Mode MenuGo
        loopMenuGo();
        break;
      }
    case State::Disconnected: { // Mode Disconnected
        loopDisconnected();
        break;
      }
    case State::Difficulty: { // Mode Difficulty
        loopDifficulty();
        break;
      }
  }
  loopTime = millis() - timeStartAt; // On mesure combien de temps le loop a pris pour s'executes en entier

  //logs(); // On affiche les logs dans le seriel
}

/*
   @func void setupRobot Est appelée pour setup les robot une première fois ou pour les remettres à zéro
   après une mort ou un retour au menu principal de sélection
   @param null
   @return void
*/
void setupRobot() {
  currentLife = MAX_LIFE;
  hurtCooldown = 0;
  //sendTracking(0x2E);
  sendEyes(0);
  sendContact(CONTACT_DEFAULT_MODE);
  sendSound(250);//StopSound
}

/*
   @func int boostSpeed Fonction qui attribue un boost en vitesse le temps du cooldown après avoir pris un coup
   @param null
   @return percent
*/
int boostSpeed() {
  int percent = 0;
  if (isCooldown()) {
    percent += 15;
  }
  return percent;
}

/*
   @func void resumeGame Fonction qui stop la pause des yeux, le son et remet les contacts en mode jeu
   @param null
   @return void
*/
void resumeGame() {
  sendEyes(0);
  sendContact(CONTACT_DEFAULT_MODE);
  sendSound(250);//StopSound
}

/*
   @func bool vérifie si l'on est en cooldown ou pas
   @param null
   @return bool
*/
bool isCooldown() {
  return (millis() < hurtCooldown);
}

/*
   @func bool hurt Execute les actions liées au fait de prendre un coup (affichage, mort, point de vie, communication avec les yeux et le son)
   @param int dmg
   @return bool
*/
bool hurt(int dmg) {
  if (dmg <= 0)return false;
  //Serial.println("Cooldown:" + String(isCooldown()));
  if (millis() < hurtCooldown)return false; //Cooldown
  int cooldownDuration = getDifficulty(COOLDOWN);
  hurtCooldown = millis() + cooldownDuration;
  currentLife -= dmg;
  //Serial.println(String(dmg) + " dmg");
  // Serial.println(String(currentLife) + " lives");
  if (currentState == State::Automatique) {
    controllerOutput = 4 + (6 - currentLife);
    memOutput = controllerOutput;
  }
  if (currentState == State::Manuel) {
    controllerOutput = 10 + (6 - currentLife);
    memOutput = controllerOutput;
  }
  if (currentLife <= 0) {//DEAD
    currentLife = 0;
    die();
  } else {
    sendEyes(1);
    sendSound(1);//HURT
    sendContact(3, CONTACT_DEFAULT_MODE, (cooldownDuration / 250)); // blink pendant (x*250ms)
  }
  return true;
}

/*
   @func void die envoie les information d'affichage et de son lors d'une mort
   @param null
   @return void
*/
void die() {
  sendEyes(3);//eyeDead
  sendSound(3);//soundDead
  sendContact(4);//animDead
}

/*
   @func bool isDead Répond oui si le robot est mort (sa vie est plus petite ou égale à 0)
   @param null
   @return bool
*/
bool isDead() {
  return (currentLife <= 0);
}

/*
   @func void Boucle qui vérifie les dégats pris en appelant l'arduino des contact et execute la fonction hurt() en conséquence
   @param null
   @return void
*/
void loopHurt() {
  if (millis() <= lastContactAt + 250) {
    return;
  }
  lastContactAt = millis();
  byte buffer[10];
  if (getData(ADDR_CONTACT, buffer, 2)) {
    if ((bool)buffer[0] == true) {
      int dmg = 0;
      // Serial.println("Which Contact:" + String(buffer[1]));
      switch (buffer[1]) { // On peut si on veut modifier le nombre de dégat pris selon les plaques touchées
        case 0: dmg = 1; break;
        case 1: dmg = 1; break;
        case 2: dmg = 1; break;
        case 3: dmg = 1; break;
        case 4: dmg = 1; break;
        case 5: dmg = 1; break;
      }
      hurt(dmg);
    }
  }
}

/*
   @func void pingModule Fonction qui ping les différents modules arduino pour vérifier leur état de connection
   Elle stocke ensuite l'état des modules dans stateModules
   @param null
   @return void
*/
void pingModules() {
  if (millis() <= lastPingAt + 500) {
    return;
  }
  lastPingAt = millis();
  for (int i = 0; i < sizeof(modules) / sizeof(modules[0]); i++) {
    bool state = pingAddr(modules[i]);
    if (state != stateModules[i]) {
      // Serial.println("Module " + String(i) + " " + String((state ? "CONNECTED" : "DISCONNECTED")));
      sendSound(0, state ? 1 : 2);
    }
    stateModules[i] = state;
  }
}

/*
   @func bool Ping le module à l'adresse demandée et retourne si elle a reçuu réponse
   @param int addr adresse du module à pinger
   @return bool
*/
bool pingAddr(int addr) {
  // if (waitingResponse)return false;
  nbTransmission++;
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

/*
   @func bool getData demande au module demandé de lui transmettre son data et le stocke dans un buffer
   @param in addr adresse du module demandé
   #param byte *buffer pour y ranger le data
   #param int nbBytes nombre de bytes attendu lors de la transmission
   @return bool
*/
bool getData(int addr, byte *buffer, int nbBytes) {
  if (waitingResponse)return false;
  if (!pingAddr(addr))return false;
  nbRequest++;
  waitingResponse = true;
  Wire.requestFrom(addr, nbBytes); // Request the transmitted two bytes from the two registers
  if (Wire.available() <= nbBytes) {
    int i = 0;
    while (Wire.available()) {
      if (i >= nbBytes)break;
      buffer[i] = Wire.read();
      i++;
    }
    waitingResponse = false;
    return true;
  }
  return false;
}

/*
   @func bool sendData envoie du data au module demandé
   @param int addr adresse du module demandé
   #param byte buffer Stockage de l'info à envoyer
   #param int nbBytes nombre de bytes à envoyer
   @return bool
*/
bool sendData(int addr, byte *buffer, int nbBytes) {
  if (waitingResponse)return false;
  nbTransmission++;
  Wire.beginTransmission(addr);
  for (int i = 0; i < nbBytes; i++) {
    Wire.write(buffer[i]);
  }
  return Wire.endTransmission() == 1;
}

/*
  @func void loopAmbiant Joue des sons et des animations des yeux pour donner une ambiance
  @param null
  @return bool
*/
void loopAmbiant() {
  if (isDead())return;
  if (IS_MINOTAURE) {
    if (millis() > lastAmbiantAt) {
      sendSound(4);//GROWL
      sendEyes(6);//ANGRY
      lastAmbiantAt = millis() + random(5000, 15000);
    }
  }
}

/*
  @func void loopDisconnected Suivant l'état de la manette (connectée/déconnectée) cette fonction
  coupe les moteurs et envoie les info d'affichage et de son
  @param null
  @return void
*/
void loopDisconnected() {
  if (onStartState()) {
    sendEyes(7);
    sendMotorValue(0, 0); // coupe les moteurs par sécurité lors d'une déconnexion de manette
    sendMotorValue(1, 0);
    sendSound(0, 4);
    sendContact(4);
  }
  if (InfoController() == CONNECTED) {
    setState(State::MenuSelection); // si la manette est à nouveau connectée on va au menu de selection
    sendEyes(8);
    sendSound(0, 5);
    sendContact(CONTACT_DEFAULT_MODE);
  }
}
/*
  @func int getSpeed applique le boost de vitesse éventuel et renvoie la vitesse transformée
  @param int speed On lui transmet la vitesse demandée
  @return int vitesse transformée ou non par le boost
*/
int getSpeed(int speed) {
  return ((100 + boostSpeed()) * speed) / 100.0;
}

/*
  @func void loopAutomatique gère le mode automatique du robot.
  récupère les informations transmisent par le module de tracking et les traite pour ensuite donner des ordres de
  vitesse et de direction aux roues. Gère aussi l'appel de loopHurt() pour gèrer les dégats.
  @param null
  @return void
*/
void loopAutomatique() {
  if (onStartState()) {//Seulement la première fois qu'il rentre dans la loop
    sendTracking(0x1E);
    if (memOutput != 4) {
      controllerOutput = 4 ; // affiche le mode de jeu avec vie pleine
    } else {
      controllerOutput = memOutput ;
    }
  }

  if (checkPause()) { // quitte directement la loop si la pause est pressée et évite que le "state" puisse être changé dans la fonction
    return;
  }

  loopHurt(); // appelle la gestion des dégats

  //vérifie si le robot est mort ou pas
  if (isDead()) {

    sendMotorValue(0, 0);
    sendMotorValue(1, 0);
    setState(State::MenuSelection); // retourne au menu de selection de mode de jeu
    setupRobot(); // setup le robot à sa mort pour remettre tous les paramètres de jeu à 0
    return;
  }

  if (millis() - lastUpdateHead >  25) { //demande à la pixy ces valeurs toutes les 25ms

    byte buffer[10];
    if (getData(ADDR_TRACKING, buffer, 3)) {
      headAngle = map(buffer[0], 0, 180, -90, 90);
      targetDistance = buffer[3];
      isFindTarget = buffer[2];

      waitingResponse = false;
      lastUpdateHead = millis();
    }
    //sendEyes(5, (3 << 3) | map(headAngle, -90, 90, 0, 5));
  }

  /* //Je n'ai pas eu le temps de comprendre pourquoi ce système de tracking ne fonctionne pas il semble que
     //le minotaure perd l'information de isFindTarget. Je laisse donc ce bout de code ici pour les futures générations sur le projet.

    short speedL = 0;
    short speedR = 0;

    //28.06.19 18h56 and 28.06.19 19h42
    if (isFindTarget) {
      if (headAngle >= 10) {
        speedR = getSpeed(map(abs(headAngle), 0, 90, getDifficulty(MAX_SPEED), 0));
        speedL = getSpeed(getDifficulty(MAX_SPEED));
      }
      else if (headAngle <= -10) {
        speedR = getSpeed(getDifficulty(MAX_SPEED));
        speedL = getSpeed(map(abs(headAngle), 0, 90, getDifficulty(MAX_SPEED), 0 ));
      }
      else if (-10 < headAngle && headAngle < 10) {
        speedR =  getSpeed(getDifficulty(MAX_SPEED));
        speedL = getSpeed(getDifficulty(MAX_SPEED));
      }

      //Envoie les infos au moteur
      sendMotorValue(1, -speedL);
      sendMotorValue(0, -speedR);
    }
    else {
      short speed = getSpeed(map(abs(headAngle), 0, 90, 0 , getDifficulty(MAX_SPEED) / 1.25));
      if (headAngle < 0) {
        sendMotorValue(0, -speed);
        sendMotorValue(1, speed);
      } else {
        sendMotorValue(0, speed);
        sendMotorValue(1, -speed);
      }
    }*/
  //29.06.19 15h04 Cet algorithme est un bricolage créé pour la présentation finale car celui que l'on voulait utiliser (ci-dessus) 
  //ne fonctionne pas et nous n'avons pas eu le temps de comprendre pourquoi...
  if (headAngle > -30 && headAngle < 30) { // si la cible est "en face" et qu'il est en train de la tracker il fonce
    if (isFindTarget) {
      int speed = getSpeed(map(targetDistance, 0, 255, 20, getDifficulty(MAX_SPEED) * 1.25));
      sendMotorValue(0, -speed); // les moteurs sont monté à l'envers sur le minotaure
      sendMotorValue(1, -speed);
    }

  } else { // tourne doucement sur lui même pour trouver une cible
    int speed = getSpeed(map(abs(headAngle), 0, 90, 0, getDifficulty(MAX_SPEED) / 1.25));
    if (headAngle < -10) {
      sendMotorValue(0, -speed);
      sendMotorValue(1, speed);
    } else if (headAngle > 10) {
      sendMotorValue(0, speed);
      sendMotorValue(1, -speed);
    } else {
      sendMotorValue(0, 0);
      sendMotorValue(1, 0);
    }
  }
}

/*
  @func void loopManuel Fonction qui gère le mode manuel du robot
  Elle utilise les infos transmises par le joystick de la manette pour les transmettre ensuite aux roues.
  @param null
  @return void
*/
void loopManuel() {
  if (onStartState()) { //Seulement la première fois qu'il rentre dans la loop
    sendTracking(0x2E);
    if (memOutput != 10) {
      controllerOutput = 10 ;
    } else {
      controllerOutput = memOutput ;
    }
  }

  //sendEyes(5, (map(AxisLX(), 0, 255, 0, 6) << 3) | map(AxisLY(), 0, 255, 0, 6));

  if (checkPause()) { // quitte directement la loop si la pause est pressée et évite que le "state" puisse être changé dans la fonction
    return;
  }

  loopHurt();

  if (isDead()) {
    setState(State::MenuSelection);
    sendMotorValue(0, 0);
    sendMotorValue(1, 0);
    setupRobot();
    return;
  }

  //Récupère les infos
  int jX = (joystickValue(AxisLX()) * 100);
  int jY =  (joystickValue(AxisLY()) * 100);

  //Viteese des roues (De base à l'arrêt)
  short speedL = 0;
  short speedR = 0;

  // Sens du moteur
  bool forward = true;

  if (jY > 10) { // ajout volontaire de "deadzone" pour rendre les transitions entre avant et arrière plus douces.
    if (jX < -30) {
      speedR = 100 + jX;
      speedL = -jX;
    } else if (jX > 30) {
      speedR = jX;
      speedL = 100 - jX;
    } else {
      speedR = jY;
      speedL = jY;
    }
  } else if (jY < -10) {
    if (jX < -30) {
      speedR = -1 * ((100 + jX));
      speedL = jX;
    } else if (jX > 30) {
      speedR = -jX;
      speedL = -100 + jX;
    } else {
      speedR = jY;
      speedL = jY;
    }
  } else {
    if (jX < -10) {
      speedR = -1 * ((100 + jX));
      speedL = jX;
    } else if (jX > 10) {
      speedR = -jX;
      speedL = -100 + jX;
    } else {
      speedR = 0;
      speedL = 0;
    }
  }

  // bloque les vitesses sur vitesseMAX pour éviter une vitesse trop grande
  short vitesseMax =  getDifficulty(MAX_SPEED);
  if (speedL > vitesseMax) {
    speedL = vitesseMax;
  }
  if (speedR > vitesseMax) {
    speedR = vitesseMax;
  }
  if (speedL < -vitesseMax) {
    speedL = -vitesseMax;
  }
  if (speedR < -vitesseMax) {
    speedR = -vitesseMax;
  }

  //Envoie les infos au moteur
  sendMotorValue(0, speedL);
  sendMotorValue(1, speedR);
}

/*
  @func bool sendSound envoie l'information demandée au module de son
  @param int id du module demandé
  #param int data
  @return bool
*/
bool sendSound(int id, int data) {
  if (waitingResponse || !stateModules[2])return false;
  if (!stateModules[2])return false;
  nbTransmission++;
  Wire.beginTransmission(ADDR_SOUND);
  Wire.write(id);
  if (data > -1) {
    Wire.write(data);
  }
  return Wire.endTransmission() == 1;

}

/*
  @func bool sendContact envoie l'information demandée au module des contacts
  @param int id du module demandé
  #param int data
  #param int duration
  @return bool
*/
bool sendContact(int id, int data, int duration) {
  if (waitingResponse || !stateModules[1])return false;
  if (!stateModules[1])return false;
  nbTransmission++;
  Wire.beginTransmission(ADDR_CONTACT);
  Wire.write(id);
  if (data > -1) {
    Wire.write(data);
  }
  if (duration > -1) {
    Wire.write(duration);
  }
  return Wire.endTransmission() == 1;

}

/*
  @func bool sendTracking envoie l'information demandée au module de tracking
  #param int id du module demandé
  @return bool
*/
bool sendTracking(int id) {
  if (waitingResponse || !stateModules[0])return false;
  if (!stateModules[0])return false;
  nbTransmission++;
  Wire.beginTransmission(ADDR_TRACKING);
  Wire.write(id);
  return Wire.endTransmission() == 1;
}

/*
  @func bool sendEyes envoie l'information demandée au module des yeux
  @param int id du module demandé
  #param int data
  @return bool
*/
bool sendEyes(int id, int data) {
  if (waitingResponse || !stateModules[4])return false;
  if (!stateModules[4])return false;
  nbTransmission++;
  Wire.beginTransmission(ADDR_EYES);
  Wire.write(id);
  if (data > -1) {
    Wire.write(data);
  }
  return Wire.endTransmission() == 1;
}

/*
  @func bool sendMotorValue envoie l'information demandée au module des roues
  @param int id du module demandé
  #param int value
  @return bool
*/
bool sendMotorValue(byte id, int value) {
  if (waitingResponse || !stateModules[3])return false;
  if (currentMotorValue[id] == value)return true; //évite de faire une comm si rien n'a changé
  currentMotorValue[id] = value;
  byte data = abs(value);
  if ( data > 100)data = 100;
  if ((value < 0)) {
    bitSet(data, 7);
  }
  nbTransmission++;
  Wire.beginTransmission(ADDR_WHEEL);
  Wire.write(id);
  Wire.write(data);
  return Wire.endTransmission() == 1;
}

/*
   @func void communicationController Gère la communication avec l'esp32 du groupe manette installé sur le pcb.
   On commence par réenvoyer les données que l'on possède ce qui fait que l'esp32 nous envoieles siennes directement.
   @param null
   @return void
*/
void communicationController() {
  byte dataBufferWrite[2] = {controllerOutput, (byte)((controllerBuzzer << 4) | controllerVibrator)}; // réenvoie les données à la manette
  Serial.write(dataBufferWrite, 2);
  // controlle la longueure de la tramme
  while (Serial.available() < BUFFER_SIZE) {}
  Serial.readBytes(dataBuffer, BUFFER_SIZE); //lit les infos en provenance de la manette

  if (InfoController() != CONNECTED) {
    setState(State::Disconnected);
  }
}

/*
   @func State setState  Elle change l'état actuelle de la variable state et retourne son état actuel.
   Permet de faire d'autres actions sur des variables lors d'un changement d'état directement dans cette fonction si nécessaire.
   Evite de passer par une variable grobale.
   @param State state
   #param int menuPos
   @return State currentState
*/
State setState(State state, int menuPos) {
  if (menuPos > -1) {
    stateMenuPos = menuPos;
  }
  if (currentState == state)return currentState; // Evite de traiter inutilement les données s'il n'y a pas de changement
  isStartedState = false;
  previousState = currentState;
  currentState = state;
  return currentState;
}

/*
  @func bool onStartState vérifie si l'on execute pour la première fois une action
  @param null
  @return bool
*/
bool onStartState() {
  if (!isStartedState) {
    isStartedState = true;
    return true;
  }
  return false;
}

/*
   @func bool checkPause Vérifie s'il y a une demande de pause lors de la partie
   @param null
   @return bool
*/
bool checkPause() {
  if (ButtonB(true)) {
    sendMotorValue(0, 0);
    sendMotorValue(1, 0);
    setState(State::PauseGenerale, 0);
    return true;
  }
  return false;
}

/*
   @func void loopMenuGo Gère le menuGo (affichage et pression des boutons)
   @param null
   @return void
*/
void  loopMenuGo() {
  if (onStartState()) {
  }
  controllerOutput = 3;
  if (ButtonA(true)) {
    setState(savedMode);
  }
}

/*
   @param changeCursorPosition Change la position du curseur dans les menus à l'aide des touches NORTH et SOUTH
   @param byte sizeMenu taille du menu en comptant à partir de 1
   @return byte stateMenuPos position du curseur
*/
byte changeCursorPosition(byte sizeMenu) {
  sizeMenu--; // permet de donner taille menu en comptant à partir de 1
  if (ButtonNORTH(true)) {
    stateMenuPos++;
    if (stateMenuPos > sizeMenu) {
      stateMenuPos = 0;
    }
  }

  if (ButtonSOUTH(true)) {
    stateMenuPos--;
    if (stateMenuPos < 0) {
      stateMenuPos = sizeMenu;
    }
  }
  return stateMenuPos;
}

/*
   @func void loopPauseGenerale Gère le mode pauseGenerale (affichage et pression des boutons)
   @param null
   @return void
*/
void loopPauseGenerale() {
  if (onStartState()) {//Seulement la première fois qu'il rentre dans la loop
    sendSound(0, 3);//Musique Pause
    sendEyes(4);// Yeux Pause
  }

  byte tailleMenu = 2; // compte à partir de 1 donc ici le menu fait 2
  byte pos = changeCursorPosition(tailleMenu);//changement position curseur

  switch (pos) {
    case 0:
      controllerOutput = 28; // Info d'affichage pour la manette
      if (ButtonA(true)) {
        resumeGame();
        setState(State::MenuGO);
      }
      break;
    case 1:
      controllerOutput = 29; // Info d'affichage pour la manette
      if (ButtonA(true)) {
        resumeGame();
        setState(State::MenuSelection);
        setupRobot();
      }
      break;
  }
}

/*
   @func void loopMenuSelection Gère le mode MenuSelection (affichage et pression des boutons)
   @param null
   @return void
*/
void  loopMenuSelection() {
  if (onStartState()) {
  }
  byte tailleMenu = 2;
  byte pos = changeCursorPosition(tailleMenu);//changement position curseur
  State selectedMode = State::Automatique;

  switch (pos) {
    default:
    case 0:
      controllerOutput = 1;
      selectedMode = State::Automatique;
      break;
    case 1:
      controllerOutput = 2;
      selectedMode = State::Manuel;
      break;
  }

  if (ButtonA(true)) {
    savedMode = selectedMode; // Lors du choix d'un mode on le stock pour que le menu pause puisse reprendre sur le bon mode
    setState(State::MenuGO);
  }
}

/*
  @func void loopDifficulty Menu de choix de la difficulté
  @param null
  @return void
*/
void loopDifficulty() {
  if (onStartState()) {
  }
  byte tailleMenu = 3;
  byte pos = changeCursorPosition(tailleMenu);//changement position curseur
  int diffTemp = -1;

  switch (pos) {
    default:
    case 0:
      controllerOutput = 1; // nbr arbitraire car non implémenté par le groupe manette...
      diffTemp = EASY;
      break;
    case 1:
      controllerOutput = 2;
      diffTemp = MEDIUM;
      break;
    case 3:
      controllerOutput = 2;
      diffTemp = HARD;
      break;
  }

  if (ButtonA(true)) {
    if (diffTemp > -1)
      currentDifficulty = diffTemp;
    setState(State::MenuSelection);
  }
  if (ButtonB(true)) {//RETOUR
    setState(State::MenuSelection);
  }
}

/*
  @func void logs Print dans la console les infos ci-dessous (utilisé pour le debug)
  @param null
  @return void
*/
void logs() {
  if (millis() < lastLogAt + LOGS_DELAY) {
    return;
  }
  lastLogAt = millis();
  /* Serial.println("======LOGS======");
    Serial.println("Execute Time: " + String(millis() / 1000.0) + "s");
    Serial.println("nbTransmission: " + String(nbTransmission));
    Serial.println("nbRequest: " + String(nbRequest));
    Serial.println("loopTime: " + String(loopTime) + "ms");
    Serial.println("currentState: " + String(currentState));
    Serial.println("headAngle: " + String(headAngle));
    Serial.println("targetDistance: " + String(targetDistance));
    Serial.println("isFindTarget: " + String(isFindTarget));
    Serial.println("currentDifficulty: " + String(currentDifficulty));
    Serial.println("currentMotorValue 1: " + String(currentMotorValue[0]) + "%");
    Serial.println("currentMotorValue 2: " + String(currentMotorValue[1]) + "%");
    Serial.println("Module Tracking (" + toHex(modules[0]) + "): " + String((stateModules[0] ? "CONNECTED" : "DISCONNECTED")));
    Serial.println("Module Contact (" + toHex(modules[1]) + "): " + String((stateModules[1] ? "CONNECTED" : "DISCONNECTED")));
    Serial.println("Module Sound (" + toHex(modules[2]) + "): " + String((stateModules[2] ? "CONNECTED" : "DISCONNECTED")));
    Serial.println("Module Wheel (" + toHex(modules[3]) + "): " + String((stateModules[3] ? "CONNECTED" : "DISCONNECTED")));
    Serial.println("Module Eyes (" + toHex(modules[4]) + "): " + String((stateModules[4] ? "CONNECTED" : "DISCONNECTED")));
    Serial.println("================");
    Serial.println();
  */
}

/*
  @func float joystickValue calcule la valeure du joystick en prenant en compte la marge
  @param byte v
  @return float tmp
*/
float joystickValue(byte v) {
  float tmp = mapfloat(v, 0, 255, 1, -1);
  if (tmp >= -JOYSTICK_MARGIN && tmp <= JOYSTICK_MARGIN)tmp = 0;
  return tmp;
}

/*
  @func float triggerValue transforme les infos de joystick entre 0 et 1 (non utilisée)
  @param byte v
  @return float tmp
*/
float triggerValue(byte v) {
  float tmp = mapfloat(v, 0, 255, 0, 1);
  return tmp;
}

/*
   @func byte InfoController retourne la valeure de l'etat de la mannette
   @param null
   @return byte
*/
byte InfoController() {
  return dataBuffer[8];
}

/*
   @func byte AxisLX retourne la valeure de l'axe X du joystick gauche
   @param null
   @return byte
*/
byte AxisLX() {
  return dataBuffer[4];
}

/*
   @func byte AxisLY retourne la valeure de l'axe Y du joystick gauche
   @param null
   @return byte
*/
byte AxisLY() {
  return dataBuffer[5];
}

/*
   @func byte AxisRX retourne la valeure de l'axe X du joystick droite
   @param null
   @return byte
*/
byte AxisRX() {
  return dataBuffer[6];
}

/*
   @func byte AxisRY retourne la valeure de l'axe Y du joystick droite
   @param null
   @return byte
*/
byte AxisRY() {
  return dataBuffer[7];
}

/*
   @func byte AxisLT retourne la valeure de la gachette droite
   @param null
   @return byte
*/
byte AxisLT() {
  return dataBuffer[2];
}

/*
   @func byte AxisRT retourne la valeure de la gachette Gauche
   @param null
   @return byte
*/
byte AxisRT() {
  return dataBuffer[3];
}

/*
   @func bool ButtonA retourne la valeure du boutton A
   @param null
   @return bool
*/
bool ButtonA(bool flanc) {
  bool v = bitRead(dataBuffer[0], 7);
  if (flanc)v = ButtonFlanc(v, 0);
  return v;
}

/*
   @func bool ButtonB retourne la valeure du boutton B
   @param null
   @return bool
*/
bool ButtonB(bool flanc) {
  bool v = bitRead(dataBuffer[0], 4);
  if (flanc)v = ButtonFlanc(v, 1);
  return v;
}

/*
   @func bool ButtonX retourne la valeure du boutton X
   @param null
   @return bool
*/
bool ButtonX(bool flanc) {
  bool v = bitRead(dataBuffer[0], 6);
  if (flanc)v = ButtonFlanc(v, 2);
  return v;
}

/*
   @func bool ButtonY retourne la valeure du boutton Y
   @param null
   @return bool
*/
bool ButtonY(bool flanc) {
  bool v = bitRead(dataBuffer[0], 5);
  if (flanc)v = ButtonFlanc(v, 3);
  return v;
}

/*
   @func bool ButtonWEST retourne la valeure du boutton WEST
   @param null
   @return bool
*/
bool ButtonWEST(bool flanc) {
  bool v = bitRead(dataBuffer[1], 4);
  if (flanc)v = ButtonFlanc(v, 4);
  return v;
}

/*
   @func bool ButtonEAST retourne la valeure du boutton EAST
   @param null
   @return bool
*/
bool ButtonEAST(bool flanc) {
  bool v = bitRead(dataBuffer[1], 6);
  if (flanc)v = ButtonFlanc(v, 5);
  return v;
}

/*
   @func bool ButtonNORTH retourne la valeure du boutton NORTH
   @param null
   @return bool
*/
bool ButtonNORTH(bool flanc) {
  bool v = bitRead(dataBuffer[1], 5);
  if (flanc)v = ButtonFlanc(v, 6);
  return v;
}

/*
   @func bool ButtonSOUTH retourne la valeure du boutton SOUTH
   @param null
   @return bool
*/
bool ButtonSOUTH(bool flanc) {
  bool v = bitRead(dataBuffer[1], 7);
  if (flanc)v = ButtonFlanc(v, 7);
  return v;
}

/*
   @func bool ButtonLB retourne la valeure du boutton LB
   @param null
   @return bool
*/
bool ButtonLB(bool flanc) {
  bool v = bitRead(dataBuffer[0], 2);
  if (flanc)v = ButtonFlanc(v, 8);
  return v;
}

/*
   @func bool ButtonRB retourne la valeure du boutton RB
   @param null
   @return bool
*/
bool ButtonRB(bool flanc) {
  bool v = bitRead(dataBuffer[0], 3);
  if (flanc)v = ButtonFlanc(v, 9);
  return v;
}

/*
   @func bool ButtonLS retourne la valeure du boutton LS
   @param null
   @return bool
*/
bool ButtonLS(bool flanc) {
  bool v = bitRead(dataBuffer[0], 4);
  if (flanc)v = ButtonFlanc(v, 10);
  return v;
}

/*
   @func bool ButtonRS retourne la valeure du boutton RS
   @param null
   @return bool
*/
bool ButtonRS(bool flanc) {
  bool v = bitRead(dataBuffer[0], 3);
  if (flanc)v = ButtonFlanc(v, 11);
  return v;
}

/*
   @func bool ButtonSTART retourne la valeure du boutton START
   @param null
   @return bool
*/
bool ButtonSTART(bool flanc) {
  bool v = bitRead(dataBuffer[0], 1);
  if (flanc)v = ButtonFlanc(v, 12);
  return v;
}

/*
   @func bool ButtonSELECT retourne la valeure du boutton SELECT
   @param null
   @return bool
*/
bool ButtonSELECT(bool flanc) {
  bool v = bitRead(dataBuffer[0], 0);
  if (flanc)v = ButtonFlanc(v, 13);
  return v;
}

/*
   @func bool ButtonFlanc Permet de détecter les flancs montants des boutons dans une seule fonction
   @param bool button
   #param byte flancId 0: null; 1: A; 2: B, 3: NORTH, 4: SOUTH
   @return bool
*/
bool ButtonFlanc(bool button, byte flancId) {
  bool temp = false;
  if (button && !flancsMontants[flancId]) {
    temp = true;
  }
  flancsMontants[flancId] = button;
  return temp;
}

/*
  @func float mapfloat Comme son nom l'indique c'est une fonction map mais qui comprends les floats
  @param float x 
  #param float in_min
  #param float in_max
  #param float out_min
  #param float out_max
  @return float la valeure mapée en float
*/
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*
  @func String toHex donne l'adresse du module en hexadécimal
  @param int num id en décimal du module
  @return string buffer valeure en hexa de l'adresse du module
*/
String toHex(int num) {
  char buffer[10];
  sprintf(buffer, "%x", num);
  return "0x" + String(buffer);
}
