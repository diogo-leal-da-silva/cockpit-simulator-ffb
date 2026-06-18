# Cockpit Simulador com Force Feedback

Projeto desenvolvido para a disciplina de Projeto de Sistemas da Computação, com o objetivo de integrar hardware e software na construção de um cockpit simulador.

O sistema utiliza Arduino Leonardo, display LCD I²C, botões, câmbio H-Shifter, freio de mão analógico, telemetria pelo SimHub e um sistema de force feedback baseado em controladora MKS ODrive, firmware FFBeast e motor BLDC de hoverboard.

## Principais funcionalidades

* Leitura de botões e comandos físicos;
* Câmbio H-Shifter;
* Freio de mão analógico;
* Display de sete segmentos para indicação de marcha;
* Display LCD 20×4 com dados de telemetria;
* Perfis para Euro Truck Simulator 2 e DiRT Rally 2.0;
* Sistema de force feedback direct drive;
* Software em Python para compilação e envio automático dos sketches;
* Integração com Arduino CLI e SimHub.

## Estrutura do repositório

* `docs/`: documentação técnica e manuais;
* `codigos/arduino/`: sketches do Arduino Leonardo;
* `codigos/python/`: software de seleção e envio dos sketches;
* `codigos/simhub/`: scripts de telemetria;
* `eletrico/`: esquemas elétricos e pinagens;
* `modelos-3d/`: modelos e peças do projeto;
* `imagens/`: fotos, diagramas e registros da montagem.

## Jogos implementados

* Euro Truck Simulator 2;
* DiRT Rally 2.0.

## Documentação

A documentação completa apresenta o funcionamento do sistema, os componentes utilizados, os protocolos de comunicação, a modelagem de dados, os códigos desenvolvidos e os procedimentos de montagem e configuração.

## Aviso de segurança

O projeto utiliza motor de alto torque, fonte de 36 V, controladora de potência e resistor de frenagem. A montagem e a utilização devem ser realizadas com cuidado, respeitando limites de corrente, isolamento, refrigeração e proteção mecânica.

## Autoria

Projeto desenvolvido por Diogo Leal da Silva para a disciplina de Projeto de Sistemas da Computação.

## Licença

Os códigos-fonte deste projeto são disponibilizados sob a licença MIT. Consulte o arquivo `LICENSE`.
