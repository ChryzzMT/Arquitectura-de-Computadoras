from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from qiskit import QuantumCircuit
from qiskit.visualization import plot_histogram
from qiskit_aer import AerSimulator


def construir_oraculo(tipo: str) -> QuantumCircuit:
    oraculo = QuantumCircuit(2, name=f"U_f ({tipo})")
    if tipo == "constante_0":
        pass  # f(x) = 0
    elif tipo == "constante_1":
        oraculo.x(1)  # f(x) = 1
    elif tipo == "balanceada_identidad":
        oraculo.cx(0, 1)  # f(x) = x
    elif tipo == "balanceada_inversa":
        oraculo.x(0); oraculo.cx(0, 1); oraculo.x(0)  # f(x) = NOT x
    return oraculo


def ejecutar_deutsch_sin_x(tipo: str):
    qc = QuantumCircuit(2, 1)
    # 1. Se omite la X sobre la ancilla: q1 permanece en |0> antes del Hadamard
    # qc.x(1)
    # 2. Hadamard en ambos cúbits: q0 -> |+>, q1 -> |+>  (no se prepara |->)
    qc.h(0)
    qc.h(1)
    # 3. Consulta única al oráculo U_f
    qc.compose(construir_oraculo(tipo), inplace=True)
    # 4. Interferencia Hadamard en q0 y medición
    qc.h(0)
    qc.measure(0, 0)
    counts = AerSimulator().run(qc, shots=1000).result().get_counts()
    return qc, counts


print("Deutsch SIN la compuerta X en la ancilla q1:")
print("La ancilla queda en |+⟩. Ese estado no genera la fase (-1)^f(x).")
print()

casos = ["constante_0", "constante_1", "balanceada_identidad", "balanceada_inversa"]
resultados = []
for caso in casos:
    circ, res = ejecutar_deutsch_sin_x(caso)
    bit = max(res, key=res.get)
    conclusion = "CONSTANTE" if bit == "0" else "BALANCEADA"
    print(f"Oráculo: {caso:22s} | Medición q0: '{bit}' -> el algoritmo diría {conclusion}")
    print(f"  conteos: {dict(sorted(res.items()))}")
    resultados.append((caso, circ, res, bit))

print()
print("Los cuatro oráculos, constantes y balanceados, colapsan en 0.")
print("Sin |−⟩ no hay phase kickback y una sola consulta ya no distingue la función.")

salida = Path(__file__).resolve().parent / "evidencias" / "reto2"
salida.mkdir(parents=True, exist_ok=True)

fig, axes = plt.subplots(2, 2, figsize=(12.5, 8.2))
for ax, (caso, circ, _res, bit) in zip(axes.ravel(), resultados):
    circ.draw(output="mpl", ax=ax, fold=-1)
    ax.set_title(f"{caso}   →   q0 = '{bit}'")
fig.tight_layout()
fig.savefig(salida / "circuito.png", dpi=140, bbox_inches="tight", facecolor="white")
plt.close(fig)

fig_hist = plot_histogram(
    [res for _caso, _circ, res, _bit in resultados],
    legend=casos,
    title="Deutsch sin X en la ancilla: medición de q0 (1 000 disparos)",
    figsize=(8.5, 4.6),
    bar_labels=False,
)
fig_hist.savefig(salida / "histograma.png", dpi=150, bbox_inches="tight", facecolor="white")
plt.close(fig_hist)

print(f"\nFiguras guardadas en: {salida}")
