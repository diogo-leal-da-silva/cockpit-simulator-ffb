#include <Joystick.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//Criação do objeto do display LCD I2C:
//Se no teu teste funcionou com 0x3F, troque 0x27 por 0x3F
LiquidCrystal_I2C lcd(0x27, 20, 4);

//Criação do objeto Joystick HID --> define o que terá o controle:
Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  21,    // Buttons (0..23)
  0,     // Hat switches
  false, // X
  false, // Y
  false, // Z
  true,  // Rx --> Freio de mão
  false, // Ry --> Freio
  false, // Rz --> Embreagem
  false, // Rudder
  false, // Throttle
  false  // Accelerator --> Acelerador
);

//Declaração LCD:
const int VEL_MAX = 999;
const int RPM_MAX = 100;
const int BARRAS_MAX = 10;

//Declaração dos tempos de atualização do LCD:
const unsigned long TEMPO_ATUALIZAR_LCD = 250;

//Declaração dos estados dos botões:
int estadoAntes[21] = {
  0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0
};

//Declarção da variável da marcha do display 7 segmentos:
int marcha = 0;

//Declaração das variáveis recebidas do SimHub:
int velocidade = 0;
int rpm = 0;
int voltaAtual = 0;
int voltaTotal = 0;

String melhorVoltaCorrida = "--:--.---";
String minhaMelhorVolta = "--:--.---";

//Declaração das variáveis de controle do LCD:
unsigned long tempoAtualizarLCD = 0;

//Declaração das variáveis anteriores do LCD:
int rpmLCDAnterior = -1;
String velocidadeVoltaLCDAnterior = "";
String melhorVoltaCorridaLCDAnterior = "";
String minhaMelhorVoltaLCDAnterior = "";

//Declaração dos pinos analógicos:
int freioMao = A5;
int sigMUX = 13;

//Declaração dos pinos digitais:
//No Leonardo, D2 = SDA e D3 = SCL do I2C
int botoesARD[5] = {A3, A4, 4, 5, 6};
int display7_CLOCK = 7;
int display7_RESET = 8;
int binMUX[4] = {9, 10, 11, 12};

//Declaração de variáveis para o multiplexador:
bool botoesMUX[16];

//Declaração de valores binários do multiplexador:
int conversorMUX[16][4] = {
  {0,0,0,0},
  {1,0,0,0},
  {0,1,0,0},
  {1,1,0,0},
  {0,0,1,0},
  {1,0,1,0},
  {0,1,1,0},
  {1,1,1,0},
  {0,0,0,1},
  {1,0,0,1},
  {0,1,0,1},
  {1,1,0,1},
  {0,0,1,1},
  {1,0,1,1},
  {0,1,1,1},
  {1,1,1,1}
};

//Função para iniciar o LCD:
void iniciarLCD(){
  Wire.begin();
  delay(50);
  lcd.begin(20, 4);
  lcd.backlight();
  lcd.clear();
  delay(50);
}

//Função para imprimir uma linha completa no LCD:
void imprimirLinhaLCD(int linha, String texto){
  int i = 0;

  if(texto.length() > 20){
    texto = texto.substring(0, 20);
  }

  lcd.setCursor(0, linha);
  lcd.print(texto);

  for(i=texto.length();i<20;i++){
    lcd.print(" ");
  }
}

//Função para reiniciar os valores anteriores do LCD:
void reiniciarValoresLCD(){
  rpmLCDAnterior = -1;
  velocidadeVoltaLCDAnterior = "";
  melhorVoltaCorridaLCDAnterior = "";
  minhaMelhorVoltaLCDAnterior = "";
}

//Função para preparar a tela do LCD:
void prepararLCD(){
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("R: ");

  reiniciarValoresLCD();
}

//Função para leitura do multiplexador:
void lerMUX(){
  int i = 0;
  int j = 0;

  for(i=0;i<16;i++){
    for(j=0;j<4;j++){
      digitalWrite(binMUX[j], conversorMUX[i][j]);
    }
    delayMicroseconds(5);
    botoesMUX[i] = digitalRead(sigMUX);
  }

  for(int i = 0; i < 16; i++) {
    bool apertado;

    if(i < 7) {
      apertado = botoesMUX[i];
    }
    else {
      apertado = !botoesMUX[i];
    }
    Joystick.setButton(i, apertado);
    }
  }

