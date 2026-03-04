# Fusão de Sensores para Estimativa de Altura

Este projeto implementa um sistema simplificado de **fusão de sensores** para estimativa de altura vertical em um sistema embarcado.

O objetivo é combinar medições provenientes de dois sensores conectados via I2C:

- **BMP180** – fornece altura absoluta a partir da pressão atmosférica.
- **MPU6050** – fornece aceleração linear no eixo vertical.

## Objetivo

Estimar a altura vertical do sistema de forma mais precisa e estável, combinando:

- A altura absoluta do barômetro (estável, porém ruidosa e com menor resposta dinâmica);
- A altura obtida pela dupla integração da aceleração (resposta rápida, porém sujeita à deriva).

A fusão é realizada por meio de um **Filtro Complementar**.

---

## Método Utilizado

### 1. Tratamento da Aceleração

O acelerômetro mede aceleração específica, incluindo a gravidade:

\[
a_{medido} = a_{movimento} + g
\]

Portanto, a gravidade (≈ 9.81 m/s²) é removida antes da integração.

---

### 2. Integração Numérica

A aceleração linear é integrada para obter:

- Velocidade
- Altura integrada

Utilizando integração discreta baseada no tempo de amostragem `dt`.

---

### 3. Filtro Complementar

A estimativa final da altura é calculada por:

\[
h_{estimada} = \alpha \cdot h_{integrada} + (1-\alpha) \cdot h_{baro}
\]

Onde:

- `h_integrada` → altura obtida pela integração da aceleração
- `h_baro` → altura medida pelo barômetro
- `α` → coeficiente entre 0 e 1

Valores típicos de α próximos de 1 favorecem resposta rápida, enquanto valores menores aumentam estabilidade.

---

## Estrutura do Sistema

Sensores → Leitura → Processamento (Fusão) → Pacote de Telemetria → Envio para Base

O processamento ocorre no `loop()` principal e os dados são enviados através de uma função de telemetria simulada.

---