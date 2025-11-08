# Sprint 4 – Placar Inteligente com FIWARE ⚙️⚽

Este projeto evolui o placar automático apresentado em [Sprint-3 Edge Computing](https://github.com/Thomas-Sievers/Sprint-3-Edge-Computing), integrando sensores físicos, um backend FIWARE completo e um dashboard web em tempo real. O objetivo é automatizar o registro de gols em partidas recreativas, eliminando anotações manuais e garantindo dados confiáveis para futuras estatísticas.

## Visão Geral
- **Firmware ESP32 (`esp32.ino`)**: monitora dois sensores PIR (um para cada gol), aciona um buzzer ao detectar gols e publica o placar via MQTT.
- **FIWARE + MQTT**: usa Orion Context Broker, IoT Agent UL e Mosquitto para receber telemetria MQTT e armazenar o contexto.
- **Dashboard Next.js (`my-app/`)**: consulta o histórico do placar e exibe gráficos e tabelas em tempo real.

## Arquitetura do Sistema
1. A ESP32 conecta-se à rede Wi-Fi, lê os sensores e publica gols nos tópicos `/TEF/device006/attrs/gb` (Time Azul) e `/TEF/device006/attrs/gr` (Time Vermelho).
2. O IoT Agent UL recebe as mensagens MQTT, converte para entidades NGSI e envia ao Orion.
3. (Opcional) Um serviço de histórico, como STH-Comet, armazena e serve os dados para consulta temporal.
4. O dashboard Next.js consome a API `/api/sth` (proxy configurável) para montar gráficos com os últimos eventos.

```
📦 sprint4-edge
├── esp32.ino                # Firmware do placar inteligente
├── fiware/                  # Stack Docker (Mongo, Orion, IoT Agent, Mosquitto)
│   ├── docker-compose.yml
│   └── mosquitto.conf
└── my-app/                  # Dashboard Next.js 16 + React 19 + Recharts
    ├── app/
    ├── components/
    └── package.json
```

## Pré-requisitos
- Node.js 20+ e npm
- Docker e Docker Compose
- IDE Arduino (ou PlatformIO) com suporte à ESP32
- Hardware: ESP32, 2 sensores PIR, buzzer, display LCD 16x2 com módulo I2C, jumpers e protoboard

## Configuração Passo a Passo

### 1. Clonar o repositório
```bash
git clone https://github.com/Thomas-Sievers/sprint4-edge.git
cd sprint4-edge
```

### 2. Subir a stack FIWARE
```bash
cd fiware
docker compose up -d
```
Verifique os serviços com `docker compose ps`. Aguarde os healthchecks ficarem `healthy`.

### 3. Provisionar o IoT Agent
Registre o serviço:
```bash
curl -X POST 'http://localhost:4041/iot/services' \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: smart' \
  -H 'fiware-servicepath: /' \
  -d '{
    "services": [{
      "apikey": "tef_api_key",
      "cbroker": "http://fiware-orion:1026",
      "entity_type": "ScoreBoard",
      "resource": "/TEF"
    }]
  }'
```

Cadastre o dispositivo `device006`:
```bash
curl -X POST 'http://localhost:4041/iot/devices' \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: smart' \
  -H 'fiware-servicepath: /' \
  -d '{
    "devices": [{
      "device_id": "device006",
      "entity_name": "scoreboard:006",
      "entity_type": "ScoreBoard",
      "transport": "MQTT",
      "attributes": [
        {"object_id": "gb", "name": "goalsBlue", "type": "Number"},
        {"object_id": "gr", "name": "goalsRed", "type": "Number"}
      ]
    }]
  }'
```

### 4. Configurar e enviar o firmware
1. Abra `esp32.ino`.
2. Ajuste `WIFI_SSID`, `WIFI_PASSWORD` e `BROKER_MQTT` para os valores da sua rede (use o IP da máquina ou container que roda o Mosquitto).
3. Faça o upload para a ESP32.
4. Use o monitor serial para confirmar a conexão Wi-Fi e MQTT.

### 5. Executar o dashboard
```bash
cd ../my-app
npm install
npm run dev
```
Acesse `http://localhost:3000`. O gráfico `Gols - Time Azul` usa o atributo `gb` e `Gols - Time Vermelho` usa `gr`.

> **Nota:** configure o endpoint real do histórico na rota `/api/sth`. É possível criar um proxy simples usando `fetch` para o STH-Comet (`http://localhost:8666/STH/v1/contextEntities/...`). Até que essa rota seja criada, os componentes exibem mensagens informando ausência de dados.

## Fluxo de Dados
- **Entrada:** sensores PIR -> ESP32 -> MQTT (Mosquitto)
- **Transformação:** IoT Agent UL -> Orion Context Broker
- **Visualização:** Next.js + Recharts, consumindo histórico (STH-Comet ou serviço equivalente)
- **Feedback:** buzzer e LCD informam cada gol, mantendo os jogadores atualizados durante a partida.

## Testes e Validação
- Utilize o monitor serial para garantir que cada detecção de movimento gera um `Gol do Blue/Red`.
- Teste o fluxo MQTT com `mosquitto_sub -t "/TEF/device006/attrs/#"`.
- Confirme a presença da entidade via Orion:
  ```bash
  curl -H 'fiware-service: smart' -H 'fiware-servicepath: /' \
    http://localhost:1026/v2/entities/scoreboard:006
  ```
- Para validar o dashboard, simule dados históricos via STH-Comet antes de colocar o firmware no ar.

## Possíveis Problemas
- **Wi-Fi não conecta:** revise SSID/senha e distância do roteador.
- **MQTT desconectando:** verifique se o IP do broker corresponde ao Mosquitto no docker (use `docker inspect fiware-mosquitto`).
- **Dashboard sem dados:** confirme se a rota `/api/sth` existe e se o serviço de histórico está ativo.

## Roadmap
- Adicionar serviço STH-Comet/QuantumLeap ao `docker-compose`.
- Criar rota Next.js `/api/sth` parametrizável via variáveis de ambiente.
- Integrar o placar físico ao futuro portal de campeonatos (cadastro de times, tabelas, estatísticas).

---
Projeto desenvolvido como evolução do placar automático apresentado no repositório [Sprint-3 Edge Computing](https://github.com/Thomas-Sievers/Sprint-3-Edge-Computing). Contribuições e sugestões são bem-vindas!