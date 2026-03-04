/*
=========================================================
 ATIVIDADE - FUSÃO DE SENSORES PARA ESTIMATIVA DE ALTURA
=========================================================

Fluxo geral do sistema:

Sensores (BMP180 + MPU6050)
        ↓
Leitura via funções fornecidas
        ↓
Processamento (Aqui é a sua implementação)
        ↓
Criação de pacote de telemetria
        ↓
Envio para base (função fornecida)

Você NÃO deve modificar:
- readBaroHeight()
- readAccelZ()
- getDeltaTime()
- sendToBase()

Seu trabalho está concentrado no loop().
*/

#include <Wire.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;

// =======================================================
// FUNÇÕES FORNECIDAS - LEITURA DOS SENSORES
// =======================================================


/*
---------------------------------------------------------
readBaroHeight()

Lê o sensor BMP180 e retorna a altura em metros.

Etapas internas:
1. Lê pressão atmosférica em Pascal.
2. Converte pressão para altura usando modelo padrão.
3. Retorna altura estimada.

IMPORTANTE:
- O valor retornado já está em METROS.
- É uma altura absoluta.
- Pode ter pequenas oscilações (ruído).
---------------------------------------------------------
*/
float readBaroHeight() {
    float pressure = bmp.readPressure();
    float seaLevel = 101325.0;  // pressão de referência
    float height = 44330.0 * (1.0 - pow(pressure / seaLevel, 0.1903));
    return height;
}


/*
---------------------------------------------------------
readAccelZ()

Lê o eixo Z do MPU6050 via I2C.

Etapas internas:
1. Acessa registrador do eixo Z.
2. Lê valor bruto (int16).
3. Converte para unidade "g".
4. Converte para m/s².

IMPORTANTE:
- O valor retornado inclui a gravidade.
- Quando parado sobre uma mesa:
      retorno ≈ +9.81 m/s²
- NÃO retorna zero quando parado.
---------------------------------------------------------
*/
float readAccelZ() {

    // Inicia comunicação com endereço do MPU6050
    Wire.beginTransmission(0x68);

    // Registrador inicial do eixo Z
    Wire.write(0x3F);

    // Finaliza escrita, mas mantém conexão ativa
    Wire.endTransmission(false);

    // Solicita 2 bytes (eixo Z)
    Wire.requestFrom(0x68, 2, true);

    // Junta os dois bytes em um inteiro de 16 bits
    int16_t raw = Wire.read() << 8 | Wire.read();

    // Conversão para g (escala ±2g)
    float accel = raw / 16384.0;

    // Conversão para m/s²
    return accel * 9.81;
}


/*
---------------------------------------------------------
sendToBase()

Simula envio de telemetria para uma base remota.

Você deve:
- Criar uma struct
- Preencher essa struct
- Passar como argumento para essa função

A função está em formato template para aceitar
qualquer tipo de struct definida por você, de modo
a não depender de uma eventual "passagem de nome igual"
para a função.
---------------------------------------------------------
*/
template <typename T>
void sendToBase(const T& packet) {
    Serial.println("[TX] Enviando pacote para base...");
}


/*
---------------------------------------------------------
Controle de tempo

getDeltaTime() retorna o intervalo de tempo (dt)
em segundos entre duas execuções consecutivas do loop().

Você DEVE usar esse valor para integração.
---------------------------------------------------------
*/
unsigned long lastTime = 0;

float getDeltaTime() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0;
    lastTime = now;
    return dt;
}


// =======================================================
// SUA IMPLEMENTAÇÃO
// =======================================================

/*
---------------------------------------------------------
A partir daqui é sua responsabilidade.

Você deve:

1. Criar uma struct de telemetria.
2. Criar variáveis de estado (altura, velocidade, etc.).
3. Definir o parâmetro alpha do filtro complementar.

Dica:
As variáveis de estado precisam manter valor entre
iterações do loop(), portanto NÃO devem ser locais.
---------------------------------------------------------
*/

struct TelemetryPacket {
    float baroHeight;
    float integratedHeight;
    float fusedHeight;
    float velocity;
    float accelLinear;
}

// =======================================================
// VARIÁVEIS DE ESTADO
// =======================================================

float velocity = 0.0f;
float integratedHeight = 0.0f;
float fusedHeight = 0.0f;

// Parâmetro para filtro complementar
const float alpha = 0.98f;

void setup() {

    Serial.begin(115200);

    // Inicializa comunicação I2C
    Wire.begin();

    // Inicializa BMP180
    bmp.begin();

    // Inicializa controle de tempo
    lastTime = millis();
}



void loop() {

    /*
    -----------------------------------------------------
    ETAPA 1 - LEITURA DOS SENSORES
    -----------------------------------------------------
    */

    float baroHeight = readBaroHeight();
    float accelZ = readAccelZ();
    float dt = getDeltaTime();


    /*
    -----------------------------------------------------
    A partir daqui começa a sua implementação.

    Você tem acesso a:

    baroHeight → altura absoluta em metros
    accelZ     → aceleração em m/s² (inclui gravidade)
    dt         → intervalo de tempo em segundos

    -----------------------------------------------------
    ETAPA 2 - PROCESSAMENTO
    -----------------------------------------------------

    Você deve:

    1. Tratar o sinal de aceleração
    2. Integrar para obter velocidade
    3. Integrar novamente para obter altura integrada
    4. Aplicar filtro complementar

    Perguntas que devem guiar sua implementação:

    - Se o sistema estiver parado, a velocidade deve crescer?
    - Se integrar diretamente accelZ, o que acontece?
    - Qual parte do sinal representa apenas movimento?

    -----------------------------------------------------
    */

    // 1. Remover gravidade
    accelZ -= 9.81;

    // 2. Integração para obter velocidade
    velocity += accelZ * dt;

    // 3. Integração para obter altura
    integratedHeight += velocity * dt;

    // 4. Filtro complementar
    fusedHeight = alpha * integratedHeight + (1.0 - alpha) * baroHeight;

    /*
    -----------------------------------------------------
    ETAPA 3 - TELEMETRIA
    -----------------------------------------------------

    Agora você deve:

    1. Criar uma variável do tipo da sua struct.
    2. Preencher os campos que considerar relevantes.
    3. Enviar para a base usando sendToBase().

    -----------------------------------------------------
    */

    TelemetryPacket packet;

    packet.baroHeight = baroHeight;
    packet.integratedHeight = integratedHeight;
    packet.fusedHeight = fusedHeight;
    packet.velocity = velocity;
    packet.accelLinear = accelZ;

    sendToBase(packet);

    delay(10);
}
