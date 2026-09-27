// ================= PIN =================
#define X_DIR 2
#define X_STEP 3
#define Y_DIR 4
#define Y_STEP 5
#define Z_DIR 6
#define Z_STEP 7

// ================= SETTINGS =================
const int stepDelayUsX = 650;
const int stepDelayUsY = 400;
const int stepDelayUsZ = 400;

// ================= LIMIT =================
const long X_MIN = -10000;
const long X_MAX = 10000;
const long Y_MIN = -20000;
const long Y_MAX = 20000;
const long Z_MIN = -8000;
const long Z_MAX = 8000;

// ================= STATE =================
long posX=0,posY=0,posZ=0;
long targetX=0,targetY=0,targetZ=0;

bool xRun=false,yRun=false,zRun=false;
int dirX=0,dirY=0,dirZ=0;

bool systemReady=false;
String mode="MANUAL";

int autoStep=1;
bool isMoving=false;

// ================= TIMER =================
unsigned long lastSend=0;
const int sendInterval=200;

// ===================================================
void setup(){
  pinMode(X_DIR,OUTPUT); pinMode(X_STEP,OUTPUT);
  pinMode(Y_DIR,OUTPUT); pinMode(Y_STEP,OUTPUT);
  pinMode(Z_DIR,OUTPUT); pinMode(Z_STEP,OUTPUT);

  Serial.begin(9600);
  Serial.println("START"); // handshake
}

// ===================================================
void loop(){

  checkSerial();

  if(!systemReady) return;

  // ===== MANUAL =====
  if(mode=="MANUAL"){
    manualMove();
  }

  // ===== AUTO =====
  if(mode=="AUTO"){
    // รอ trigger BOX
  }

  // ===== SEND DATA =====
  if(millis()-lastSend>sendInterval){
    lastSend=millis();
    sendData();
  }
}

// ================= SERIAL =================
void checkSerial(){
  if(Serial.available()){
    String cmd=Serial.readStringUntil('\n');
    cmd.trim();

    // ===== INIT =====
    if(cmd.startsWith("INIT")){
      char m[10];
      int x,y,z;
      sscanf(cmd.c_str(),"INIT,%[^,],%d,%d,%d",m,&x,&y,&z);

      mode=String(m);
      posX=x; posY=y; posZ=z;
      targetX=x; targetY=y; targetZ=z;

      systemReady=true;
      Serial.println("READY");
    }

    if(!systemReady) return;

    // ===== MODE =====
    if(cmd=="AUTO") mode="AUTO";
    else if(cmd=="MANUAL") mode="MANUAL";

    // ===== AUTO TRIGGER =====
    if(cmd=="BOX" && mode=="AUTO"){
      runAuto();
    }

    // ===== MANUAL CONTROL =====
    if(mode=="MANUAL"){
      if(cmd=="X+") dirX=1;
      else if(cmd=="X-") dirX=-1;

      else if(cmd=="Y+") dirY=1;
      else if(cmd=="Y-") dirY=-1;

      else if(cmd=="Z+") dirZ=1;
      else if(cmd=="Z-") dirZ=-1;

      else if(cmd=="STOP"){
        dirX=0; dirY=0; dirZ=0;
      }
    }
  }
}

// ================= MANUAL MOVE =================
void manualMove(){
  moveAxisContinuous(1);
  moveAxisContinuous(2);
  moveAxisContinuous(3);
}

// ================= CONTINUOUS MOVE =================
void moveAxisContinuous(int axis){
  long *pos;
  int dirPin,stepPin,delayUs;
  long minL,maxL;
  int *dir;
  bool *run;

  if(axis==1){
    pos=&posX; dirPin=X_DIR; stepPin=X_STEP;
    delayUs=stepDelayUsX; minL=X_MIN; maxL=X_MAX;
    dir=&dirX; run=&xRun;
  }
  else if(axis==2){
    pos=&posY; dirPin=Y_DIR; stepPin=Y_STEP;
    delayUs=stepDelayUsY; minL=Y_MIN; maxL=Y_MAX;
    dir=&dirY; run=&yRun;
  }
  else{
    pos=&posZ; dirPin=Z_DIR; stepPin=Z_STEP;
    delayUs=stepDelayUsZ; minL=Z_MIN; maxL=Z_MAX;
    dir=&dirZ; run=&zRun;
  }

  if(*dir!=0){
    *run=true;

    digitalWrite(dirPin, (*dir>0));

    digitalWrite(stepPin,HIGH);
    delayMicroseconds(delayUs);
    digitalWrite(stepPin,LOW);
    delayMicroseconds(delayUs);

    *pos += *dir;

    if(*pos<=minL || *pos>=maxL){
      *dir=0;
      *run=false;
    }
  }else{
    *run=false;
  }
}

// ================= AUTO =================
void runAuto(){
  if(isMoving) return;
  isMoving=true;

  // กลับ Home
  setTarget(0,0,0);
  waitStop();

  // Z cycle
  zCycle();

  // ===== ไปตำแหน่ง + แจ้ง Node-RED =====
  if(autoStep==1){
    setTarget(9000,-17200,0);
    waitStop();
    Serial.println("ARRIVED,P1");
    delay(1000);
  }
  else if(autoStep==2){
    setTarget(5700,-17200,0);
    waitStop();
    Serial.println("ARRIVED,P2");
    delay(1000);
  }
  else if(autoStep==3){
    setTarget(9000,-4000,0);
    waitStop();
    Serial.println("ARRIVED,P3");
    delay(1000);
  }
  else if(autoStep==4){
    setTarget(5700,-4000,0);
    waitStop();
    Serial.println("ARRIVED,P4");
    delay(1000);
  }

  // กลับ Home
  setTarget(0,0,0);
  waitStop();

  autoStep++;
  if(autoStep>4) autoStep=1;

  isMoving=false;
}

// ================= TARGET =================
void setTarget(long x,long y,long z){
  targetX=x; targetY=y; targetZ=z;
}

void waitStop(){
  while(posX!=targetX || posY!=targetY || posZ!=targetZ){
    stepToTarget(1);
    stepToTarget(2);
    stepToTarget(3);
    sendData();
  }
}

void stepToTarget(int axis){
  long *pos,*target;
  int dirPin,stepPin,delayUs;
  bool *run;

  if(axis==1){
    pos=&posX; target=&targetX;
    dirPin=X_DIR; stepPin=X_STEP;
    delayUs=stepDelayUsX; run=&xRun;
  }
  else if(axis==2){
    pos=&posY; target=&targetY;
    dirPin=Y_DIR; stepPin=Y_STEP;
    delayUs=stepDelayUsY; run=&yRun;
  }
  else{
    pos=&posZ; target=&targetZ;
    dirPin=Z_DIR; stepPin=Z_STEP;
    delayUs=stepDelayUsZ; run=&zRun;
  }

  if(*pos!=*target){
    *run=true;
    digitalWrite(dirPin,(*target>*pos));
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(delayUs);
    digitalWrite(stepPin,LOW);
    delayMicroseconds(delayUs);

    if(*target>*pos)(*pos)++;
    else (*pos)--;
  }else{
    *run=false;
  }
}

// ================= Z =================
void zCycle(){
  setTarget(posX,posY,6600);
  waitStop();
  delay(1000);
  setTarget(posX,posY,0);
  waitStop();
}

// ================= SEND =================
void sendData(){
  String data = mode + "," +
                String(xRun)+","+String(posX)+","+
                String(yRun)+","+String(posY)+","+
                String(zRun)+","+String(posZ);

  Serial.println(data);
}