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
const int GASOLINA_MAX = 100;
const int SONO_MAX = 100;
const int BARRAS_MAX = 10;

//Declaração dos tempos de troca das telas do LCD:
const unsigned long TEMPO_TELA_PRINCIPAL = 5000;
const unsigned long TEMPO_TELA_ENTREGA = 3000;
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
int gasolina = 0;
int sono = 0;

String marchaTexto = "N";
String dataAtual = "--";
String horaAtual = "--:--";
String dataEntrega = "";
String horaEntrega = "--";
String tempoAteEntrega = "--";
String distanciaRestante = "--";

//Declaração das variáveis de controle do LCD:
int telaLCD = 0;
int telaLCDAnterior = -1;
unsigned long tempoTrocaLCD = 0;
unsigned long tempoAtualizarLCD = 0;

//Declaração das variáveis anteriores do LCD:
int velocidadeLCDAnterior = -1;
String marchaLCDAnterior = "";
int rpmLCDAnterior = -1;
int gasolinaLCDAnterior = -1;
int sonoLCDAnterior = -1;
String dataHoraAtualLCDAnterior = "";
String dataHoraEntregaLCDAnterior = "";
String tempoDistanciaLCDAnterior = "";

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

//Declaração do caractere personalizado de cama/sono:
byte caractereCama[8] = {
  B00000,
  B10000,
  B11110,
  B10001,
  B11111,
  B11111,
  B10001,
  B00000
};

//Declaração do caractere personalizado de gasolina:
byte caractereGasolina[8] = {
  B01110,
  B01010,
  B01110,
  B01110,
  B01111,
  B01111,
  B01111,
  B00000
};


//Função para iniciar o LCD:
void iniciarLCD(){
  Wire.begin();
  delay(50);
  lcd.begin(20, 4);
  lcd.backlight();
  lcd.createChar(0, caractereCama);
  lcd.createChar(1, caractereGasolina);
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
  velocidadeLCDAnterior = -1;
  marchaLCDAnterior = "";
  rpmLCDAnterior = -1;
  gasolinaLCDAnterior = -1;
  sonoLCDAnterior = -1;
  dataHoraAtualLCDAnterior = "";
  dataHoraEntregaLCDAnterior = "";
  tempoDistanciaLCDAnterior = "";
}

//Função para preparar a tela principal do LCD:
void prepararLCDPrincipal(){
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("VEL: ");

  lcd.setCursor(0, 1);
  lcd.print("MARCHA: ");

  lcd.setCursor(0, 2);
  lcd.print("RPM: ");

  lcd.setCursor(0, 3);
  lcd.write(byte(1));
  lcd.print(": ");

  reiniciarValoresLCD();
}

