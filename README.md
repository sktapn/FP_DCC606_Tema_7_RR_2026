# Otimização de Rotas de Transporte Público 

Projeto final da disciplina de **Análise de Algoritmos (DCC606)** da Universidade Federal de Roraima (UFRR).

**Autores:** Abrahão Picanço e Wesley Silva Araújo

## 📌 Resumo
Este projeto resolve o Problema do Caixeiro Viajante Multiobjetivo (MO-TSP) para otimizar linhas de ônibus em Boa Vista-RR. O sistema busca o melhor equilíbrio (*Fronteira de Pareto*) entre maximizar a quantidade de passageiros atendidos e minimizar o tempo total de viagem da frota

O núcleo de otimização foi desenvolvido nativamente em **C++11** (utilizando algoritmos Exatos, Gulosos e o NSGA-II), com a plotagem cartográfica e de gráficos feita em **Python 3**.

## 🚀 Como rodar rápido

Abra o terminal na raiz do projeto e execute:

```bash
# 1. Compile os códigos fonte
g++ -Wall -O3 -std=c++11 src/*.cpp -o roteamento_bv

# 2. Execute a otimização (irá gerar os arquivos CSV na pasta results/)
./roteamento_bv

# 3. Gere as imagens do Mapa e da Fronteira de Pareto
python3 plot_pareto.py
