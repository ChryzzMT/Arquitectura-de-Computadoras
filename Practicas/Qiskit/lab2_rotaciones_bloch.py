import math

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

qc_ejemplo = QuantumCircuit(1, 1)
qc_ejemplo.ry(math.pi / 2, 0)
qc_ejemplo.measure(0, 0)
print("CIRCUITO (ejemplo θ = π/2):")
print(qc_ejemplo.draw(output="text"))
print()

shots = 10000
print(f"TABLA Ry(θ), {shots} disparos por ángulo")
print(f"{'Ángulo':<36} {'P(0) med':>9} {'P(0) teo':>9} {'P(1) med':>9} {'P(1) teo':>9}")
for theta, desc in angulos:
    c = simular_angulo(theta)
    p0 = c.get("0", 0) / shots
    p1 = c.get("1", 0) / shots
    t0 = math.cos(theta / 2) ** 2
    t1 = math.sin(theta / 2) ** 2
    print(
        f"{desc:<36} {p0 * 100:8.2f}% {t0 * 100:8.2f}% {p1 * 100:8.2f}% {t1 * 100:8.2f}%"
    )
    print(f"  conteos: |0⟩={c.get('0', 0)}  |1⟩={c.get('1', 0)}")

print()
print("Justificación: Ry(θ)|0⟩ = cos(θ/2)|0⟩ + sin(θ/2)|1⟩. Al medir en Z,")
print("P(|0⟩) = cos²(θ/2) y P(|1⟩) = sin²(θ/2). En θ = π/4, cos²(π/8) ≈ 85.36 %.")
print("La diferencia con la columna teórica es el ruido de 10 000 disparos.")
