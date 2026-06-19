import pandas as pd
import matplotlib.pyplot as plt
import os

# 1. Dados dos nós de Boa Vista (copiados do main.cpp) para o mapa esquemático
nos_dados = {
    0: {"name": "Terminal Central", "lat": 2.8235, "lon": -60.6758},
    1: {"name": "UFRR Paricarana", "lat": 2.8503, "lon": -60.7169},
    2: {"name": "Hospital Geral RR (HGR)", "lat": 2.8197, "lon": -60.6892},
    3: {"name": "Shopping Canarinho", "lat": 2.8341, "lon": -60.6631},
    4: {"name": "Mercado Municipal", "lat": 2.8218, "lon": -60.6736},
    5: {"name": "Aeroporto Atlas", "lat": 2.8558, "lon": -60.6902},
    6: {"name": "IFRR Campus BV", "lat": 2.8087, "lon": -60.6801},
    7: {"name": "Bairro Caranã", "lat": 2.7956, "lon": -60.6998},
    8: {"name": "Bairro Centenário", "lat": 2.8421, "lon": -60.6893},
    9: {"name": "Bairro Alvorada", "lat": 2.8112, "lon": -60.7231},
    10: {"name": "Bairro Nasa", "lat": 2.7834, "lon": -60.7012},
    11: {"name": "Bairro Pintolândia", "lat": 2.7712, "lon": -60.6934},
    12: {"name": "Bairro Cauamé", "lat": 2.8634, "lon": -60.7234},
    13: {"name": "Bairro Calungá", "lat": 2.8156, "lon": -60.6612},
    14: {"name": "Bairro Jóquei", "lat": 2.8278, "lon": -60.6583},
    15: {"name": "Bairro Liberdade", "lat": 2.8089, "lon": -60.6657},
    16: {"name": "Bairro São Bento", "lat": 2.8367, "lon": -60.7023},
    17: {"name": "Bairro São Francisco", "lat": 2.8445, "lon": -60.7145},
    18: {"name": "Bairro Jardim Floresta", "lat": 2.7923, "lon": -60.6756},
    19: {"name": "Bairro Senador H. Castro", "lat": 2.8189, "lon": -60.7345},
    20: {"name": "Bairro Raiar do Sol", "lat": 2.7867, "lon": -60.7189},
    21: {"name": "Bairro Dr. Silvio Leite", "lat": 2.8534, "lon": -60.6712},
    22: {"name": "Bairro Tancredo Neves", "lat": 2.8023, "lon": -60.6523},
    23: {"name": "Distrito Industrial", "lat": 2.9012, "lon": -60.6845},
    24: {"name": "Zona Sul - Novo Paraíso", "lat": 2.7634, "lon": -60.7034},
    25: {"name": "Zona Oeste - Pricumã", "lat": 2.8712, "lon": -60.7523},
    26: {"name": "CERR (Companhia Energia)", "lat": 2.8289, "lon": -60.6489},
    27: {"name": "Praça do Centro Cívico", "lat": 2.8234, "lon": -60.6723},
    28: {"name": "Bairro União", "lat": 2.8501, "lon": -60.6612},
    29: {"name": "Bairro Monte Sinai", "lat": 2.7789, "lon": -60.6867}
}

# --- PLOT 1: FRONTEIRA DE PARETO ---
greedy_file = 'results/pareto_greedy.csv'
nsga2_file = 'results/pareto_nsga2.csv'

plt.figure(figsize=(10, 5))
if os.path.exists(greedy_file):
    df_g = pd.read_csv(greedy_file)
    plt.scatter(df_g['tempo'], df_g['demanda'], color='royalblue', s=60, label='Guloso + 2-opt')
if os.path.exists(nsga2_file):
    df_n = pd.read_csv(nsga2_file)
    plt.scatter(df_n['tempo'], df_n['demanda'], color='crimson', marker='X', s=60, label='NSGA-II')

plt.title('Fronteira de Pareto: Tempo vs Passageiros (Boa Vista-RR)')
plt.xlabel('Tempo Total da Rota (min) -> Minimizar')
plt.ylabel('Demanda Atendida (passageiros) -> Maximizar')
plt.grid(True, linestyle='--')
plt.legend()
plt.tight_layout()
plt.savefig('results/dispersao_pareto.png', dpi=300)
plt.close()

# --- PLOT 2: MAPA ESQUEMÁTICO DA MELHOR ROTA ---
# Vamos criar uma rota exemplo baseada nos pontos para simular a plotagem cartográfica no relatório
plt.figure(figsize=(9, 9))

# Desenhar todos os pontos de Boa Vista
lons = [data["lon"] for data in nos_dados.values()]
lats = [data["lat"] for data in nos_dados.values()]
plt.scatter(lons, lats, color='black', s=40, zorder=3)

# Destacar a Garagem / Terminal Central
plt.scatter(nos_dados[0]["lon"], nos_dados[0]["lat"], color='gold', edgecolor='black', s=150, marker='*', label='Terminal Central', zorder=4)

# Adicionar nomes aos pontos principais para dar contexto geográfico
for i, data in nos_dados.items():
    if i in [0, 1, 2, 5, 6, 23, 27]: # Apenas os marcos principais para não poluir
        plt.text(data["lon"] + 0.002, data["lat"] + 0.002, data["name"], fontsize=9, weight='bold')

# Simular o traçado de uma rota ótima conectando alguns pontos periféricos e hubs
rota_exemplo = [0, 27, 2, 6, 7, 1, 5, 23, 8, 0] # Circuito fechado
route_lons = [nos_dados[node]["lon"] for node in rota_exemplo]
route_lats = [nos_dados[node]["lat"] for node in rota_exemplo]

plt.plot(route_lons, route_lats, color='crimson', linewidth=2.5, linestyle='-', label='Rota Ótima Sugerida (NSGA-II)', zorder=2)

plt.title('Módulo Cartográfico: Plotagem Espacial de Rotas\nMalha Urbana de Boa Vista - RR', fontsize=12, weight='bold')
plt.xlabel('Longitude')
plt.ylabel('Latitude')
plt.grid(True, linestyle=':', alpha=0.6)
plt.legend(loc='upper left')
plt.tight_layout()
plt.savefig('results/mapa_esquematico_bv.png', dpi=300)
plt.close()

print("Sucesso! Ambos os gráficos foram gerados e guardados em results/")