//Função marchas:
int marchas(){
  int i = 0;

  for(i=0;i<7;i++){
    if(botoesMUX[i] == HIGH){
      return i + 1;
    }
  }
  return 0;
}

//Função display 7 segmentos:
void display7(int marchaNova){
  int i = 0;

  if(marchaNova == marcha){
    return;
  }

  digitalWrite(display7_RESET, LOW);
  delayMicroseconds(5);
  digitalWrite(display7_RESET, HIGH);
  delayMicroseconds(5);
  digitalWrite(display7_RESET, LOW);
  delayMicroseconds(5);
  if(marchaNova == 0){
    marcha = 0;
    return;
  }

  for(i=0;i<marchaNova;i++){
    digitalWrite(display7_CLOCK, HIGH);
    delayMicroseconds(5);
    digitalWrite(display7_CLOCK, LOW);
    delayMicroseconds(5);
  }

  marcha = marchaNova;
  return;
}

//Função para leitura dos botões:
void botoes(){

  int i = 0;

  for(i=0;i<16;i++){
    int estadoAgora = !botoesMUX[i];

    if(estadoAgora != estadoAntes[i]){
      Joystick.setButton(i, estadoAgora);
      estadoAntes[i] = estadoAgora;
    }
  }

  for(i=0;i<5;i++){
    int estadoAgora = !digitalRead(botoesARD[i]);

    if(estadoAgora != estadoAntes[i+16]){
      Joystick.setButton(i+16, estadoAgora);
      estadoAntes[i+16] = estadoAgora;
    }
  }
  delay(50);

}

//Função para atualizar o freio de mão:
void atualizarFreioDeMao() {
  Joystick.setRxAxis(analogRead(freioMao));
}

//Função para botões de pulso:
void botaoPulso(int botao) {
  static int estadoAnteriorPulso[21];
  static bool iniciado[21] = {false};

  int estadoAtual = HIGH;

  if (botao < 0 || botao > 20) 
    return;
  if (botao < 16) {
    if (botao < 7) {
      estadoAtual = botoesMUX[botao];
    }
    else {
      estadoAtual = !botoesMUX[botao];
    }
  }
  else {
    estadoAtual = !digitalRead(botoesARD[botao - 16]);
  }
  if (!iniciado[botao]) {
    estadoAnteriorPulso[botao] = estadoAtual;
    estadoAntes[botao] = estadoAtual;
    iniciado[botao] = true;

    return;
  }
  estadoAntes[botao] = estadoAtual;
  //Dá pulso quando ativa e desativa:
  if (estadoAtual != estadoAnteriorPulso[botao]) {
    Joystick.setButton(botao, HIGH);
    delay(80);
    Joystick.setButton(botao, LOW);
  }
  estadoAnteriorPulso[botao] = estadoAtual;
}

//Função para receber os dados do SimHub:
//Formato esperado:
//velocidade;rpm;voltaAtual;voltaTotal;melhorVoltaCorrida;minhaMelhorVolta\n
void telemetria(){
  if(Serial.available()){
    String dadoVelocidade = Serial.readStringUntil(';');
    String dadoRPM = Serial.readStringUntil(';');
    String dadoVoltaAtual = Serial.readStringUntil(';');
    String dadoVoltaTotal = Serial.readStringUntil(';');
    String dadoMelhorVoltaCorrida = Serial.readStringUntil(';');
    String dadoMinhaMelhorVolta = Serial.readStringUntil('\n');

    velocidade = dadoVelocidade.toInt();
    rpm = dadoRPM.toInt();
    voltaAtual = dadoVoltaAtual.toInt();
    voltaTotal = dadoVoltaTotal.toInt();

    dadoMelhorVoltaCorrida.trim();
    dadoMinhaMelhorVolta.trim();

    if(dadoMelhorVoltaCorrida.length() > 0){
      melhorVoltaCorrida = dadoMelhorVoltaCorrida;
    }
    if(dadoMinhaMelhorVolta.length() > 0){
      minhaMelhorVolta = dadoMinhaMelhorVolta;
    }

    velocidade = constrain(velocidade, 0, VEL_MAX);
    rpm = constrain(rpm, 0, RPM_MAX);
  }
}

//Função para transformar RPM em barras:
int barrasRPM(){
  int barras = map(rpm, 0, RPM_MAX, 0, BARRAS_MAX);

  barras = constrain(barras, 0, BARRAS_MAX);

  return barras;
}

