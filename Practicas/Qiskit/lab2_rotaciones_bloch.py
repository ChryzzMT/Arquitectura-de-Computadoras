import math
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from qiskit import QuantumCircuit
from qiskit_aer import AerSimulator


def simular_angulo(theta: float):
    # 1. Circuito de 1 cúbit y 1 bit clásico
    qc = QuantumCircuit(1, 1)
    # 2. Rotación en el eje Y por ángulo continuo theta
    qc.ry(theta, 0)
    # 3. Medición en la base computacional Z
    qc.measure(0, 0)
    # 4. Simulación de 10,000 mediciones
    backend = AerSimulator()
    counts = backend.run(qc, shots=10000).result().get_counts()
    return counts


# Evaluación en 4 posiciones geométricas clave de la Esfera de Bloch:
angulos = [
    (0.0, "Polo Norte (theta = 0)"),
    (math.pi / 4, "Ángulo 45° (theta = pi/4)"),
    (math.pi / 2, "Ecuador (theta = pi/2, Estado |+>)"),
    (math.pi, "Polo Sur (theta = pi, Estado |1>)"),
]

# Circuito de referencia (ecuador) para el diagrama
qc_ejemplo = QuantumCircuit(1, 1)
qc_ejemplo.ry(math.pi / 2, 0)
qc_ejemplo.measure(0, 0)
print("CIRCUITO (ejemplo θ = π/2):")
print(qc_ejemplo.draw(output="text"))
print()

# Barrido fino para comparar la curva medida con cos²(θ/2) y sin²(θ/2).
# Los cuatro ángulos de la guía se imprimen al pasar por ellos.
n_puntos = 17
shots = 10000
thetas = []
p0_med = []
p1_med = []
p0_err = []
p1_err = []

for i in range(n_puntos):
    theta = i * math.pi / (n_puntos - 1)
    c = simular_angulo(theta)
    p0 = c.get("0", 0) / shots
    p1 = c.get("1", 0) / shots
    thetas.append(theta)
    p0_med.append(p0)
    p1_med.append(p1)
    p0_err.append(math.sqrt(p0 * (1.0 - p0) / shots))
    p1_err.append(math.sqrt(p1 * (1.0 - p1) / shots))
    for theta_clave, desc in angulos:
        if abs(theta - theta_clave) < 1e-9:
            print(f"{desc:35s} -> P(|0⟩): {p0 * 100:5.1f}% | P(|1⟩): {p1 * 100:5.1f}%")

desviacion = max(abs(p - math.cos(t / 2) ** 2) for t, p in zip(thetas, p0_med))
print(f"\nDesviación máxima |P(|0⟩) - cos²(θ/2)| = {desviacion * 100:.2f} puntos porcentuales")

# Curva teórica continua y puntos medidos
thetas_teo = [j * math.pi / 400 for j in range(401)]
teo0 = [math.cos(t / 2) ** 2 for t in thetas_teo]
teo1 = [math.sin(t / 2) ** 2 for t in thetas_teo]

fig, ax = plt.subplots(figsize=(8.2, 5.0))
ax.plot(thetas_teo, teo0, color="#1f4e79", linewidth=2.0, label="teoría  cos²(θ/2)")
ax.plot(thetas_teo, teo1, color="#c45911", linewidth=2.0, linestyle="--", label="teoría  sin²(θ/2)")
ax.errorbar(
    thetas, p0_med, yerr=p0_err, fmt="o", color="#1f4e79",
    markersize=5, capsize=3, linewidth=1, label="P(|0⟩) medida",
)
ax.errorbar(
    thetas, p1_med, yerr=p1_err, fmt="s", color="#c45911",
    markersize=5, capsize=3, linewidth=1, label="P(|1⟩) medida",
)
ax.set_xlim(0, math.pi)
ax.set_ylim(-0.05, 1.05)
ax.set_xticks([0, math.pi / 4, math.pi / 2, 3 * math.pi / 4, math.pi])
ax.set_xticklabels(["0", "π/4", "π/2", "3π/4", "π"])
ax.set_xlabel("ángulo θ de la compuerta Ry")
ax.set_ylabel("probabilidad")
ax.set_title("Rotación Ry(θ): medición frente a cos²(θ/2) y sin²(θ/2)")
ax.grid(True, alpha=0.35)
ax.legend(loc="center right")
fig.tight_layout()

salida = Path(__file__).resolve().parent / "evidencias" / "lab2"
salida.mkdir(parents=True, exist_ok=True)
fig.savefig(salida / "grafica.png", dpi=150, bbox_inches="tight", facecolor="white")
plt.close(fig)

fig_circ = qc_ejemplo.draw(output="mpl", fold=-1)
if not hasattr(fig_circ, "savefig"):
    fig_circ = plt.gcf()
fig_circ.savefig(salida / "circuito.png", dpi=150, bbox_inches="tight", facecolor="white")
plt.close(fig_circ)

print(f"Figuras guardadas en: {salida}")