//Função para preparar a tela de entrega do LCD:
void prepararLCDEntrega(){
  lcd.clear();

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
//velocidade;rpm;gasolina;marcha;dataAtual;horaAtual;dataEntrega;horaEntrega;tempoAteEntrega;distanciaRestante;sono\n
void telemetria(){
  if(Serial.available()){
    String dadoVelocidade = Serial.readStringUntil(';');
    String dadoRPM = Serial.readStringUntil(';');
    String dadoGasolina = Serial.readStringUntil(';');
    String dadoMarcha = Serial.readStringUntil(';');
    String dadoDataAtual = Serial.readStringUntil(';');
    String dadoHoraAtual = Serial.readStringUntil(';');
    String dadoDataEntrega = Serial.readStringUntil(';');
    String dadoHoraEntrega = Serial.readStringUntil(';');
    String dadoTempoAteEntrega = Serial.readStringUntil(';');
    String dadoDistanciaRestante = Serial.readStringUntil(';');
    String dadoSono = Serial.readStringUntil('\n');

    velocidade = dadoVelocidade.toInt();
    rpm = dadoRPM.toInt();
    gasolina = dadoGasolina.toInt();
    sono = dadoSono.toInt();

    dadoMarcha.trim();
    dadoDataAtual.trim();
    dadoHoraAtual.trim();
    dadoDataEntrega.trim();
    dadoHoraEntrega.trim();
    dadoTempoAteEntrega.trim();
    dadoDistanciaRestante.trim();

    dadoMarcha.toUpperCase();

    if(dadoMarcha.length() > 0){
      marchaTexto = dadoMarcha;
    }
    if(dadoDataAtual.length() > 0){
      dataAtual = dadoDataAtual;
    }
    if(dadoHoraAtual.length() > 0){
      horaAtual = dadoHoraAtual;
    }
    if(dadoDataEntrega.length() > 0){
      dataEntrega = dadoDataEntrega;
    }
    if(dadoHoraEntrega.length() > 0){
      horaEntrega = dadoHoraEntrega;
    }
    if(dadoTempoAteEntrega.length() > 0){
      tempoAteEntrega = dadoTempoAteEntrega;
    }
    if(dadoDistanciaRestante.length() > 0){
      distanciaRestante = dadoDistanciaRestante;
    }

    velocidade = constrain(velocidade, 0, VEL_MAX);
    rpm = constrain(rpm, 0, RPM_MAX);
    gasolina = constrain(gasolina, 0, GASOLINA_MAX);
    sono = constrain(sono, 0, SONO_MAX);
  }
}

//Função para transformar RPM em barras:
int barrasRPM(){
  int barras = map(rpm, 0, RPM_MAX, 0, BARRAS_MAX);

  barras = constrain(barras, 0, BARRAS_MAX);

  return barras;
}

//Função para transformar gasolina em barras:
int barrasGasolina(){
  int barras = map(gasolina, 0, GASOLINA_MAX, 0, BARRAS_MAX);

  barras = constrain(barras, 0, BARRAS_MAX);

  return barras;
}

//Função para transformar sono em barras:
int barrasSono(){
  int barras = map(sono, 0, SONO_MAX, 0, BARRAS_MAX);

  barras = constrain(barras, 0, BARRAS_MAX);

  return barras;
}

//Função para mostrar a velocidade na primeira linha:
void lcdVelocidade(){
  if(velocidade == velocidadeLCDAnterior){
    return;
  }

  lcd.setCursor(5, 0);
  lcd.print("        ");
  lcd.setCursor(5, 0);
  lcd.print(velocidade);
  lcd.print(" km/h");

  velocidadeLCDAnterior = velocidade;
}

//Função para mostrar a marcha na segunda linha:
void lcdMarcha(){
  String marchaMostrar = "";

  if(marchaTexto == "R" || marchaTexto == "R1" || marchaTexto == "R2"){
    marchaMostrar = "R";
  }
  else if(marchaTexto.length() == 1){
    marchaMostrar = marchaTexto;
  }
  else{
    marchaMostrar = marchaTexto.substring(0, 2);
  }

  if(marchaMostrar == marchaLCDAnterior){
    return;
  }

  lcd.setCursor(8, 1);
  lcd.print("  ");
  lcd.setCursor(8, 1);
  lcd.print(marchaMostrar);

  marchaLCDAnterior = marchaMostrar;
}

//Função para desenhar a linha do RPM:
void lcdRPM(){
  int i = 0;
  int barras = barrasRPM();
  int barrasLCD = map(barras, 0, BARRAS_MAX, 0, 14);

  barrasLCD = constrain(barrasLCD, 0, 14);

  if(barrasLCD == rpmLCDAnterior){
    return;
  }

  lcd.setCursor(5, 2);

  for(i=0;i<14;i++){
    if(i < barrasLCD){
      lcd.write(byte(255));
    }
    else{
      lcd.print(" ");
    }
  }

  lcd.print(" ");

  rpmLCDAnterior = barrasLCD;
}

//Função para desenhar a linha da gasolina:
void lcdGasolina(){
  int i = 0;
  int barras = barrasGasolina();
  int barrasLCD = map(barras, 0, BARRAS_MAX, 0, 16);

  barrasLCD = constrain(barrasLCD, 0, 16);

  if(barrasLCD == gasolinaLCDAnterior){
    return;
  }

  lcd.setCursor(3, 3);

  for(i=0;i<16;i++){
    if(i < barrasLCD){
      lcd.write(byte(255));
    }
    else{
      lcd.print(" ");
    }
  }

  lcd.print(" ");

  gasolinaLCDAnterior = barrasLCD;
}

//Função para mostrar a data e horário atual na primeira linha da segunda tela:
void lcdDataHoraAtual(){
  String texto = "";

  texto += "ATUAL - ";
  texto += dataAtual;
  texto += " ";
  texto += horaAtual;

  if(texto == dataHoraAtualLCDAnterior){
    return;
  }

  imprimirLinhaLCD(0, texto);

  dataHoraAtualLCDAnterior = texto;
}

//Função para mostrar o tempo restante de entrega na segunda linha da segunda tela:
void lcdDataHoraEntrega(){
  String texto = "";

  texto += "LIMITE - ";
  texto += horaEntrega;

  if(texto == dataHoraEntregaLCDAnterior){
    return;
  }

  imprimirLinhaLCD(1, texto);

  dataHoraEntregaLCDAnterior = texto;
}

//Função para mostrar tempo e distância restante na terceira linha da segunda tela:
void lcdTempoDistancia(){
  String texto = "";

  texto += "RESTAM - ";
  texto += tempoAteEntrega;
  texto += " ";
  texto += distanciaRestante;

  if(texto == tempoDistanciaLCDAnterior){
    return;
  }

  imprimirLinhaLCD(2, texto);

  tempoDistanciaLCDAnterior = texto;
}

//Função para desenhar a linha de sono/cansaço:
void lcdSono(){
  int i = 0;
  int barras = barrasSono();
  int barrasLCD = map(barras, 0, BARRAS_MAX, 0, 16);

  barrasLCD = constrain(barrasLCD, 0, 16);

  if(barrasLCD == sonoLCDAnterior){
    return;
  }

  lcd.setCursor(0, 3);
  lcd.write(byte(0));
  lcd.print(": ");

  for(i=0;i<16;i++){
    if(i < barrasLCD){
      lcd.write(byte(255));
    }
    else{
      lcd.print(" ");
    }
  }

  lcd.print(" ");

  sonoLCDAnterior = barrasLCD;
}

//Função geral da tela principal do display LCD:
void displayLCDPrincipal(){
  lcdVelocidade();
  lcdMarcha();
  lcdRPM();
  lcdGasolina();
}

//Função geral da tela de entrega do display LCD:
void displayLCDEntrega(){
  lcdDataHoraAtual();
  lcdDataHoraEntrega();
  lcdTempoDistancia();
  lcdSono();
}

//Função geral do display LCD:
void displayLCD(){
  unsigned long agora = millis();

  if(telaLCD == 0 && agora - tempoTrocaLCD >= TEMPO_TELA_PRINCIPAL){
    telaLCD = 1;
    tempoTrocaLCD = agora;
  }
  else if(telaLCD == 1 && agora - tempoTrocaLCD >= TEMPO_TELA_ENTREGA){
    telaLCD = 0;
    tempoTrocaLCD = agora;
  }

  if(telaLCD != telaLCDAnterior){
    if(telaLCD == 0){
      prepararLCDPrincipal();
    }
    else{
      prepararLCDEntrega();
    }

    telaLCDAnterior = telaLCD;
    tempoAtualizarLCD = 0;
  }

  if(agora - tempoAtualizarLCD < TEMPO_ATUALIZAR_LCD){
    return;
  }

  tempoAtualizarLCD = agora;

  if(telaLCD == 0){
    displayLCDPrincipal();
  }
  else{
    displayLCDEntrega();
  }
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
  tempoTrocaLCD = millis();
  tempoAtualizarLCD = 0;
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