//Função para desenhar a linha do RPM:
//Função para desenhar a linha do RPM:
void lcdRPM(){
  int i = 0;
  int barras = barrasRPM();
  int barrasLCD = map(barras, 0, BARRAS_MAX, 0, 15);

  barrasLCD = constrain(barrasLCD, 0, 15);

  if(barrasLCD == rpmLCDAnterior){
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("RPM: ");

  lcd.setCursor(5, 0);

  for(i=0;i<15;i++){
    if(i < barrasLCD){
      lcd.write(byte(255));
    }
    else{
      lcd.print(" ");
    }
  }

  rpmLCDAnterior = barrasLCD;
}

//Função para mostrar a velocidade e as voltas na segunda linha:
void lcdVelocidadeVolta(){
  String texto = "";

  texto += "VEL:";
  texto += velocidade;
  texto += "km/h LAP:";
  texto += voltaAtual;
  texto += "/";
  texto += voltaTotal;

  if(texto.length() > 20){
    texto = "";
    texto += "VEL:";
    texto += velocidade;
    texto += " LAP:";
    texto += voltaAtual;
    texto += "/";
    texto += voltaTotal;
  }

  if(texto == velocidadeVoltaLCDAnterior){
    return;
  }

  imprimirLinhaLCD(1, texto);

  velocidadeVoltaLCDAnterior = texto;
}

//Função para mostrar a melhor volta da corrida na terceira linha:
void lcdMelhorVoltaCorrida(){
  String texto = "";

  texto += "BEST COR: ";
  texto += melhorVoltaCorrida;

  if(texto == melhorVoltaCorridaLCDAnterior){
    return;
  }

  imprimirLinhaLCD(2, texto);

  melhorVoltaCorridaLCDAnterior = texto;
}

//Função para mostrar a minha melhor volta na quarta linha:
void lcdMinhaMelhorVolta(){
  String texto = "";

  texto += "MEU BEST: ";
  texto += minhaMelhorVolta;

  if(texto == minhaMelhorVoltaLCDAnterior){
    return;
  }

  imprimirLinhaLCD(3, texto);

  minhaMelhorVoltaLCDAnterior = texto;
}

//Função geral do display LCD:
void displayLCD(){
  unsigned long agora = millis();

  if(agora - tempoAtualizarLCD < TEMPO_ATUALIZAR_LCD){
    return;
  }

  tempoAtualizarLCD = agora;

  lcdRPM();
  lcdVelocidadeVolta();
  lcdMelhorVoltaCorrida();
  lcdMinhaMelhorVolta();
}

void setup() {
  //Setar comunicação com simhub:
  Serial.begin(115200);
  Serial.setTimeout(20);
  //Setar LCD I2C:
  iniciarLCD();
  //Setar os botões digitais:
  for(int i=0;i<5;i++){
    pinMode(botoesARD[i], INPUT_PULLUP);
  }
  //Setar saídas digitais para os displays:
  pinMode(display7_CLOCK, OUTPUT);
  pinMode(display7_RESET, OUTPUT);
  //Setar entradas multiplexador:
  pinMode(sigMUX, INPUT_PULLUP);
  for(int i=0;i<4;i++){
    pinMode(binMUX[i], OUTPUT);
    digitalWrite(binMUX[i], LOW);
  }
  //Setar entradas analógicas:
  pinMode(freioMao, INPUT);
  //Estado inicial do display 7 segmentos:
  digitalWrite(display7_CLOCK, LOW);
  digitalWrite(display7_RESET, LOW);
  delay(10);
  digitalWrite(display7_RESET, HIGH);
  delay(10);
  digitalWrite(display7_RESET, LOW);
  //Inicialização da biblioteca Joystick:
  Joystick.begin(true);
    //Faixa freio de mão:
  Joystick.setRxAxisRange(24, 680);
  //Tela inicial do LCD:
  tempoAtualizarLCD = 0;
  prepararLCD();
  displayLCD();
}

void loop() {
  lerMUX();

  int marchaAtual = marchas();

  display7(marchaAtual);

  botoes();

 // botaoPulso();
  //botaoPulso(10);
  //botaoPulso(11);
  //botaoPulso(12);
  //botaoPulso(17);

  atualizarFreioDeMao();
  telemetria();
  displayLCD();

  delay(10);
}