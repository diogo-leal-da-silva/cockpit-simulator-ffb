# Cockpit Simulador com Force Feedback

Projeto de um cockpit de simulação desenvolvido para a disciplina de **Projeto de Sistemas da Computação**, integrando sistemas embarcados, eletrônica de potência, programação, telemetria e controle de motores.

O sistema combina um **Arduino Leonardo** para os comandos auxiliares e a telemetria com uma controladora **MKS baseada em ODrive**, executando o firmware **FFBeast**, para o controle de um motor BLDC de hoverboard utilizado como base *direct drive*.

> [!WARNING]
> Este projeto utiliza motor de alto torque, fonte de 36 V, controladora de potência e resistor de frenagem. A montagem e os testes devem ser realizados com proteção mecânica, limites de corrente corretamente configurados, botão de emergência, isolamento elétrico e refrigeração adequada.

## Visão geral

O cockpit foi desenvolvido para reunir, em um único sistema:

* volante *direct drive* com *force feedback*;
* motor BLDC de hoverboard;
* controladora MKS/ODrive com firmware FFBeast;
* encoder incremental para leitura da posição do volante;
* Arduino Leonardo configurado como dispositivo USB HID;
* painel com botões e chave de seta;
* câmbio H-Shifter;
* freio de mão analógico;
* pedais analógicos;
* display LCD I²C 20×4 com dados de telemetria;
* display de sete segmentos para indicação da marcha;
* multiplexador 74HC4067;
* contador e decodificador CD4026;
* SimHub para leitura e envio da telemetria;
* aplicativo em Python para compilar e gravar os sketches por meio do Arduino CLI.

## Jogos implementados

Atualmente, o projeto possui perfis para:

* **Euro Truck Simulator 2**
* **DiRT Rally 2.0**

Cada jogo utiliza um sketch e um script de telemetria próprios, pois as informações exibidas no LCD são diferentes.

### Euro Truck Simulator 2

O perfil apresenta informações como:

* velocidade;
* RPM;
* marcha;
* combustível;
* nível de cansaço;
* horário do jogo;
* prazo da entrega;
* tempo e distância restantes.

### DiRT Rally 2.0

O perfil apresenta:

* velocidade;
* RPM;
* volta atual;
* total de voltas;
* melhor volta da corrida;
* melhor volta registrada pelo piloto.

## Arquitetura resumida

```text
Botões, câmbio e freio de mão
              │
              ▼
      Arduino Leonardo
              │
              ├── USB HID ─────────────► Jogo
              │
              ├── I²C ─────────────────► LCD 20×4
              │
              └── sinais digitais ─────► CD4026 e display de 7 segmentos

Jogo ──► SimHub ──► USB Serial ──► Arduino Leonardo

Jogo ──► FFBeast ──► MKS/ODrive ──► Motor BLDC
                         ▲
                         │
                      Encoder
```

## Estrutura do repositório

| Pasta                      | Conteúdo                                                                                |
| -------------------------- | --------------------------------------------------------------------------------------- |
| [`codigos/`](codigos/)     | Sketches do Arduino, scripts do SimHub, aplicativo em Python e documentação dos códigos |
| [`docs/`](docs/)           | Documentação principal do projeto, PDF completo e diagrama de modelagem de dados        |
| [`esquemas/`](esquemas/)   | Esquemas elétricos, pinagens e diagramas dos componentes                                |
| [`montagem/`](montagem/)   | Procedimentos e informações relacionadas à montagem                                     |
| [`resultado/`](resultado/) | Resultados, observações, limitações e comportamentos identificados                      |
| [`software/`](software/)   | Arquivos e instruções do software de seleção e envio dos sketches                       |

## Códigos disponíveis

Na pasta [`codigos/`](codigos/) estão disponíveis:

* [`ETS2.ino`](codigos/ETS2.ino): sketch utilizado com o Euro Truck Simulator 2;
* [`DiRTRally2.ino`](codigos/DiRTRally2.ino): sketch utilizado com o DiRT Rally 2.0;
* [`simhub_ETS2.txt`](codigos/simhub_ETS2.txt): script de telemetria do ETS2;
* [`simhub_DR2.txt`](codigos/simhub_DR2.txt): script de telemetria do DiRT Rally 2.0;
* [`app.py`](codigos/app.py): interface em Python para compilação e envio dos sketches;
* [`codigos.md`](codigos/codigos.md): explicação detalhada da estrutura e do funcionamento dos códigos.

## Documentação

A documentação principal apresenta:

* introdução, motivação, justificativa e objetivos;
* funcionamento da controladora MKS/ODrive;
* funcionamento do firmware FFBeast;
* controle FOC e acionamento do motor BLDC;
* motor de hoverboard;
* resistor de frenagem;
* protocolos de comunicação;
* componentes ligados ao Arduino Leonardo;
* software desenvolvido;
* modelagem e fluxo dos dados;
* resultados e limitações observadas.

Arquivos principais:

* [`Documentacao_Projeto_Cockpit_FFB.pdf`](docs/Documentacao_Projeto_Cockpit_FFB.pdf)
* [`documentacao_principal.md`](docs/documentacao_principal.md)
* [`modelagem_dados.png`](docs/modelagem_dados.png)

## Requisitos de software

Para utilizar e modificar o projeto, podem ser necessários:

* Arduino IDE ou Arduino CLI;
* core `arduino:avr`;
* biblioteca `Joystick`;
* biblioteca `LiquidCrystal_I2C`;
* Python 3;
* SimHub;
* FFBeast Setup;
* STM32CubeProgrammer, para gravação do firmware na controladora compatível.

## Uso do aplicativo de sketches

O aplicativo em Python permite:

1. selecionar o jogo;
2. detectar a porta do Arduino;
3. instalar ou atualizar o core AVR;
4. compilar o sketch;
5. gravar automaticamente o código no Arduino Leonardo.

Para adicionar outro jogo, deve-se criar um novo sketch, configurar a telemetria correspondente no SimHub e incluir o perfil no arquivo `config.json` utilizado pelo programa.

## Observações técnicas

Durante os testes, foram identificados alguns pontos que ainda podem ser aprimorados:

* o motor possui torque elevado e deve operar com limites de corrente e força corretamente configurados;
* configurações inadequadas do FFBeast ou do encoder podem provocar movimentos bruscos ou perda de referência;
* a leitura das marchas pelo multiplexador pode apresentar atraso em trocas muito rápidas;
* o resistor de frenagem deve possuir dissipação e refrigeração adequadas;
* todas as ligações de potência devem ser verificadas antes da energização.

Essas observações não impedem o funcionamento do projeto, mas indicam pontos importantes para futuras revisões de hardware, software e segurança.

## Estado do projeto

O projeto encontra-se em desenvolvimento e documentação. Os arquivos podem ser atualizados à medida que forem realizados novos testes, ajustes mecânicos, correções de software e melhorias na montagem.

## Autoria

Desenvolvido por **Diogo Leal da Silva** para a disciplina de **Projeto de Sistemas da Computação**.

## Licença

O repositório utiliza a licença [MIT](LICENSE).

A licença MIT cobre os códigos disponibilizados no repositório. Arquivos, bibliotecas, firmwares e projetos de terceiros, como Arduino, SimHub, FFBeast e ODrive, permanecem sujeitos às suas respectivas licenças e termos de uso.
